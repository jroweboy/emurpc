
#pragma once

#include <memory>
#include <random>
#include <vector>
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
        // write random data
        for (int i = 0; i < memory.size() / access_size; i += 16 * access_size) {
            std::vector<u8> rand_data(access_size);
            std::generate(rand_data.begin(), rand_data.end(), [&]() { return dist(generator); });
            WriteMemory(access_size * i, access_size, rand_data.data());
        }
        // and read it too
        for (int i = 0; i < memory.size() / access_size; i += 16 * access_size) {
            std::vector<u8> rand_data(access_size);
            ReadMemory(access_size * i, access_size, rand_data.data());
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
