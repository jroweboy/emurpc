
#pragma once

#include <memory>
#include <string>
#include <boost/variant.hpp>
#include "common.h"
#include "emurpc.hpp"

namespace Request {
class Packet;
}

namespace Response {
class Packet;
}

class RPCServer {
public:
    explicit RPCServer(std::string hostname = "0.0.0.0", u16 port = 0);

    void HandleClientEvent(const Request::Packet*);

    void HandleEmuCallbacks(const Response::Packet*);

private:
    std::string hostname;
    u16 port;
};
