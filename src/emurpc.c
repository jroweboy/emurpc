
#include <libwebsockets.h>
#include <time.h>
#include <tinycthread.h>

#include "emurpc.h"
#include "rpc_server.h"

struct emurpc_state {
#ifdef HAVE_THREADS
    thrd_t thread;
#endif
    void* user_data;
    struct rpcserver* rpcserver;
    bool initialized;
};

#ifdef HAVE_THREADS
#include <stdio.h>
#include <windows.h>
double RealElapsedTime(void) { // granularity about 50 microsecs on my machine
    static LARGE_INTEGER freq, start;
    LARGE_INTEGER count;
    if (!QueryPerformanceCounter(&count))
        ;                 // FatalError("QueryPerformanceCounter");
    if (!freq.QuadPart) { // one time initialization
        if (!QueryPerformanceFrequency(&freq))
            ; // FatalError("QueryPerformanceFrequency");
        start = count;
    }
    return (double) (count.QuadPart - start.QuadPart) / freq.QuadPart;
}

struct rpcserver_setup {
    struct rpcserver* server;
    struct rpcserver_config config;
};
static int run_rpcserver(struct rpcserver_setup* setup) {
    struct rpcserver* server = setup->server;
    if (!rpcserver_start(setup->server, setup->config)) {
        free(setup);
        return 0;
    }
    free(setup);
    struct timespec one_ns = {0, 1};
    RealElapsedTime();
    while (1) {
        rpcserver_process_events(server);
        printf("start time:\t%f\n", RealElapsedTime());
		thrd_yield();
        //thrd_sleep(&one_ns, NULL);
        printf("end time:\t%f\n\n", RealElapsedTime());
    }
    return 1;
}
#endif

struct emurpc_state* emurpc_create() {
    struct emurpc_state* state = calloc(1, sizeof(struct emurpc_state));
    if (!state) {
        return NULL;
    }
    state->rpcserver = rpcserver_create();
    if (!state->rpcserver) {
        free(state);
        return NULL;
    }
    return state;
}

struct emurpc_state* emurpc_start(struct emurpc_config config) {
    struct emurpc_state* state = emurpc_create();
    if (!state) {
        return NULL;
    }
    state->user_data = config.user_data;

    struct rpcserver_config rpc_config;
    rpc_config.server_name = "localhost";
    rpc_config.port = 8080;

#ifdef HAVE_THREADS
    struct rpcserver_setup* setup = malloc(sizeof(struct rpcserver_setup));
    setup->config = rpc_config;
    setup->server = state->rpcserver;
    thrd_create(&state->thread, run_rpcserver, setup);
#else
    rpcserver_start(state->rpcserver, rpc_config);
#endif

    return state;
}

void emurpc_destroy(struct emurpc_state* state) {
    if (!state) {
        return;
    }
    rpcserver_destroy(state->rpcserver);
    free(state);
}

void emurpc_process_events(struct emurpc_state* state) {
    if (!state) {
        return;
    }
    rpcserver_process_events(state->rpcserver);
}

void emurpc_on_frame_end(struct emurpc_state* state) {
    if (!state) {
        return;
    }
#ifdef HAVE_THREADS

#else

#endif
}

void emurpc_on_memory_access(struct emurpc_state* state,
                             struct emurpc_memory_access config) {}

void emurpc_on_gpu_access(struct emurpc_state* state,
                          struct emurpc_gpu_access config) {}

void emurpc_on_special_access(struct emurpc_state* state,
                              struct emurpc_special_access config) {}
