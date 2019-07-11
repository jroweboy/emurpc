
#pragma once

#include <stdexcept>

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

class version_mismatch_error : public std::runtime_error {
public:
    explicit version_mismatch_error(const char* c) : std::runtime_error(c){};
};

class invalid_method_error : public std::runtime_error {
public:
    explicit invalid_method_error(const char* c) : std::runtime_error(c){};
};

std::vector<u8> DecodeBase64(const std::string& val);

std::string EncodeBase64(const std::vector<u8>& val);