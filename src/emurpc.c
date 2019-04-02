
#include <libwebsockets.h>

#include "emurpc.h"
#include "rpc_server.h"

struct emurpc_state_impl {
#ifdef HAVE_THREADS
	
#endif
	void* user_data;
	struct rpcserver* rpc_server;
};

bool emurpc_init(struct emurpc_state* state, struct emurpc_config config) {
	struct emurpc_state_impl* impl = malloc(sizeof(struct emurpc_state_impl));
	if (!impl) {
		return false;
	}

	struct rpcserver_config rpc_config;
	rpc_config.server_name = "ur mom";
	rpc_config.port = 8080;
	struct rpcserver* server = malloc(sizeof(struct rpcserver));
	if (!rpcserver_init(server, rpc_config)) {
		return false;
	}
	impl->rpc_server = server;
	impl->user_data = config.user_data;
	state->impl = impl;
	return true;
}

void emurpc_shutdown(struct emurpc_state* state) {
	if (!state) {
		return;
	}
	if (state->impl) {
		rpcserver_destroy(state->impl->rpc_server);
		free(state->impl);
	}
	free(state);
}

bool emurpc_process_events(struct emurpc_state* state) {
	return true;
}

void emurpc_on_frame_end(struct emurpc_state* state) {

}

void emurpc_on_memory_access(struct emurpc_state* state, enum emurpc_access_type type) {

}

void emurpc_on_gpu_access(struct emurpc_state* state, enum emurpc_access_type type) {

}

void emurpc_on_register_access(struct emurpc_state* state, enum emurpc_access_type type) {

}