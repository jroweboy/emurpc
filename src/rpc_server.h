
#pragma once

#include <memory>
#include <string>
#include <thread>
#include "protocol.h"

namespace boost {
namespace asio {
class io_context;
}
} // namespace boost

class Listener;
class Session;

class RPCServer {
public:
    explicit RPCServer(const std::string& hostname = "0.0.0.0", u16 port = 8080);

    ~RPCServer();

    void ProcessClientEvents();

    void HandleEmuCallbacks(const Response::Packet*);

private:
    std::unique_ptr<boost::asio::io_context> ioc;
    std::shared_ptr<Listener> listener;
    std::shared_ptr<Session> Session;
    std::thread io_thread;
};
