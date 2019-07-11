
#include <unordered_set>
#include <vector>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/variant/polymorphic_get.hpp>
#include <nlohmann/json.hpp>
#include "json_protocol.h"

using json = nlohmann::json;

std::vector<u8> DecodeBase64(const std::string& val) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    std::vector<u8> out{};
    std::copy(It(val.begin()), It(val.end()), std::back_inserter(out));
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
                                     });

NLOHMANN_JSON_SERIALIZE_ENUM(Method, {
                                         {Method::Command, "command"},
                                         {Method::MemoryRead, "memory_read"},
                                         {Method::MemoryWrite, "memory_write"},
                                         {Method::GPURead, "gpu_read"},
                                         {Method::GPUWrite, "gpu_write"},
                                         {Method::SpecialRead, "special_read"},
                                         {Method::SpecialWrite, "special_write"},
                                     });

NLOHMANN_JSON_SERIALIZE_ENUM(Sync, {
                                       {Sync::Blocking, "blocking"},
                                       {Sync::NonBlocking, "nonblocking"},
                                   });

NLOHMANN_JSON_SERIALIZE_ENUM(Function, {
                                           {Function::Once, "once"},
                                           {Function::Conditional, "conditional"},
                                           {Function::Callback, "callback"},
                                       });

NLOHMANN_JSON_SERIALIZE_ENUM(CommandType, {
                                              {CommandType::Continue, "continue"},
                                              {CommandType::CancelCallback, "cancel"},
                                              {CommandType::ClearCPUCache, "clear_cpu_cache"},
                                              {CommandType::SaveState, "save_state"},
                                              {CommandType::LoadState, "load_state"},
                                              {CommandType::PauseEmu, "pause_emu"},
                                              {CommandType::ResumeEmu, "resume_emu"},
                                              {CommandType::LoadRom, "load_rom"},
                                              {CommandType::CloseRom, "close_rom"},
                                              {CommandType::ResetRom, "reset_rom"},
                                              {CommandType::CreateOverlay, "create_overlay"},
                                              {CommandType::DrawOverlay, "draw_overlay"},
                                          });

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

void to_json(json& j, const MemoryRead& p) {
    j = ToBasePacket(p);
    auto params = j["params"];
    params["length"] = p.length;
    params["address"] = p.address;
}

void from_json(const json& j, MemoryRead& p) {
    auto params = FromBasePacket(j, p);
    params.at("address").get_to(p.address);
    params.at("length").get_to(p.length);
}

// TODO: will this be required?
void to_json(json& j, const AnyPacket& p) {
    switch (static_cast<PacketList>(p.which())) {
    case PacketList::MemoryWrite:
        MemoryWrite a = boost::get<MemoryWrite>(p);
        to_json(j, a);
        break;
    }
    // j = ToBasePacket(p);
}

} // namespace Request

namespace Response {

template <typename T>
void FromBasePacket(const json& j, T& p) {
    j.at("id").get_to(p.id);
    Error e{};
    auto err = j.at("error");
    err.at("code").get_to(e.code);
    err.at("message").get_to(e.message);
}

template <typename T>
json ToBasePacket(const T& p) {
    return json{
        {"jsonrpc", "2.0"},
        {"id", p.id},
    };
}

bool EncodeIfError(json& j, const Packet* p) {
    if (p->error.code == static_cast<u32>(ErrorCode::None)) {
        return false;
    }
    j["error"] = {{"code", p->error.code}, {"message", p->error.message}};
    return true;
}

void to_json(json& j, const MemoryRead& p) {
    j = ToBasePacket(p);
    if (!EncodeIfError(j, &p)) {
        j["result"] = EncodeBase64(p.result);
    }
}

void from_json(const json& j, MemoryRead& p) {
    std::string s;
    FromBasePacket(j, p);
    j.at("params").at("data").get_to(s);
    p.result = DecodeBase64(s);
}

void to_json(json& j, const MemoryWrite& p) {
    j = ToBasePacket(p);
    if (!EncodeIfError(j, &p)) {
        j["result"] = "";
    }
}

void from_json(const json& j, MemoryWrite& p) {
    FromBasePacket(j, p);
}

void to_json(json& j, const AnyPacket& packet) {
    if (const MemoryWrite* p = boost::get<MemoryWrite>(&packet)) {
        to_json(j, *p);
    } else if (const MemoryRead* p = boost::get<MemoryRead>(&packet)) {
        to_json(j, *p);
    }
}

} // namespace Response

std::vector<u8> JSONSerializer::SerializeResponse(const Response::AnyPacket& packet) {
    json j = packet;
    std::string str = j.dump();
    return {str.begin(), str.end()};
}

Request::AnyPacket JSONSerializer::DeserializeRequest(const std::vector<u8>& raw) {
    json j = json::parse(raw);

    // Check for the appropriate json rpc version
    if (j.at("jsonrpc") != "2.0") {
        throw version_mismatch_error("Version mismatch.");
    }
    Request::Method m;
    j.at("method").get_to(m);
    if (m == Request::Method::MemoryWrite) {
        Request::MemoryWrite a;
        from_json(j, a);
        return a;
    } else if (m == Request::Method::MemoryRead) {
        Request::MemoryRead a;
        from_json(j, a);
        return a;
    }
    throw invalid_method_error("Invalid method");
}

std::vector<u8> MsgPackSerializer::SerializeResponse(const Response::AnyPacket& packet) {
    // json j(packet);
    return {};
}

Request::AnyPacket MsgPackSerializer::DeserializeRequest(const std::vector<u8>& raw) {
    // json j(raw);
    return {};
}
