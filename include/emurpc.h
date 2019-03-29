#ifndef EMURPC_EMURPC_H
#define EMURPC_EMURPC_H

#include "emurpc/types.h"

/**
 * 
 */
typedef bool (*emurpc_save_state_callback)(uint16_t save_slot);

/**
 * 
 */
typedef bool (*emurpc_load_state_callback)(uint16_t load_slot);

/**
 * 
 * Return false if the rom could not be loaded
 */
typedef bool (*emurpc_load_rom_callback)(const char* filename);

struct emurpc_params_create_overlay {
    uint32_t bg_color; /// Color of the overlay as rgba8
    uint16_t x; /// X Position of the overlay starting from the top left of the render window
    uint16_t y; /// Y Position of the overlay starting from the top left of the render window
    uint32_t width;
    uint32_t height;
};
typedef bool (*emurpc_callback_create_overlay)(struct emurpc_params_create_overlay);

struct emurpc_params_draw_overlay_text {
    uint32_t color; /// Text color as rgba8
    uint16_t x; /// X position of the text from the top left of the overlay
    uint16_t y; /// Y position of the text from the top left of the overlay
    const char* text;
};
typedef bool (*emurpc_callback_draw_overlay_text)(struct emurpc_params_draw_overlay_text);

/**
 * 
 */
struct emurpc_config {
    emurpc_save_state_callback save_state_callback;
    emurpc_load_state_callback load_state_callback;
    emurpc_load_rom_callback load_rom_callback;

    /**
     * 
     */
    bool enable_memory_access_timing;
    bool enable_gpu_access_timing;
    bool enable_register_access_timing;
};

struct emurpc_state_impl;
struct emurpc_state {
    struct emurpc_state_impl* impl;
};

enum emurpc_access_type {
    EMURPC_ACCESS_TYPE_READ,
    EMURPC_ACCESS_TYPE_WRITE,
};

bool emurpc_init(struct emurpc_state*, struct emurpc_config);

bool emurpc_process_events(struct emurpc_state*);

void emurpc_on_frame_end(struct emurpc_state*);

void emurpc_on_memory_access(struct emurpc_state*, enum emurpc_access_type);

void emurpc_on_gpu_access(struct emurpc_state*, enum emurpc_access_type);

void emurpc_on_register_access(struct emurpc_state*, enum emurpc_access_type);

void emurpc_shutdown(struct emurpc_state*);

#endif
