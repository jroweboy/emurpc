
#pragma once

#include <memory>
#include <string>
#include <thread>
#include <boost/variant.hpp>
#include "common.h"
#include "emurpc.hpp"

namespace Request {
class Packet;
}

namespace Response {
class Packet;
}

namespace boost {
namespace asio {
class io_context;
}
} // namespace boost

class RPCServer {
public:
    explicit RPCServer(const std::string& hostname = "0.0.0.0", u16 port = 0);

    ~RPCServer();

    void ProcessClientEvents();

    void HandleEmuCallbacks(const Response::Packet*);

private:
    std::unique_ptr<boost::asio::io_context> ioc;
    std::thread io_thread;
};
