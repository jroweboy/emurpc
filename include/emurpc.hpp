
#pragma once

#include "common.h"

class EmuRPC {
public:
    // Callbacks for Client -> Guest application
    using MemoryRead = u8* (*)(void* user_data, u64 address, u64 size);
    using MemoryWrite = void (*)(void* user_data, u64 address, u64 size, u8* data);
    using GPURead = u8* (*)(void* user_data, const char* field);
    using GPUWrite = void (*)(void* user_data, const char* field, u64 size, u8* data);
    using SpecialRead = u8* (*)(void* user_data, const char* field);
    using SpecialWrite = void (*)(void* user_data, const char* field, u64 size, u8* data);

    // Callbacks for Client -> Emulator communication
    using LoadState = bool (*)(void* user_data, u16 save_slot);
    using SaveState = bool (*)(void* user_data, u16 save_slot);
    using LoadRom = bool (*)(void* user_data, const char* filename);
    using CreateOverlay = bool (*)(void* user_data, u32 bg_color, u16 x, u16 y, u32 width,
                                   u32 height);
    using DrawOverlayText = bool (*)(void* user_data, u32 color, u16 x, u16 y, const char* text);

    struct Config {
        void* user_data; /// Extra data passed into the callbacks

        MemoryRead memory_read_callback;
        MemoryWrite memory_write_callback;
        GPURead gpu_read_callback;
        GPUWrite gpu_write_callback;
        SpecialRead special_read_callback;
        SpecialWrite special_write_callback;

        SaveState save_state_callback;
        LoadState load_state_callback;
        LoadRom load_rom_callback;
        CreateOverlay create_overlay_callback;
        DrawOverlayText draw_overlay_text_callback;

        bool enable_memory_access_timing;
        bool enable_gpu_access_timing;
        bool enable_special_access_timing;
    };

    explicit EmuRPC(Config);
    ~EmuRPC();

    // Emulator -> Client communication
    enum class AccessType {
        Read,
        Write,
    };

    void OnFrameEnd();

    void OnMemoryAccess(AccessType, u64 address);

    void OnGPUAccess(AccessType, const char* field);

    void OnSpecialAccess(AccessType, const char* field);

    // Helper methods for accessors

    bool CheckRange(u64 addr, u64 size) const;

private:
    class Impl;
    Impl* impl;
};
