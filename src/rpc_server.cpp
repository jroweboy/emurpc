
#include <boost/beast/websocket.hpp>

#include "rpc_client.h"
#include "rpc_server.h"

RPCServer::RPCServer(std::string hostname, u16 port) {}

void RPCServer::HandleClientEvent(ClientToEmuMessage) {}

void RPCServer::HandleEmuCallbacks(EmuToClientMessage) {}
