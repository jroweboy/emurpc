
#pragma once

#include "common.h"

class EmuRPC {
public:
    // Callbacks for Client -> Guest application
    using MemoryReadCallback = void (*)(void* user_data, u64 address, u64 size, u8* out);
    using MemoryWriteCallback = void (*)(void* user_data, u64 address, u64 size, const u8* data);
    using GPUReadCallback = void (*)(void* user_data, const char* field, u8* out);
    using GPUWriteCallback = void (*)(void* user_data, const char* field, u64 size, const u8* data);
    using SpecialReadCallback = void (*)(void* user_data, const char* field, u8* out);
    using SpecialWriteCallback = void (*)(void* user_data, const char* field, u64 size,
                                          const u8* data);

    // Callbacks for Client -> Emulator communication
    using LoadStateCallback = bool (*)(void* user_data, u16 save_slot);
    using SaveStateCallback = bool (*)(void* user_data, u16 save_slot);
    using LoadRomCallback = bool (*)(void* user_data, const char* filename);
    using CreateOverlayCallback = bool (*)(void* user_data, u32 bg_color, u16 x, u16 y, u32 width,
                                           u32 height);
    using DrawOverlayTextCallback = bool (*)(void* user_data, u32 color, u16 x, u16 y,
                                             const char* text);

    struct Config {
        void* user_data; /// Extra data passed into the callbacks

        MemoryReadCallback memory_read_callback;
        MemoryWriteCallback memory_write_callback;
        GPUReadCallback gpu_read_callback;
        GPUWriteCallback gpu_write_callback;
        SpecialReadCallback special_read_callback;
        SpecialWriteCallback special_write_callback;

        SaveStateCallback save_state_callback;
        LoadStateCallback load_state_callback;
        LoadRomCallback load_rom_callback;
        CreateOverlayCallback create_overlay_callback;
        DrawOverlayTextCallback draw_overlay_text_callback;

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

    void OnMemoryAccess(AccessType, u64 address, u64 size);

    void OnGPUAccess(AccessType, const char* field);

    void OnSpecialAccess(AccessType, const char* field);

    // Helper methods for accessors
    bool CheckRange(u64 addr, u64 size) const;

private:
    class Impl;
    Impl* impl;
};
