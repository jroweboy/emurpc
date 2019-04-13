
#include "protocol.h"

namespace Request {

Packet::Packet() = default;

Packet::Packet(u32 id, Method method, Timing timing, Sync sync, Function function)
    : id(id), method(method), timing(timing), sync(sync), function(function) {}

} // namespace Request

namespace Response {

Packet::Packet() = default;

Packet::Packet(u32 client_id, Type type, ErrorType error)
    : client_id(client_id), type(type), error(error) {}

MemoryRead::MemoryRead(u32 client_id, std::vector<u8>&& data, Type type, ErrorType error)
    : Packet(client_id, type, error), data(std::move(data)) {}

MemoryWrite::MemoryWrite(u32 client_id, Type type, ErrorType error)
    : Packet(client_id, type, error) {}

} // namespace Response
