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
  "id": 1,
  "result": ""
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
    std::vector<u8> data;
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
    "length": 64
  }
})a"_json;
        p = MemoryRead(1, Timing::Immediate, Sync::NonBlocking, Function::Once, 1000, 64);
        std::string j_str = j.dump();
        std::vector<u8> bytes{j_str.begin(), j_str.end()};
        deser = ser.DeserializeRequest(bytes);
        REQUIRE(p == deser);
    }

    SECTION("Request::MemoryWrite") {
        j = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "method": "memory_write",
  "params": {
    "timing": "immediate",
    "sync": "nonblocking",
    "function": "once",
    "address": 1000,
	"data": "SEVMTE8gV09STEQh"
  }
})a"_json;
        data = {'H', 'E', 'L', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D', '!'};
        p = MemoryWrite(1, Timing::Immediate, Sync::NonBlocking, Function::Once, 1000,
                        std::move(data));
        std::string j_str = j.dump();
        std::vector<u8> bytes{j_str.begin(), j_str.end()};
        deser = ser.DeserializeRequest(bytes);
        REQUIRE(p == deser);
    }
}

// Data sent from client isn't even valid JSON
TEST_CASE("Unparseable Request", "[json_serializer]") {
    using namespace Request;
    JSONSerializer ser{};
    Request::AnyPacket p;
    Request::AnyPacket deser;
    std::string str;

    SECTION("Garbage data") {
        str = "akjsbndfljkhabs";
        std::vector<u8> bytes{str.begin(), str.end()};
        REQUIRE_THROWS_AS(ser.DeserializeRequest(bytes), json::exception);
    }

    SECTION("Empty Request") {
        str = "";
        std::vector<u8> bytes{str.begin(), str.end()};
        REQUIRE_THROWS_AS(ser.DeserializeRequest(bytes), json::exception);
    }

    SECTION("String with null") {
        str = "\0";
        std::vector<u8> bytes{str.begin(), str.end()};
        REQUIRE_THROWS_AS(ser.DeserializeRequest(bytes), json::exception);
    }
}

// Data sent from client is valid JSON but is invalid request
TEST_CASE("Invalid Request") {
    using namespace Request;
    JSONSerializer ser{};
    Request::AnyPacket p;
    Request::AnyPacket deser;
    json j;
    SECTION("Wrong jsonrpc version") {
        j = R"a({
  "jsonrpc": "2.1",
  "id": 1,
  "method": "memory_read",
  "params": {
    "timing": "immediate",
    "sync": "nonblocking",
    "function": "once",
    "address": 1000,
    "length": 64
  }
})a"_json;
        std::string j_str = j.dump();
        std::vector<u8> bytes{j_str.begin(), j_str.end()};
        REQUIRE_THROWS_AS(ser.DeserializeRequest(bytes), version_mismatch_error);
    }

    SECTION("Empty JSON request") {
        j = "{}";
        std::string j_str = j.dump();
        std::vector<u8> bytes{j_str.begin(), j_str.end()};
        REQUIRE_THROWS_AS(ser.DeserializeRequest(bytes), json::exception);
    }
}