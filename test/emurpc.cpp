
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <catch.hpp>
#include <nlohmann/json.hpp>

#include "emurpc.hpp"

class MemoryOnlyEmu {
public:
    explicit MemoryOnlyEmu() : memory(0x10000) {
        EmuRPC::Config conf = {
            this, // user_data
            [](void* d, u64 addr, u64 size, u8* out) {
                ((MemoryOnlyEmu*)d)->ReadMemory(addr, size, out);
            }, // memory_read_callback
            [](void* d, u64 addr, u64 size, const u8* data) {
                ((MemoryOnlyEmu*)d)->WriteMemory(addr, size, data);
            },       // memory_write_callback
            nullptr, // gpu_read_callback
            nullptr, // gpu_write_callback
            nullptr, // special_read_callback
            nullptr, // special_write_callback
            nullptr, // save_state_callback
            nullptr, // load_state_callback
            nullptr, // load_rom_callback
            nullptr, // create_overlay_callback
            nullptr, // draw_overlay_text_callback
            true,    // enable_memory_access_timing;
            false,   // enable_gpu_access_timing;
            false,   // enable_special_access_timing;
        };
        emurpc = std::make_unique<EmuRPC>(conf);
    }

    void Run(u32 num_frames) {
        while (num_frames-- > 0) {
            RunCPU();
            AdvanceFrame();
        }
    }

private:
    void RunCPU() {
        // Every frame, randomly read or write a bunch of random data to every 16 `write_size`
        std::random_device device;
        std::mt19937 generator{device()};
        std::uniform_int_distribution<short> dist{0, 255};
        constexpr size_t access_size = 0x10;
        for (int i = 0; i < memory.size() / access_size; i += 16 * access_size) {
            if (dist(generator) % 2 == 0) {
                std::vector<u8> rand_data(access_size);
                std::generate(rand_data.begin(), rand_data.end(),
                              [&]() { return dist(generator); });
                WriteMemory(access_size * i, access_size, rand_data.data());
            } else {
                std::vector<u8> rand_data(access_size);
                ReadMemory(access_size * i, access_size, rand_data.data());
            }
        }
    }

    void ReadMemory(u64 addr, u64 size, u8* out) {
        std::copy(memory.begin() + addr, memory.begin() + addr + size, out);
        emurpc->OnMemoryAccess(EmuRPC::AccessType::Read, addr, size);
    }

    void WriteMemory(u64 addr, u64 size, const u8* data) {
        std::copy(data, data + size, memory.begin() + addr);
        emurpc->OnMemoryAccess(EmuRPC::AccessType::Write, addr, size);
    }

    void AdvanceFrame() {
        frame_count++;
        emurpc->OnFrameEnd();
    }

    u32 frame_count = 0;
    std::vector<u8> memory;
    std::unique_ptr<EmuRPC> emurpc;
};

TEST_CASE("Memory Read/Write", "[emurpc]") {
    using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
    namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>
    using json = nlohmann::json;

    // start the fake emulator and emurpc server
    MemoryOnlyEmu emu;

    boost::asio::io_context ioc;
    tcp::resolver resolver{ioc};
    websocket::stream<tcp::socket> ws{ioc};
    auto const results = resolver.resolve("localhost", "8080");
    boost::asio::connect(ws.next_layer(), results.begin(), results.end());

    // Perform the websocket handshake
    ws.handshake("localhost", "/");

    std::string request = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "method": "memory_read",
  "params": {
    "timing": "immediate",
    "sync": "nonblocking",
    "function": "once",
    "address": 1000,
    "length": 64
  }
})a";

    // Send the request
    ws.write(boost::asio::buffer(request));
    emu.Run(1);

    // This buffer will hold the incoming message
    boost::beast::flat_buffer buffer;

    // Read a message into our buffer
    ws.read(buffer);

    std::string temp = boost::beast::buffers_to_string(buffer.data());

    json response = json::parse(temp);

    json expected = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "result": "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="
})a"_json;

    REQUIRE(response == expected);

    // Close the WebSocket connection
    ws.close(websocket::close_code::normal);
}
