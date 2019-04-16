
#include "protocol.h"

namespace Request {

Packet::Packet() = default;

Packet::Packet(u32 id, Method method, Timing timing, Sync sync, Function function)
    : id(id), method(method), timing(timing), sync(sync), function(function) {}

} // namespace Request

namespace Response {

Error NoneError{
    static_cast<u32>(ErrorCode::None),
    "Sentinal value",
};
Error MismatchError{
    static_cast<u32>(ErrorCode::Mismatch),
    "Client version and server version does not match",
};
Error InvalidHeaderError{
    static_cast<u32>(ErrorCode::InvalidHeader),
    "Request was malformed",
};
Error UnsupportedError{
    static_cast<u32>(ErrorCode::Unsupported),
    "Unsupported operation",
};

Packet::Packet() = default;

Packet::Packet(u32 id, Error error) : id(id), error(error) {}

MemoryRead::MemoryRead(u32 id, std::vector<u8>&& result) : Packet(id), result(std::move(result)) {}

MemoryWrite::MemoryWrite(u32 id) : Packet(id) {}

} // namespace Response
