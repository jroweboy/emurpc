#ifndef EMURPC_EMURPC_H
#define EMURPC_EMURPC_H

#include "emurpc/types.h"

/**
 * 
 */
typedef bool (*emurpc_save_state_callback)(uint16_t save_slot, void* user_data);

/**
 * 
 */
typedef bool (*emurpc_load_state_callback)(void* user_data, uint16_t load_slot);

/**
 * 
 * Return false if the rom could not be loaded
 */
typedef bool (*emurpc_load_rom_callback)(void* user_data, const char* filename);

struct emurpc_params_create_overlay {
    uint32_t bg_color; /// Color of the overlay as rgba8
    uint16_t x; /// X Position of the overlay starting from the top left of the render window
    uint16_t y; /// Y Position of the overlay starting from the top left of the render window
    uint32_t width;
    uint32_t height;
};
typedef bool (*emurpc_callback_create_overlay)(void* user_data, struct emurpc_params_create_overlay);

struct emurpc_params_draw_overlay_text {
    uint32_t color; /// Text color as rgba8
    uint16_t x; /// X position of the text from the top left of the overlay
    uint16_t y; /// Y position of the text from the top left of the overlay
    const char* text;
};
typedef bool (*emurpc_callback_draw_overlay_text)(void* user_data, struct emurpc_params_draw_overlay_text);

struct emurpc_config {
	emurpc_save_state_callback save_state_callback;
    emurpc_load_state_callback load_state_callback;
    emurpc_load_rom_callback load_rom_callback;
	void* user_data; // Extra data passed into the callbacks

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
	uint64_t address;
};

struct emurpc_gpu_access {
	enum emurpc_access_type type;
	char* field_name;
};

struct emurpc_special_access {
	enum emurpc_access_type type;
	char* field_name;
};

struct emurpc_state;
struct emurpc_state* emurpc_start(struct emurpc_config);

void emurpc_destroy(struct emurpc_state*);

void emurpc_process_events(struct emurpc_state*);

void emurpc_on_frame_end(struct emurpc_state*);

void emurpc_on_memory_access(struct emurpc_state*, struct emurpc_memory_access);

void emurpc_on_gpu_access(struct emurpc_state*, struct emurpc_gpu_access);

void emurpc_on_special_access(struct emurpc_state*, struct emurpc_special_access);

// Helper methods for accesors

bool emurpc_check_addr_range(struct emurpc_state*, uint64_t addr, uint64_t size);

#endif
