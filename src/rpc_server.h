
#ifndef EMURPC_RPC_SERVER
#define EMURPC_RPC_SERVER

#include "emurpc/types.h"

struct rpcserver_config {
	const char* server_name;
	uint16_t port;
};

struct rpcserver;
struct rpcserver* rpcserver_create();

bool rpcserver_start(struct rpcserver*, struct rpcserver_config);

void rpcserver_destroy(struct rpcserver*);

void rpcserver_process_events(struct rpcserver*);

void rpcserver_handle_timing(struct rpcserver*, enum emurpc_request_timing);

#endif