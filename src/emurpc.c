
#include "emurpc.h"

struct emurpc_state_impl {
	bool initialized;
};

bool emurpc_init(struct emurpc_state* state, struct emurpc_config config) {
	return true;
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

void emurpc_shutdown(struct emurpc_state* state) {

}