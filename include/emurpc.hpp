
#pragma once

namespace EmuRPC {


struct emurpc_state;
struct emurpc_state* emurpc_start(struct emurpc_config);

void emurpc_destroy(struct emurpc_state*);

void emurpc_process_events(struct emurpc_state*);

void emurpc_on_frame_end(struct emurpc_state*);

void emurpc_on_memory_access(struct emurpc_state*, struct emurpc_memory_access);

void emurpc_on_gpu_access(struct emurpc_state*, struct emurpc_gpu_access);

void emurpc_on_special_access(struct emurpc_state*,
                              struct emurpc_special_access);

}
