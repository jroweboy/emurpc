
#ifndef EMURPC_RPC_SERVER
#define EMURPC_RPC_SERVER

#include "emurpc/types.h"

struct rpcserver_config {
	const char* server_name;
	uint16_t port;
};

struct rpcserver_impl;
struct rpcserver {
	struct rpcserver_impl* impl;
};

bool rpcserver_init(struct rpcserver*, struct rpcserver_config);

void rpcserver_destroy(struct rpcserver*);

void rpcserver_process_events(struct rpcserver*);

#endif