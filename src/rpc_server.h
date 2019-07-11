
#pragma once

#include <memory>
#include <string>
#include <thread>
#include <boost/lockfree/spsc_queue.hpp>
#include "protocol.h"

namespace boost {
namespace asio {
class io_context;
}
} // namespace boost

// u32 is the connected client id
using ClientRequest = std::tuple<u32, Request::AnyPacket>;
using ClientResponse = std::tuple<u32, Response::AnyPacket>;

using ClientEmuQueue = boost::lockfree::spsc_queue<ClientRequest, boost::lockfree::capacity<1024>>;
using EmuClientQueue = boost::lockfree::spsc_queue<ClientResponse, boost::lockfree::capacity<1024>>;

class Listener;
class Session;

class RPCServer {
public:
    explicit RPCServer(ClientEmuQueue& client_emu, EmuClientQueue& emu_client,
                       const std::string& hostname = "0.0.0.0", u16 port = 8080);

    ~RPCServer();

    void SendResponse(u32 client_id, const Response::AnyPacket&);

private:
    // std::atomic<bool> stopped;
    EmuClientQueue& emu_client;
    std::unique_ptr<boost::asio::io_context> ioc;
    std::shared_ptr<Listener> listener;
    std::thread io_thread;
};
