
#pragma once

#include "protocol.h"

class JSONSerializer : public ProtocolSerializer {
public:
    std::vector<u8> SerializeResponse(Response::AnyPacket&&) override;
    Request::AnyPacket DeserializeRequest(std::vector<u8>&&) override;
};

class MsgPackSerializer : public ProtocolSerializer {
public:
    std::vector<u8> SerializeResponse(Response::AnyPacket&&) override;
    Request::AnyPacket DeserializeRequest(std::vector<u8>&&) override;
};
