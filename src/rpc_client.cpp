
#include "rpc_client.h"

RPCClient::RPCClient() {}

RPCClient::~RPCClient() = default;

void RPCClient::PushMessage(Message&& m) {
    //
}

boost::optional<Message> RPCClient::PopPending() {
    return {};
}
