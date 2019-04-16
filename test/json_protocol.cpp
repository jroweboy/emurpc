#include <catch.hpp>
#include <nlohmann/json.hpp>
#include "json_protocol.h"

using json = nlohmann::json;

TEST_CASE("JSON serialization", "[json_serializer]") {
    JSONSerializer j{};
    Response::AnyPacket p;
    std::vector<u8> data;
    std::vector<u8> serial;
    json out;

    SECTION("Memory Read") {
        data = {'H', 'E', 'L', 'L', 'O', ' ', 'W', 'O', 'R', 'L', 'D', '!'};
        out = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "result": "SEVMTE8gV09STEQh"
})a"_json;
        p = Response::MemoryRead{1, std::move(data)};
		serial = j.SerializeResponse(p);
        REQUIRE(json::parse(serial.begin(), serial.end()) == out);
    }
}
