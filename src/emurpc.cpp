
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <boost/lockfree/spsc_queue.hpp>
#include "callback.h"
#include "emurpc.h"
#include "emurpc.hpp"
#include "rpc_server.h"

class EmuRPC::Impl {
public:
    Config config;
    std::atomic_bool stopped = false;
    std::atomic_bool blocking_emu = false;

    std::vector<FrameEndCallback> frame_callback;
    std::vector<MemoryCallback> memory_callback;
    std::vector<GPUCallback> gpu_callback;
    std::vector<SpecialCallback> special_callback;

    boost::lockfree::spsc_queue<EmuToClientMessage, boost::lockfree::capacity<1024>> emu_client;
    boost::lockfree::spsc_queue<ClientToEmuMessage, boost::lockfree::capacity<1024>> client_emu;
    std::thread thread;
    std::unique_ptr<RPCServer> server = nullptr;

    Impl() = default;

    ~Impl() {
        stopped = true;
        thread.join();
    }

    // called by server thread
    void ProcessEvents() {
        EmuToClientMessage message;
        while (!stopped) {
            while (emu_client.pop(message)) {
                server->HandleEmuCallbacks(message);
            }
            std::this_thread::yield();
        }
    }

    // Called by emu thread
    void HandleTiming(EmuToClientMessage message) {
        // Check this timings callbacks to see what we need to do
        if (FrameEnd* m = boost::get<FrameEnd>(&message)) {
            for (const auto& cb : frame_callback) {
                if (cb.IsBlocking()) {
                    blocking_emu = true;
                }
                HandleCallback(&cb);
            }
        }
        const auto& process = [&](ClientToEmuMessage m) { blocking_emu = true; };
        client_emu.consume_all(process);
        if (blocking_emu) {
            ClientToEmuMessage message;
        }
    }

private:
    void HandleCallback(const Callback* cb) {}
};

EmuRPC::EmuRPC(Config config) {
    impl = new Impl;
    impl->config = config;
    RPCServer* server = new RPCServer();
    impl->server = std::unique_ptr<RPCServer>(server);
    impl->thread = std::thread([this] { impl->ProcessEvents(); });
}

EmuRPC::~EmuRPC() {
    delete impl;
}

void EmuRPC::OnFrameEnd() {
    EmuToClientMessage m{FrameEnd{}};
    impl->emu_client.push(m);
    impl->HandleTiming(m);
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
