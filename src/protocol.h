#ifndef EMURPC_PROTOCOL_H
#define EMURPC_PROTOCOL_H

#include "types.h"

enum emurpc_request_timing {
    EMURPC_TIMING_IMMEDIATE, /// Run this action immediately after processing this message
    EMURPC_TIMING_FRAME_END, /// Queue this action to happen on the next frame boundary
    EMURPC_TIMING_MEMORY_ACCESS, /// Hook memory access 
    EMURPC_TIMING_GPU, /// Emulator specific GPU timings such as before Vertex Stage
    EMURPC_TIMING_SPECIAL, /// Emulator timings that are unique to this system
};

enum emurpc_request_type {
    EMURPC_TYPE_COMMAND, /// Special commands for the RPC server or the emulator (but not the guest application)
    EMURPC_TYPE_MEMORY_READ, /// Reads data from the specified guest addresses. The data will be in the guest's endianness
    EMURPC_TYPE_MEMORY_WRITE, /// Writes data to the specified guest addresses. The data should be in guest's endianness
    EMURPC_TYPE_REGISTER_READ, /// Read the CPU register state for the provided register
    EMURPC_TYPE_REGISTER_WRITE, /// Write to CPU register provided
    EMURPC_TYPE_GPU_READ, /// Read data from GPU register or buffers
    EMURPC_TYPE_GPU_WRITE, /// Write data to GPU register or buffers
};

enum emurpc_request_sync {
    EMURPC_SYNC_BLOCKING_RPC, /// The server stops processing queued non-command requests until a Continue Command packet is received
    EMURPC_SYNC_BLOCKING_EMU, /// The server blocks the guest application but continues processing queued requests until a Continue Command is received
    EMURPC_SYNC_BLOCKING_ALL, /// The server should stop processing queued non-command packets AND blocks the guest application until a Continue Command packet is received
    EMURPC_SYNC_NONBLOCKING, /// The server should immediately return to processing packets after handling this request without blocking the emulation
};

enum emurpc_request_function {
    EMURPC_FUNCTION_ONCE, /// Run this action only once.
    EMURPC_FUNCTION_CONDITIONAL, /// Only run this action if the condition is met.
    EMURPC_FUNCTION_CALLBACK, /// Configure the server to respond every time the condition is met (or always called if no condition is provided)
                              /// The Callback can be cancelled with a CancelCallback command packet passing in the ID for the Callback
};

enum emurpc_request_commandtype {
    EMURPC_COMMANDTYPE_CONTINUE, /// Special packet to resume the server after a Blocking(RPC/Emu/All) call is handled
    EMURPC_COMMANDTYPE_CANCEL_CALLBACK, /// Cancel the callback with the provided ID.
    EMURPC_COMMANDTYPE_CLEAR_CPU_CACHE, /// Requests that the emulator clear any cached CPU instructions, useful when a script overwrites exectuable memory
    EMURPC_COMMANDTYPE_SAVE_STATE, /// Saves the state of the game to the slot provided
    EMURPC_COMMANDTYPE_LOAD_STATE, /// Loads game state from the slot provided
    EMURPC_COMMANDTYPE_PAUSE_EMU, /// Pauses the emulated application
    EMURPC_COMMANDTYPE_RESUME_EMU, /// Resumes the emulated application
    EMURPC_COMMANDTYPE_LOAD_ROM, /// Launches the ROM provided by path. If a ROM is running, closes the previous ROM before launching.
    EMURPC_COMMANDTYPE_CLOSE_ROM, /// Closes currently running ROM if there is anything running
    EMURPC_COMMANDTYPE_CREATE_OVERLAY, /// Creates an overlay with the specified width/height/position and additional parameters
    EMURPC_COMMANDTYPE_DRAW_OVERLAY, /// Writes text to the overlay at position X, Y with additional parameters
};

struct emurpc_request {
    uint32_t id; /// Each request has an ID and the server will respond with the ID passed it. Sequential IDs are good enough.
    enum emurpc_request_type type;
    enum emurpc_request_timing timing;
    enum emurpc_request_sync sync;
    enum emurpc_request_function function;
};


struct emurpc_request_memory_write {
    struct emurpc_request header;
    uint64_t address;
    uint64_t length;
    uint8_t* data;
};

struct emurpc_request_memory_read {
    struct emurpc_request header;
    uint64_t address;
    uint64_t length;
};

/**
 * A Command is a special packet that controls other systems beside the guest application.
 */

struct emurpc_request_command {
    struct emurpc_request header;
    enum emurpc_request_commandtype command_type;
};

struct emurpc_request_command_save_state {
	struct emurpc_request_command cmd_header;
    uint16_t slot;
};

struct emurpc_request_command_load_state {
	struct emurpc_request_command cmd_header;
    uint16_t slot;
};

struct emurpc_request_command_load_rom {
	struct emurpc_request_command cmd_header;
    uint16_t slot;
    const char* path;
};

struct emurpc_request_command_create_overlay {
	struct emurpc_request_command cmd_header;
    uint32_t bg_color; /// Color of the overlay as rgba8
    uint16_t x; /// X Position of the overlay starting from the top left of the render window
    uint16_t y; /// Y Position of the overlay starting from the top left of the render window
    uint16_t width;
    uint16_t height;
};


struct emurpc_request_command_draw_overlay_text {
	struct emurpc_request_command cmd_header;
    uint32_t color; /// Text color as rgba8
    uint16_t x; /// X position of the text from the top left of the overlay
    uint16_t y; /// Y position of the text from the top left of the overlay
    const char* text;
};

enum emurpc_response_type {
    EMURPC_RESPONSE_TYPE_SUCCESS, /// Empty response that just returns the ID of the packet that succeeded
    EMURPC_RESPONSE_TYPE_ERROR, /// General error, check ErrorType for more information about the error
    EMURPC_RESPONSE_TYPE_CALLBACK, /// Server is processing a callback request. The ID of the packet refers to the original callback or repeat that was sent earlier
    EMURPC_RESPONSE_TYPE_COMMAND, /// Special packets sent by the server for informational purposes (such as game start/end)
};

enum emurpc_response_errortype {
    EMURPC_RESPONSE_ERRORTYPE_NONE,
    EMURPC_RESPONSE_ERRORTYPE_VERSION_MISMATCH, /// Returned if the client version is a mismatch. The server should close the connection afterwards.
    EMURPC_RESPONSE_ERRORTYPE_INVALID_HEADER, /// If any of the client header fields do not match any enum values
    EMURPC_RESPONSE_ERRORTYPE_UNSUPPORTED, /// If the server doesn't support this request
};

struct emurpc_response_message {
    uint32_t client_id; /// ID of the packet that this is a Response for
    enum emurpc_response_type type;
    enum emurpc_response_errortype error;
};

#endif
