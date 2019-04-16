
#pragma once

#include "protocol.h"

class JSONSerializer : public ProtocolSerializer {
public:
    std::vector<u8> SerializeResponse(const Response::AnyPacket&) override;
    Request::AnyPacket DeserializeRequest(const std::vector<u8>&) override;
};

class MsgPackSerializer : public ProtocolSerializer {
public:
    std::vector<u8> SerializeResponse(const Response::AnyPacket&) override;
    Request::AnyPacket DeserializeRequest(const std::vector<u8>&) override;
};
