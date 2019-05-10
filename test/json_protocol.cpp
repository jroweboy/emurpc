#include <catch.hpp>
#include <nlohmann/json.hpp>
#include "json_protocol.h"

using json = nlohmann::json;

TEST_CASE("Response JSON De/Serialization", "[json_serializer]") {
    JSONSerializer ser{};
    Response::AnyPacket p;
    std::vector<u8> data;
    std::vector<u8> serial;
    json j;

    SECTION("Response::MemoryRead") {
        data = {'H', 'E', 'L', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D', '!'};
        j = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "result": "SEVMTE8gV09STEQh"
})a"_json;
        p = Response::MemoryRead{1, std::move(data)};
		serial = ser.SerializeResponse(p);
        REQUIRE(json::parse(serial.begin(), serial.end()) == j);
    }

    SECTION("Response::MemoryWrite") {
        j = R"a({
  "jsonrpc": "2.0",
  "id": 1
})a"_json;
        p = Response::MemoryWrite(1);
        serial = ser.SerializeResponse(p);
        REQUIRE(json::parse(serial.begin(), serial.end()) == j);
    }

}

TEST_CASE("Request JSON De/Serialization", "[json_serializer]") {
	using namespace Request;
    JSONSerializer ser{};
    Request::AnyPacket p;
    Request::AnyPacket deser;
    json j;
    SECTION("Request::MemoryRead") {
        j = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "method": "memory_read",
  "params": {
    "timing": "immediate",
    "sync": "nonblocking",
    "function": "once",
    "address": 1000,
    "length": 64,
  }
})a"_json;
		p = MemoryRead(1, Timing::Immediate, Sync::Blocking, Function::Once, 1000, 64);
		std::string j_str = j.dump();
		std::vector<u8> bytes{j_str.begin(), j_str.end()};
		deser = ser.DeserializeRequest(bytes);
		REQUIRE(p == deser);
    }
}
