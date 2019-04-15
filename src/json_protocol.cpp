
#include <unordered_set>
#include <vector>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <nlohmann/json.hpp>
#include "json_protocol.h"

using json = nlohmann::json;

std::vector<u8> DecodeBase64(const std::string& val) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    std::vector<u8> out{};
    std::copy(It(val.c_str()), It(val.c_str() + val.size()), std::back_inserter(out));
    return out;
}

std::string EncodeBase64(const std::vector<u8>& val) {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::vector<u8>::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
}

namespace Request {
NLOHMANN_JSON_SERIALIZE_ENUM(Timing, {
                                         {Timing::Immediate, "immediate"},
                                         {Timing::FrameEnd, "frame_end"},
                                         {Timing::MemoryAccess, "memory_access"},
                                         {Timing::GPUAccess, "gpu_access"},
                                         {Timing::SpecialAccess, "special_access"},
                                     })

template <typename T>
json FromBasePacket(const json& j, T& p) {
    j.at("id").get_to(p.id);
    j.at("method").get_to(p.method);
    auto params = j.at("params");
    params.at("timing").get_to(p.timing);
    params.at("sync").get_to(p.sync);
    params.at("function").get_to(p.function);
    return params;
}

template <typename T>
json ToBasePacket(const T& p) {
    return json{{"jsonrpc", "2.0"},
                {"method", p.method},
                {"id", p.id},
                {
                    "params",
                    {
                        {"timing", p.timing},
                        {"sync", p.sync},
                        {"function", p.function},
                    },
                }};
}

void to_json(json& j, const MemoryWrite& p) {
    j = ToBasePacket(p);
    auto params = j["params"];
    params["data"] = EncodeBase64(p.data);
    params["address"] = p.address;
}

void from_json(const json& j, MemoryWrite& p) {
    std::string s;
    auto params = FromBasePacket(j, p);
    params.at("data").get_to(s);
    p.data = DecodeBase64(s);
    params.at("address").get_to(p.address);
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
