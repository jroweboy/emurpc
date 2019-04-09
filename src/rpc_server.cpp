
#include <boost/beast/websocket.hpp>

#include "protocol.h"
#include "rpc_client.h"
#include "rpc_server.h"

RPCServer::RPCServer(std::string hostname, u16 port) {}

void RPCServer::HandleClientEvent(const Request::Packet* p) {}

void RPCServer::HandleEmuCallbacks(const Response::Packet* p) {}
