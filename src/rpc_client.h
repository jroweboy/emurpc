#ifndef EMURPC_RPCCLIENT_H
#define EMURPC_RPCCLIENT_H

#include "emurpc/types.h"

// Defined by Libwebsockets
struct lws;

struct message {
    uint8_t* payload;
    size_t len;
    bool is_binary;
};

uint8_t* create_read_payload(size_t);
uint8_t* create_write_payload(size_t);

struct rpcclient;
struct rpcclient* rpcclient_create();

bool rpcclient_start(struct rpcclient*, struct lws* wsi);

/**
 * Pushes a message to the read queue for the client to process.
 * If the client should need to write anything to the response, it should call
 * lws_callback_on_writable to notify lws that there is data to write.
 */
bool rpcclient_push_message(struct rpcclient*, struct message*);

/**
 * Removes messages from the write queue and returns a pointer to the data.
 * Its the responsibility of the caller to free the memory returned after its
 * done. Returns null if there are no more messages to process
 */
struct message* rpcclient_pop_pending(struct rpcclient*);

void rpcclient_destroy(struct rpcclient*);

#endif
