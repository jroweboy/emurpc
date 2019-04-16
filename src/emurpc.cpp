
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <boost/lockfree/spsc_queue.hpp>
#include <boost/variant/polymorphic_get.hpp>
#include "emurpc.h"
#include "emurpc.hpp"
#include "protocol.h"
#include "rpc_server.h"

class EmuRPC::Impl {
public:
    Config config;
    std::atomic_bool stopped;
    std::atomic_bool blocking_emu;

    std::unordered_map<Request::Timing, std::vector<Request::AnyPacket>> callbacks;
    boost::lockfree::spsc_queue<Request::AnyPacket, boost::lockfree::capacity<1024>> client_emu;
    boost::lockfree::spsc_queue<Response::AnyPacket, boost::lockfree::capacity<1024>> emu_client;
    std::thread thread;
    std::unique_ptr<RPCServer> server = nullptr;

    Impl(Config config) : config(config) {}

    ~Impl() {
        stopped = true;
        thread.join();
    }

    // called by server thread
    void ProcessEvents() {
        while (!stopped) {
            // Process Emu -> Client responses
            Response::AnyPacket res;
            while (emu_client.pop(res)) {
                const Response::Packet* p = boost::polymorphic_get<Response::Packet>(&res);
                server->HandleEmuCallbacks(p);
            }
            // Process any requests that have come in since then
            server->ProcessClientEvents();
            while (client_emu.pop()) {
            }

            std::this_thread::yield();
        }
    }

    // Called by emu thread
    void HandleTiming(Request::Timing t) {
        // Check this timings callbacks to see what we need to do
        for (const auto& cb : callbacks[t]) {
            const Request::Packet* p = boost::polymorphic_get<Request::Packet>(&cb);
            if (p->sync == Request::Sync::Blocking) {
                blocking_emu = true;
            }
            HandleRequest(p);
        }
        const auto process = [&](const Request::AnyPacket& message) {
            const Request::Packet* p = boost::polymorphic_get<Request::Packet>(&message);
            if (p->method == Request::Method::Command) {
                auto* m = static_cast<const Request::Command*>(p);
                if (m->command_type == Request::CommandType::Continue) {
                    return false;
                }
            }
            return true;
        };
        client_emu.consume_all(process);
        if (blocking_emu) {
            Request::AnyPacket message;
            bool is_blocked = true;
            while (client_emu.pop(&message)) {
                is_blocked = process(message);
            }
        }
    }

private:
    void HandleRequest(const Request::Packet* r) {
        using namespace Request;
        switch (r->method) {
        case Method::Command:
            HandleCommand(static_cast<const Command*>(r));
            break;
        case Method::MemoryRead: {
            auto* p = static_cast<const MemoryRead*>(r);
            std::vector<u8> data{};
            data.reserve(p->length);
            config.memory_read_callback(config.user_data, p->address, p->length, data.data());
            emu_client.push(Response::MemoryRead(p->id, std::move(data)));
            break;
        }
        case Method::MemoryWrite: {
            auto* p = static_cast<const MemoryWrite*>(r);
            config.memory_write_callback(config.user_data, p->address, p->data.size(),
                                         p->data.data());
            emu_client.push(Response::MemoryWrite(p->id));
            break;
        }
        case Method::GPURead:
            break;
        case Method::GPUWrite:
            break;
        case Method::SpecialRead:
            break;
        case Method::SpecialWrite:
            break;
        }
    }

    void SendResponse(const Response::AnyPacket* r) {
        //
    }

    void HandleCommand(const Request::Command* c) {
        //
    }
};

EmuRPC::EmuRPC(Config config) {
    impl = new Impl(config);
    RPCServer* server = new RPCServer();
    impl->server = std::unique_ptr<RPCServer>(server);
    impl->thread = std::thread([this] { impl->ProcessEvents(); });
}

EmuRPC::~EmuRPC() {
    delete impl;
}

void EmuRPC::OnFrameEnd() {
    impl->HandleTiming(Request::Timing::FrameEnd);
}

void EmuRPC::OnMemoryAccess(EmuRPC::AccessType, u64 address) {}

void EmuRPC::OnGPUAccess(EmuRPC::AccessType, const char* field) {}

void EmuRPC::OnSpecialAccess(EmuRPC::AccessType, const char* field) {}

bool EmuRPC::CheckRange(u64 addr, u64 size) const {
    return true;
}

// C API

emurpc_state emurpc_start(struct emurpc_config config) {
    EmuRPC::Config conf{
        config.user_data,
        config.memory_read_callback,
        config.memory_write_callback,
        config.gpu_read_callback,
        config.gpu_write_callback,
        config.special_read_callback,
        config.special_write_callback,
        config.save_state_callback,
        config.load_state_callback,
        config.load_rom_callback,
        config.create_overlay_callback,
        config.draw_overlay_text_callback,
        config.enable_memory_access_timing,
        config.enable_gpu_access_timing,
        config.enable_special_access_timing,
    };
    return new EmuRPC(conf);
}

void emurpc_destroy(emurpc_state state) {
    EmuRPC* rpc = (EmuRPC*)state;
    delete rpc;
}

void emurpc_on_frame_end(emurpc_state state) {
    EmuRPC* rpc = (EmuRPC*)state;
    rpc->OnFrameEnd();
}

void emurpc_on_memory_access(emurpc_state state, struct emurpc_memory_access config) {
    EmuRPC* rpc = (EmuRPC*)state;
    rpc->OnMemoryAccess(static_cast<EmuRPC::AccessType>(config.type), config.address);
}

void emurpc_on_gpu_access(emurpc_state state, struct emurpc_gpu_access config) {
    EmuRPC* rpc = (EmuRPC*)state;
    rpc->OnGPUAccess(static_cast<EmuRPC::AccessType>(config.type), config.field_name);
}

void emurpc_on_special_access(emurpc_state state, struct emurpc_special_access config) {
    EmuRPC* rpc = (EmuRPC*)state;
    rpc->OnSpecialAccess(static_cast<EmuRPC::AccessType>(config.type), config.field_name);
}

bool emurpc_check_addr_range(emurpc_state state, u64 addr, u64 size) {
    return false;
}
