#ifndef _TCPSERVER_H_
#define _TCPSERVER_H_
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define TCP_PORT 80
#define DEBUG_printf(...)
#define BUF_SIZE 2048
#define TEST_ITERATIONS 10
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_POST "POST"

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
    async_context_t *context;
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[512];
    char result[1024];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;

TCP_SERVER_T* tcp_server_init(void);
bool tcp_server_open(void *arg, const char *ap_name);
void tcp_server_close(TCP_SERVER_T *state);

#endif
