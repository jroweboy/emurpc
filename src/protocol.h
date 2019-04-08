
#pragma once

#include <vector>
#include "common.h"

namespace Request {
enum class Timing {
    Immediate,     /// Run this action immediately after processing this message
    FrameEnd,      /// Queue this action to happen on the next frame boundary
    MemoryAccess,  /// Hook memory access
    GPUAccess,     /// Emulator specific GPU timings such as before Vertex Stage
    SpecialAccess, /// Emulator timings that are unique to this system
};

enum class Type {
    Command, /// Special commands for the RPC server or the emulator (but not the guest application)
    MemoryRead,   /// Reads data from the specified guest addresses. The data will be in the guest's
                  /// endianness
    MemoryWrite,  /// Writes data to the specified guest addresses. The data should be in guest's
                  /// endianness
    RegisterRead, /// Read the CPU register state for the provided register
    RegisterWrite, /// Write to CPU register provided
    GPURead,       /// Read data from GPU register or buffers
    GPUWrite,      /// Write data to GPU register or buffers
};

enum class Sync {
    Blocking, /// The server stops processing queued non-command requests until a Continue Command
              /// packet is received
    NonBlocking, /// The server should immediately return to processing packets after handling this
                 /// request without blocking the emulation
};

enum class Function {
    Once,        /// Run this action only once.
    Conditional, /// Only run this action if the condition is met.
    Callback, /// Configure the server to respond every time the condition is met (or always called
              /// if no condition is provided) The Callback can be cancelled with a CancelCallback
              /// command packet passing in the ID for the Callback
};

enum CommandType {
    Continue,       /// Special packet to resume the server after a Blocking call is handled
    CancelCallback, /// Cancel the callback with the provided ID.
    ClearCPUCache,  /// Requests that the emulator clear any cached CPU instructions, useful when a
                    /// script overwrites exectuable memory
    SaveState,      /// Saves the state of the game to the slot provided
    LoadState,      /// Loads game state from the slot provided
    PauseEmu,       /// Pauses the emulated application
    ResumeEmu,      /// Resumes the emulated application
    LoadRom,  /// Launches the ROM provided by path. If a ROM is running, closes the previous ROM
              /// before launching.
    CloseRom, /// Closes currently running ROM if there is anything running
    CreateOverlay, /// Creates an overlay with the specified width/height/position and additional
                   /// parameters
    DrawOverlay,   /// Writes text to the overlay at position X, Y with additional parameters
};

struct Header {
    u32 id; /// Each request has an ID and the server will respond with the ID passed it.
            /// Sequential IDs are good enough.
    Type type;
    Timing timing;
    Sync sync;
    Function function;
};

struct MemoryWrite : Header {
    u64 address;
    std::vector<u8> data;
};

struct MemoryRead : Header {
    u64 address;
    u64 length;
};

/**
 * A Command is a special packet that controls other systems beside the guest application.
 */

struct Command : Header {
    CommandType command_type;
};

struct SaveStateCommand : Command {
    u16 slot;
};

struct LoadStateCommand : Command {
    u16 slot;
};

struct LoadRomCommand : Command {
    u16 slot;
    std::string path;
};

struct CreateOverlay : Command {
    u32 bg_color; /// Color of the overlay as rgba8
    u16 x;        /// X Position of the overlay starting from the top left of the render window
    u16 y;        /// Y Position of the overlay starting from the top left of the render window
    u16 width;
    u16 height;
};

struct DrawOverlayText : Command {
    u32 color; /// Text color as rgba8
    u16 x;     /// X position of the text from the top left of the overlay
    u16 y;     /// Y position of the text from the top left of the overlay
    std::string text;
};

} // namespace Request

namespace Response {

enum Type {
    Success,  /// Empty response that just returns the ID of the packet that succeeded
    Error,    /// General error, check ErrorType for more information about the error
    Callback, /// Server is processing a callback request. The ID of the packet refers to the
              /// original callback or repeat that was sent earlier
    Command,  /// Special packets sent by the server for informational purposes (such as game
              /// start/end)
};

enum ErrorType {
    None,
    Mismatch,      /// Returned if the client version is a mismatch. The server should close the
                   /// connection afterwards.
    InvalidHeader, /// If any of the client header fields do not match any enum values
    Unsupported,   /// If the server doesn't support this request
};

struct Header {
    u32 client_id; /// ID of the packet that this is a Response for
    Type type;
    ErrorType error;
};

} // namespace Response
