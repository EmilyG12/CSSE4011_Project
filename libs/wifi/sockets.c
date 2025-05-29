#include <zephyr/logging/log.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socketutils.h>
#include <zephyr/net/dns_resolve.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/net/http/client.h>
#include "sockets.h"

LOG_MODULE_REGISTER(artemis_http_post);

#if defined(CONFIG_ARTEMIS_MANUAL_SERVER)
#define ARTEMIS_SERVER CONFIG_ARTEMIS_API_SERVER_IP
#else
#define ARTEMIS_SERVER "172.30.75.181" 
#endif

#define ARTEMIS_URL "/api/post-data"

#define HTTP_PORT "8080" 

static const char *artemis_http_headers[] = {
	"Host: 172.30.75.181:8080\r\n",
	"Content-Type: application/json\r\n",
	NULL
};

int artemis_connect(struct artemis_context *ctx)
{
	struct addrinfo *addr;
	struct addrinfo hints;
	char hr_addr[INET6_ADDRSTRLEN];
	char *port;
	int dns_attempts = 3;
	int ret = -1;

	memset(&hints, 0, sizeof(hints));

	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (IS_ENABLED(CONFIG_NET_IPV6)) {
		hints.ai_family = AF_INET6;
	} else if (IS_ENABLED(CONFIG_NET_IPV4)) {
		hints.ai_family = AF_INET;
	}
	port = HTTP_PORT;

	while (dns_attempts--) {
		ret = getaddrinfo(ARTEMIS_SERVER, port, &hints, &addr);
		if (ret == 0) {
			break;
		}
		k_sleep(K_SECONDS(1));
	}

	if (ret < 0) {
		LOG_ERR("Could not resolve dns, error: %d", ret);
		return ret;
	}

	LOG_INF("%s address: %s",
		(addr->ai_family == AF_INET ? "IPv4" : "IPv6"),
		net_addr_ntop(addr->ai_family,
			      &net_sin(addr->ai_addr)->sin_addr,
			      hr_addr, sizeof(hr_addr)));

	ctx->sock = socket(hints.ai_family,
			   hints.ai_socktype,
			   hints.ai_protocol);
	if (ctx->sock < 0) {
		LOG_ERR("Failed to create %s HTTP socket (%d)",
			(addr->ai_family == AF_INET ? "IPv4" : "IPv6"),
			-errno);

		freeaddrinfo(addr);
		return -errno;
	}

	if (connect(ctx->sock, addr->ai_addr, addr->ai_addrlen) < 0) {
		LOG_ERR("Cannot connect to %s remote (%d)",
			(addr->ai_family == AF_INET ? "IPv4" : "IPv6"),
			-errno);

		freeaddrinfo(addr);
		return -errno;
	}
    LOG_INF("Connected");
	freeaddrinfo(addr);

	return 0;
}

int artemis_http_push(struct artemis_context *ctx,
		     http_response_cb_t resp_cb)
{
	LOG_INF("Push to artemis server\n");
	struct http_request req;
	int ret;

	memset(&req, 0, sizeof(req));

	req.method		= HTTP_POST;
	req.host		= "172.30.75.181:8080";
	req.port		= HTTP_PORT;
	req.url			= ARTEMIS_URL;
	req.header_fields	= artemis_http_headers;
	req.protocol		= "HTTP/1.1";
	req.response		= resp_cb;
	req.payload		= ctx->payload;
	req.payload_len		= strlen(ctx->payload);
	req.recv_buf		= ctx->resp;
	req.recv_buf_len	= sizeof(ctx->resp);

    LOG_INF("JSON: %s", ctx->payload);
    
	char request[512]; 
	
	snprintf(request, sizeof(request),
         "POST /api/post-data HTTP/1.1\r\n"
         "Host: 172.30.75.181:8080\r\n"
         "Content-Type: application/json\r\n"
         "Content-Length: %d\r\n"
         "\r\n"
         "%s",
         strlen(ctx->payload), ctx->payload);

    send(ctx->sock, request, strlen(request), 0);
    
	ret = http_client_req(ctx->sock, &req,
			      CONFIG_ARTEMIS_HTTP_CONN_TIMEOUT * MSEC_PER_SEC,
			      ctx);

	LOG_INF("Pushed to artemis server %d\n", ret);
	close(ctx->sock);
	ctx->sock = -1;

	return ret;
}
