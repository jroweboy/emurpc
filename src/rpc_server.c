
#include <libwebsockets.h>

#include "rpc_server.h"

struct message {
        void *payload; /* is malloc'd */
        size_t len;
};

struct per_session_data__minimal {
        struct per_session_data__minimal *pss_list;
        struct lws *wsi;
        uint32_t tail;

        unsigned int culled:1;
};


/* one of these is created for each vhost our protocol is used with */
struct per_vhost_data__minimal {
        struct lws_context *context;
        struct lws_vhost *vhost;
        const struct lws_protocols *protocol;

        struct per_session_data__minimal *pss_list; /* linked-list of live pss*/

        struct lws_ring *ring; /* ringbuffer holding unsent messages */
};

static void __minimal_destroy_message(void *_msg) {
        struct message *msg = _msg;

        free(msg->payload);
        msg->payload = NULL;
        msg->len = 0;
}

static int callback_minimal(struct lws *wsi, enum lws_callback_reasons reason,
                        void *user, void *in, size_t len) {
        struct per_session_data__minimal *pss =
                        (struct per_session_data__minimal *)user;
        struct per_vhost_data__minimal *vhd =
                        (struct per_vhost_data__minimal *)
                        lws_protocol_vh_priv_get(lws_get_vhost(wsi),
                                        lws_get_protocol(wsi));
        const struct message* pmsg;
        struct message amsg;
        int n, m;

        switch (reason) {
        case LWS_CALLBACK_PROTOCOL_INIT:
                vhd = lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
                                lws_get_protocol(wsi),
                                sizeof(struct per_vhost_data__minimal));
                vhd->context = lws_get_context(wsi);
                vhd->protocol = lws_get_protocol(wsi);
                vhd->vhost = lws_get_vhost(wsi);

                vhd->ring = lws_ring_create(sizeof(struct message), 8,
                                            __minimal_destroy_message);
                if (!vhd->ring)
                        return 1;
                break;

        case LWS_CALLBACK_PROTOCOL_DESTROY:
                lws_ring_destroy(vhd->ring);
                break;

        case LWS_CALLBACK_ESTABLISHED:
                /* add ourselves to the list of live pss held in the vhd */
                lwsl_user("LWS_CALLBACK_ESTABLISHED: wsi %p\n", wsi);
                lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
                pss->tail = lws_ring_get_oldest_tail(vhd->ring);
                pss->wsi = wsi;
                break;

        case LWS_CALLBACK_CLOSED:
                lwsl_user("LWS_CALLBACK_CLOSED: wsi %p\n", wsi);
                /* remove our closing pss from the list of live pss */
                lws_ll_fwd_remove(struct per_session_data__minimal, pss_list,
                                  pss, vhd->pss_list);
                break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
                if (pss->culled)
                        break;
                pmsg = lws_ring_get_element(vhd->ring, &pss->tail);
                if (!pmsg)
                        break;

                /* notice we allowed for LWS_PRE in the payload already */
                m = lws_write(wsi, ((unsigned char *)pmsg->payload) +
                              LWS_PRE, pmsg->len, LWS_WRITE_TEXT);
                if (m < (int)pmsg->len) {
                        lwsl_err("ERROR %d writing to ws socket\n", m);
                        return -1;
                }

                lws_ring_consume_and_update_oldest_tail(
                        vhd->ring,	/* lws_ring object */
                        struct per_session_data__minimal, /* type of objects with tails */
                        &pss->tail,	/* tail of guy doing the consuming */
                        1,		/* number of payload objects being consumed */
                        vhd->pss_list,	/* head of list of objects with tails */
                        tail,		/* member name of tail in objects with tails */
                        pss_list	/* member name of next object in objects with tails */
                );

                /* more to do for us? */
                if (lws_ring_get_element(vhd->ring, &pss->tail))
                        /* come back as soon as we can write more */
                        lws_callback_on_writable(pss->wsi);
                break;

        case LWS_CALLBACK_RECEIVE:
                n = (int)lws_ring_get_count_free_elements(vhd->ring);
                if (!n) {
                        /* forcibly make space */
                        //cull_lagging_clients(vhd);
                        n = (int)lws_ring_get_count_free_elements(vhd->ring);
                }
                if (!n)
                        break;

                lwsl_user("LWS_CALLBACK_RECEIVE: free space %d\n", n);

                amsg.len = len;
                /* notice we over-allocate by LWS_PRE... */
                amsg.payload = malloc(LWS_PRE + len);
                if (!amsg.payload) {
                        lwsl_user("OOM: dropping\n");
                        break;
                }

                /* ...and we copy the payload in at +LWS_PRE */
                memcpy((char *)amsg.payload + LWS_PRE, in, len);
                if (!lws_ring_insert(vhd->ring, &amsg, 1)) {
                        __minimal_destroy_message(&amsg);
                        lwsl_user("dropping!\n");
                        break;
                }

                /*
                 * let everybody know we want to write something on them
                 * as soon as they are ready
                 */
                lws_start_foreach_llp(struct per_session_data__minimal **,
                                      ppss, vhd->pss_list) {
                        lws_callback_on_writable((*ppss)->wsi);
                } lws_end_foreach_llp(ppss, pss_list);
                break;

        default:
                break;
        }

        return 0;
}

static struct lws_protocols protocols[] = {
        { "http", lws_callback_http_dummy, 0, 0 },
        {
            "lws-minimal",
            callback_minimal,
            sizeof(struct per_session_data__minimal),
            0,
        },
        { NULL, NULL, 0, 0 } /* terminator */
};

static const struct lws_http_mount mount = {
        /* .mount_next */		NULL,		/* linked-list "next" */
        /* .mountpoint */		"/",		/* mountpoint URL */
        /* .origin */			"./mount-origin", /* serve from dir */
        /* .def */			"index.html",	/* default filename */
        /* .protocol */			NULL,
        /* .cgienv */			NULL,
        /* .extra_mimetypes */		NULL,
        /* .interpret */		NULL,
        /* .cgi_timeout */		0,
        /* .cache_max_age */		0,
        /* .auth_mask */		0,
        /* .cache_reusable */		0,
        /* .cache_revalidate */		0,
        /* .cache_intermediaries */	0,
        /* .origin_protocol */		LWSMPRO_FILE,	/* files in a dir */
        /* .mountpoint_len */		1,		/* char count */
        /* .basic_auth_login_file */	NULL,
};

struct rpcserver_impl {
	struct rpcclient* client;
    struct lws_context *context;
};

bool rpcserver_init(struct rpcserver* server, struct rpcserver_config config) {
    struct lws_context_creation_info info;
	struct lws_context* context;
    int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE;
    lws_set_log_level(logs, NULL);

	memset(&info, 0, sizeof(info)); /* otherwise uninitialized garbage */
    info.port = config.port;
    info.mounts = &mount;
    info.protocols = protocols;
    info.options = LWS_SERVER_OPTION_UV_NO_SIGSEGV_SIGFPE_SPIN | LWS_SERVER_OPTION_PEER_CERT_NOT_REQUIRED;
#ifdef EMURPC_HAVE_LIBUV
	info.options |= LWS_SERVER_OPTION_LIBUV;
#endif
	context = lws_create_context(&info);


	struct rpcserver_impl* impl = malloc(sizeof(struct rpcserver_impl));
	impl->context = context;
	impl->client = NULL;
	server->impl = impl;
	return true;
}

void rpcserver_destroy(struct rpcserver* server) {
	if (!server) {
		return;
	}
	if (!server->impl) {
		if (server) {
			free(server);
			server = NULL;
		}
		return;
	}
	if (server->impl->context) {
		lws_context_destroy(server->impl->context);
		server->impl->context = NULL;
    }
    if (server->impl->client) {
		//	rpcclient_destroy(server->impl->client);
		server->impl->client = NULL;
    }
	free(server->impl);
	free(server);
	server = NULL;

}

void rpcserver_process_events(struct rpcserver* server) {
	lws_service(server->impl->context, 0);
}

