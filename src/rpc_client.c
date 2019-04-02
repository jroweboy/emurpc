
#include <libwebsockets.h>

#include "rpc_client.h"

static void destroy_message(struct message* msg) {
    if (!msg) {
        return;
    }
    if (msg->payload) {
        free(msg->payload);
    }
    free(msg);
}

struct message_ll {
    struct message_ll* next;
    struct message* message;
};

static struct message_ll* message_ll_create(struct message* msg) {
    struct message_ll* msg_ll = calloc(1, sizeof(struct message));
    msg_ll->message = msg;
    msg_ll->next = NULL;
    return msg_ll;
}

static struct message* pop_ll(struct message_ll** head) {
    if (!head) {
        return NULL;
    }
    struct message_ll* h = *head;
    if (!h) {
        return NULL;
    }
    struct message_ll* old = h;
    h = h->next;
    struct message* message = h->message;
    free(h);
    return message;
}

static void push_ll(struct message_ll** head, struct message_ll** tail,
                    struct message* message) {
    if (!head || !tail) {
        // shouldn't be trying to push something on nothing
        return;
    }
    struct message_ll* msg = calloc(1, sizeof(struct message_ll));
    struct message_ll* h = *head;
    struct message_ll* t = *tail;
    if (!h && !t) {
        // Nothing has been pushed so far so set both to msg
        h = t = msg;
        return;
    }
    t->next = msg;
    t = msg;
}

struct rpcclient {
    struct lws* wsi;
    struct message_ll* write_head;
    struct message_ll* write_tail;
    struct message_ll* read_head;
    struct message_ll* read_tail;
};

struct rpcclient* rpcclient_create() {
    return calloc(1, sizeof(struct rpcclient));
}

bool rpcclient_start(struct rpcclient* client, struct lws* wsi) {
    client->wsi = wsi;
    return true;
}

struct message* rpcclient_pop_pending(struct rpcclient* client) {
    return pop_ll(&client->write_head);
}

bool rpcclient_push_message(struct rpcclient* client, struct message* msg) {
    struct message_ll* msg_ll = message_ll_create(msg);
    client->read_tail->next = msg_ll;
    client->read_tail = msg_ll;
    return true;
}

void rpcclient_destroy(struct rpcclient* client) {
    struct message* message;
    while (message = pop_ll(&client->read_head)) {
        destroy_message(message);
    }
    while (message = pop_ll(&client->write_head)) {
        destroy_message(message);
    }
    free(client);
}

uint8_t* create_read_payload(size_t len) {
    return calloc(len, sizeof(uint8_t));
}

uint8_t* create_write_payload(size_t len) {
    uint8_t* ptr = calloc(len + LWS_PRE, sizeof(uint8_t));
    // We want to write data to the payload after the LWS_PRE header data,
    // so move the pointer we return forward by LWS_PRE so the other client
    // code can simply memcpy to this pointer
    return ptr + LWS_PRE;
}
