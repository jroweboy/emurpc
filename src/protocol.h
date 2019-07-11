
#pragma once

#include <string>
#include <vector>
#include <boost/variant.hpp>
#include "common.h"

namespace Request {
enum class Timing {
    Immediate,     /// Run this action immediately after processing this message
    FrameEnd,      /// Queue this action to happen on the next frame boundary
    MemoryAccess,  /// Hook memory access
    GPUAccess,     /// Emulator specific GPU timings such as before Vertex Stage
    SpecialAccess, /// Emulator timings that are unique to this system
};

enum class Method {
    Command, /// Special commands for the RPC server or the emulator (but not the guest application)
    MemoryRead,   /// Reads data from the specified guest addresses. The data will be in the guest's
                  /// endianness
    MemoryWrite,  /// Writes data to the specified guest addresses. The data should be in guest's
                  /// endianness
    GPURead,      /// Read data from GPU register or buffers
    GPUWrite,     /// Write data to GPU register or buffers
    SpecialRead,  /// Read the special state that this emulator provides
    SpecialWrite, /// Write the special state that this emulator provides
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

enum class CommandType {
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
    ResetRom, /// Resets currently running ROM if there is anything running
    CreateOverlay, /// Creates an overlay with the specified width/height/position and additional
                   /// parameters
    DrawOverlay,   /// Writes text to the overlay at position X, Y with additional parameters
};

class Packet {
public:
    u32 id;
    Method method;
    Timing timing;
    Sync sync;
    Function function;

    Packet();
    Packet(u32 id, Method type, Timing timing, Sync sync, Function function);

    bool operator==(const Packet& o) const {
        return std::tie(id, method, timing, sync, function) ==
               std::tie(o.id, o.method, o.timing, o.sync, o.function);
    }

    friend std::ostream& operator<<(std::ostream& out, const Packet& b) {
        return b.print(out);
    }

    // We'll rely on member function print() to do the actual printing
    // Because print is a normal member function, it can be virtualized
    virtual std::ostream& print(std::ostream& out) const {
        out << "Base";
        return out;
    }
};

class MemoryWrite : public Packet {
public:
    MemoryWrite() : Packet(), address(0), data() {}
    MemoryWrite(u32 id, Timing timing, Sync sync, Function function, u64 address,
                std::vector<u8>&& data)
        : Packet(id, Method::MemoryWrite, timing, sync, function), address(address),
          data(std::move(data)) {}

    bool operator==(const MemoryWrite& o) const {
        return (Packet::operator==(o)) & std::tie(address, data) == std::tie(o.address, o.data);
    }

    u64 address;
    std::vector<u8> data;
};

class MemoryRead : public Packet {
public:
    MemoryRead() : Packet(), address(0), length(0) {}
    MemoryRead(u32 id, Timing timing, Sync sync, Function function, u64 address, u64 length)
        : Packet(id, Method::MemoryRead, timing, sync, function), address(address), length(length) {
    }

    bool operator==(const MemoryRead& o) const {
        return (Packet::operator==(o)) & std::tie(address, length) == std::tie(o.address, o.length);
    }

    u64 address;
    u64 length;
};

/**
 * A Command is a special packet that controls other systems beside the guest application.
 */

class Command : public Packet {
public:
    CommandType command_type;
};

class SaveStateCommand : public Command {
public:
    u16 slot;
};

class LoadStateCommand : public Command {
public:
    u16 slot;
};

class LoadRomCommand : public Command {
public:
    u16 slot;
    std::string path;
};

class CreateOverlay : public Command {
public:
    u32 bg_color; /// Color of the overlay as rgba8
    u16 x;        /// X Position of the overlay starting from the top left of the render window
    u16 y;        /// Y Position of the overlay starting from the top left of the render window
    u16 width;
    u16 height;
};

class DrawOverlayText : public Command {
public:
    u32 color; /// Text color as rgba8
    u16 x;     /// X position of the text from the top left of the overlay
    u16 y;     /// Y position of the text from the top left of the overlay
    std::string text;
};

using AnyPacket =
    boost::variant<Packet, MemoryRead, MemoryWrite, SaveStateCommand, LoadStateCommand>;

enum class PacketList {
    Packet,
    MemoryRead,
    MemoryWrite,
    SaveStateCommand,
    LoadStateCommand,
};

} // namespace Request

namespace Response {

enum class ResponseType {
    MemoryRead,
    MemoryWrite,
};

enum class ErrorCode {
    None,
    Mismatch,      /// Returned if the client version is a mismatch. The server should close the
                   /// connection afterwards.
    InvalidHeader, /// If any of the client header fields do not match any enum values
    Unsupported,   /// If the server doesn't support this request
};

struct Error {
    u32 code;
    std::string message;
};

extern Error NoneError;
extern Error MismatchError;
extern Error InvalidHeaderError;
extern Error UnsupportedError;

class Packet {
public:
    Packet();
    Packet(u32 id, Error error = NoneError);
    u32 id;

    Error error;
};

class MemoryRead : public Packet {
public:
    MemoryRead(u32 id, std::vector<u8>&& result);

    std::vector<u8> result;
};

class MemoryWrite : public Packet {
public:
    MemoryWrite(u32 id);
};

using AnyPacket = boost::variant<Packet, MemoryRead, MemoryWrite>;

} // namespace Response

class ProtocolSerializer {
public:
    virtual std::vector<u8> SerializeResponse(const Response::AnyPacket&) = 0;
    virtual Request::AnyPacket DeserializeRequest(const std::vector<u8>&) = 0;

private:
};
