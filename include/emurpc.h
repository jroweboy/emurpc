#ifndef EMURPC_EMURPC_H
#define EMURPC_EMURPC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

typedef u8* (*emurpc_memory_read_callback)(void* user_data, u64 address, u64 size);
typedef void (*emurpc_memory_write_callback)(void* user_data, u64 address, u64 size, u8* data);
typedef u8* (*emurpc_gpu_read_callback)(void* user_data, const char* field);
typedef void (*emurpc_gpu_write_callback)(void* user_data, const char* field, u64 size, u8* data);
typedef u8* (*emurpc_special_read_callback)(void* user_data, const char* field);
typedef void (*emurpc_special_write_callback)(void* user_data, const char* field, u64 size,
                                              u8* data);

typedef bool (*emurpc_save_state_callback)(void* user_data, u16 save_slot);

typedef bool (*emurpc_load_state_callback)(void* user_data, u16 load_slot);

typedef bool (*emurpc_load_rom_callback)(void* user_data, const char* filename);

typedef bool (*emurpc_create_overlay_callback)(void* user_data,
                                               u32 bg_color, /// Color of the overlay as rgba8
                                               u16 x, /// X Position of the overlay starting from
                                                      /// the top left of the render window
                                               u16 y, /// Y Position of the overlay starting from
                                                      /// the top left of the render window
                                               u32 width, u32 height);

typedef bool (*emurpc_draw_overlay_text_callback)(
    void* user_data,
    u32 color, /// Text color as rgba8
    u16 x,     /// X position of the text from the top left of the overlay
    u16 y,     /// Y position of the text from the top left of the overlay
    const char* text);

struct emurpc_config {
    void* user_data; /// Extra data passed into the callbacks

    /// Callbacks for Client->Guest Application communication
    emurpc_memory_read_callback memory_read_callback;
    emurpc_memory_write_callback memory_write_callback;
    emurpc_gpu_read_callback gpu_read_callback;
    emurpc_gpu_write_callback gpu_write_callback;
    emurpc_special_read_callback special_read_callback;
    emurpc_special_write_callback special_write_callback;

    /// Callbacks for Client->Emulator communication
    emurpc_save_state_callback save_state_callback;
    emurpc_load_state_callback load_state_callback;
    emurpc_load_rom_callback load_rom_callback;
    emurpc_create_overlay_callback create_overlay_callback;
    emurpc_draw_overlay_text_callback draw_overlay_text_callback;

    bool enable_memory_access_timing;
    bool enable_gpu_access_timing;
    bool enable_special_access_timing;
};

enum emurpc_access_type {
    EMURPC_ACCESS_TYPE_READ,
    EMURPC_ACCESS_TYPE_WRITE,
};

struct emurpc_memory_access {
    enum emurpc_access_type type;
    u64 address;
};

struct emurpc_gpu_access {
    enum emurpc_access_type type;
    char* field_name;
};

struct emurpc_special_access {
    enum emurpc_access_type type;
    char* field_name;
};

typedef void* emurpc_state;

emurpc_state emurpc_start(struct emurpc_config);

void emurpc_destroy(emurpc_state);

void emurpc_process_events(emurpc_state);

void emurpc_on_frame_end(emurpc_state);

void emurpc_on_memory_access(emurpc_state, struct emurpc_memory_access);

void emurpc_on_gpu_access(emurpc_state, struct emurpc_gpu_access);

void emurpc_on_special_access(emurpc_state, struct emurpc_special_access);

// Helper methods for accesors

bool emurpc_check_addr_range(emurpc_state, u64 addr, u64 size);

#ifdef __cplusplus
}
#endif

#endif
