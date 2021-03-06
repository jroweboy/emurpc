
#include <stdio.h>
#include <string.h>
#include "emurpc.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

static bool save_state_callback(void* user_data, u16 save_slot) {
    printf("Save state called by RPC client");
    return false;
}

static bool load_state_callback(void* user_data, u16 save_slot) {
    printf("Load state called by RPC client");
    return true;
}

int main(int argv, char** argc) {
    struct emurpc_config config;
    config.enable_gpu_access_timing = true;
    config.enable_memory_access_timing = true;
    config.enable_special_access_timing = false;
    config.load_rom_callback = NULL;
    config.save_state_callback = save_state_callback;
    config.load_state_callback = load_state_callback;

    emurpc_state emurpc = emurpc_start(config);

    printf("Listening on 0.0.0.0:8080\nPress any key to quit\n");
    fflush(stdout);
    while (!getchar()) {
#ifdef _WIN32
        Sleep(0);
#else
        sleep(0);
#endif
    }
    emurpc_destroy(emurpc);
    return 0;
}
