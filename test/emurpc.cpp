
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <catch.hpp>
#include <nlohmann/json.hpp>

#include "emulator.h"
#include "emurpc.hpp"

// TODO Change the test runner to only make the emulator (and emurpc session) once for all of the
// tests so we don't have to bind to a socket every test case

TEST_CASE("Memory Read RPC", "[emurpc]") {
    using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
    namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>
    using json = nlohmann::json;

    // start the fake emulator and emurpc server
    MemoryOnlyEmu emu;

    boost::asio::io_context ioc;
    tcp::resolver resolver{ioc};
    websocket::stream<tcp::socket> ws{ioc};
    auto const results = resolver.resolve("localhost", "8080");
    boost::asio::connect(ws.next_layer(), results.begin(), results.end());

    // Perform the websocket handshake
    ws.handshake("localhost", "/");

    SECTION("Read null bytes immediate nonblocking once") {
        std::string request = R"a({
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
})a";

        json expected = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "result": "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="
})a"_json;

        // Send the request
        ws.write(boost::asio::buffer(request));
        emu.Run(1);

        boost::beast::flat_buffer buffer;
        ws.read(buffer);

        json response = json::parse(boost::beast::buffers_to_string(buffer.data()));
        REQUIRE(response == expected);
    }

    SECTION("Read random data immediate nonblocking once") {
        std::string request = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "method": "memory_read",
  "params": {
    "timing": "immediate",
    "sync": "nonblocking",
    "function": "once",
    "address": 0,
    "length": 64
  }
})a";
        json nullbytes_response = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "result": "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="
})a"_json;

        // Send the request
        ws.write(boost::asio::buffer(request));
        emu.Run(1);

        boost::beast::flat_buffer buffer;
        ws.read(buffer);

        json response = json::parse(boost::beast::buffers_to_string(buffer.data()));
        REQUIRE_FALSE(response.at("result") == nullbytes_response.at("result"));
    }

    // Close the WebSocket connection
    ws.close(websocket::close_code::normal);
}

TEST_CASE("Memory Write RPC", "[emurpc]") {
    using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
    namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>
    using json = nlohmann::json;

    // start the fake emulator and emurpc server
    MemoryOnlyEmu emu;

    boost::asio::io_context ioc;
    tcp::resolver resolver{ioc};
    websocket::stream<tcp::socket> ws{ioc};
    auto const results = resolver.resolve("localhost", "8080");
    boost::asio::connect(ws.next_layer(), results.begin(), results.end());

    // Perform the websocket handshake
    ws.handshake("localhost", "/");

    SECTION("Write null bytes immediate nonblocking once") {
        std::string request = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "method": "memory_write",
  "params": {
    "timing": "immediate",
    "sync": "nonblocking",
    "function": "once",
    "address": 0,
    "data": "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA=="
  }
})a";
        json accepted_response = R"a({
  "jsonrpc": "2.0",
  "id": 1,
  "result": ""
})a"_json;
        // Send the request
        ws.write(boost::asio::buffer(request));
        emu.Run(1);

        boost::beast::flat_buffer buffer;
        ws.read(buffer);

        json response = json::parse(boost::beast::buffers_to_string(buffer.data()));
        REQUIRE(response == accepted_response);
    }

    // Close the WebSocket connection
    ws.close(websocket::close_code::normal);
}

TEST_CASE("Memory Write Then Read RPC", "[emurpc]") {}
