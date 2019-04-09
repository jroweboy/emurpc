
#include <unordered_set>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <nlohmann/json.hpp>
#include "json_protocol.h"

using json = nlohmann::json;

namespace Request {
NLOHMANN_JSON_SERIALIZE_ENUM(Timing, {
                                         {Timing::Immediate, "immediate"},
                                         {Timing::FrameEnd, "frame_end"},
                                         {Timing::MemoryAccess, "memory_access"},
                                         {Timing::GPUAccess, "gpu_access"},
                                         {Timing::SpecialAccess, "special_access"},
                                     })

void to_json(json& j, const MemoryWrite& p) {
    j = json{
        {"jsonrpc", "2.0"},
        {"method", p.method},
        {"id", p.id},
    };
}

void from_json(const json& j, MemoryWrite& p) {
    j.at("id").get_to(p.id);
    j.at("method").get_to(p.method);
    auto params = j.at("params");
    params.at("timing").get_to(p.timing);
    params.at("sync").get_to(p.sync);
    params.at("function").get_to(p.function);
    params.at("address").get_to(p.address);
	std::string s;
    params.at("data").get_to(s);
}
} // namespace Request

std::vector<u8> JSONSerializer::SerializeResponse(Response::AnyPacket&& packet) {
    // json j(packet);
    return {};
}

Request::AnyPacket JSONSerializer::DeserializeRequest(std::vector<u8>&& raw) {
    json j(raw);
    return {};
}

std::vector<u8> MsgPackSerializer::SerializeResponse(Response::AnyPacket&& packet) {
    // json j(packet);
    return {};
}

Request::AnyPacket MsgPackSerializer::DeserializeRequest(std::vector<u8>&& raw) {
    // json j(raw);
    return {};
}
