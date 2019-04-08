
#pragma once

#include <string>
#include <boost/variant.hpp>
#include "common.h"
#include "emurpc.hpp"

struct FrameEnd {};
struct MemoryAccess {
    EmuRPC::AccessType type;
    u64 address;
};
struct GPUAccess {
    EmuRPC::AccessType type;
    std::string field;
};
struct SpecialAccess {
    EmuRPC::AccessType type;
    std::string field;
};

using EmuToClientMessage = boost::variant<FrameEnd, MemoryAccess, GPUAccess, SpecialAccess>;

struct Finished {};

using ClientToEmuMessage = boost::variant<Finished>;

class RPCServer {
public:
    explicit RPCServer(std::string hostname = "0.0.0.0", u16 port = 0);

    void HandleClientEvent(ClientToEmuMessage);

    void HandleEmuCallbacks(EmuToClientMessage);

private:
    std::string hostname;
    u16 port;
};
