
#define ARTEMIS_SOCKET_MAX_BUF_LEN 1280

struct artemis_context {
	int sock;
	uint8_t payload[ARTEMIS_SOCKET_MAX_BUF_LEN];
	uint8_t resp[ARTEMIS_SOCKET_MAX_BUF_LEN];
};

int artemis_connect(struct artemis_context *ctx);
int artemis_http_push(struct artemis_context *ctx,
		     http_response_cb_t resp_cb);
