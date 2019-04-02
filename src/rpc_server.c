
#include <libwebsockets.h>

#include "rpc_client.h"
#include "rpc_server.h"

struct per_session_data {
    struct per_session_data* pss_list;
    struct lws* wsi;
    struct rpcclient* client;
};

/* one of these is created for each vhost our protocol is used with */
struct per_vhost_data {
    struct lws_context* context;
    struct lws_vhost* vhost;
    const struct lws_protocols* protocol;

    struct per_session_data* pss_list; /* linked-list of live pss*/
};

static int callback_minimal(struct lws* wsi, enum lws_callback_reasons reason,
                            void* user, void* in, size_t len) {
    struct message* message;
    struct per_session_data* pss = (struct per_session_data*) user;
    struct per_vhost_data* vhd =
        (struct per_vhost_data*) lws_protocol_vh_priv_get(
            lws_get_vhost(wsi), lws_get_protocol(wsi));
    int m;

    switch (reason) {
    case LWS_CALLBACK_PROTOCOL_INIT:
        vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
                                          lws_get_protocol(wsi),
                                          sizeof(struct per_vhost_data));
        vhd->context = lws_get_context(wsi);
        vhd->protocol = lws_get_protocol(wsi);
        vhd->vhost = lws_get_vhost(wsi);
        break;

    case LWS_CALLBACK_ESTABLISHED:
        /* add ourselves to the list of live pss held in the vhd */
        lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
        pss->wsi = wsi;
        break;

    case LWS_CALLBACK_CLOSED:
        /* remove our closing pss from the list of live pss */
        lws_ll_fwd_remove(struct per_session_data, pss_list, pss,
                          vhd->pss_list);
        break;

    case LWS_CALLBACK_SERVER_WRITEABLE:
        // TODO: Get the message from rpcclient
        while (message = rpcclient_pop_pending(pss->client)) {

            m = lws_write(wsi, message->payload, message->len,
                          message->is_binary ? LWS_WRITE_BINARY
                                             : LWS_WRITE_TEXT);
            if (m < (int) message->len) {
                free(message);
                lwsl_err("ERROR %d writing to ws\n", m);
                return -1;
            }
            free(message);
        }
        break;

    case LWS_CALLBACK_RECEIVE:
        message = calloc(1, sizeof(struct message));
        message->payload = create_read_payload(len);
        if (!message->payload) {
            lwsl_user("OOM: dropping\n");
            break;
        }

        memcpy((uint8_t*) message->payload, in, len);
        message->len = len;
        message->is_binary = lws_frame_is_binary(wsi);
        rpcclient_push_message(pss->client, message);
        ///*
        // * let everybody know we want to write something on them
        // * as soon as they are ready
        // */
        // lws_start_foreach_llp(struct per_session_data**, ppss, vhd->pss_list)
        // {
        //    lws_callback_on_writable((*ppss)->wsi);
        //}
        // lws_end_foreach_llp(ppss, pss_list);
        break;

    default:
        break;
    }

    return 0;
}

static struct lws_protocols protocols[] = {
    {"http", lws_callback_http_dummy, 0, 0},
    {
        "emurpc",
        callback_minimal,
        sizeof(struct per_session_data),
        0,
    },
    {NULL, NULL, 0, 0} /* terminator */
};

static const struct lws_http_mount mount = {
    /* .mount_next */ NULL,         /* linked-list "next" */
    /* .mountpoint */ "/",          /* mountpoint URL */
    /* .origin */ "./mount-origin", /* serve from dir */
    /* .def */ "index.html",        /* default filename */
    /* .protocol */ NULL,
    /* .cgienv */ NULL,
    /* .extra_mimetypes */ NULL,
    /* .interpret */ NULL,
    /* .cgi_timeout */ 0,
    /* .cache_max_age */ 0,
    /* .auth_mask */ 0,
    /* .cache_reusable */ 0,
    /* .cache_revalidate */ 0,
    /* .cache_intermediaries */ 0,
    /* .origin_protocol */ LWSMPRO_FILE, /* files in a dir */
    /* .mountpoint_len */ 1,             /* char count */
    /* .basic_auth_login_file */ NULL,
};

struct rpcserver {
    struct rpcclient* client;
    struct lws_context* context;
};

struct rpcserver* rpcserver_create() {
    struct rpcserver* server = calloc(1, sizeof(struct rpcserver));
    if (!server) {
        return NULL;
    }
    server->client = rpcclient_create();
    if (!server->client) {
        free(server);
        return NULL;
    }
    return server;
}

bool rpcserver_start(struct rpcserver* server, struct rpcserver_config config) {
    struct lws_context_creation_info info;
    struct lws_context* context;
    int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
    lws_set_log_level(logs, NULL);

    memset(&info, 0, sizeof(info)); /* otherwise uninitialized garbage */
    info.port = config.port;
    info.mounts = &mount;
    info.protocols = protocols;
    info.options = LWS_SERVER_OPTION_UV_NO_SIGSEGV_SIGFPE_SPIN |
                   LWS_SERVER_OPTION_PEER_CERT_NOT_REQUIRED;
#ifdef EMURPC_HAVE_LIBUV
    info.options |= LWS_SERVER_OPTION_LIBUV;
#endif
    context = lws_create_context(&info);
    if (!context) {
        return false;
    }
    server->context = context;
    return true;
}

void rpcserver_destroy(struct rpcserver* server) {
    if (!server) {
        return;
    }
    lws_context_destroy(server->context);
    rpcclient_destroy(server->client);
    free(server);
}

void rpcserver_process_events(struct rpcserver* server) {
    lws_service(server->context, 0);
}
