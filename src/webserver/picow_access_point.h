#ifndef picow_access_point_h
#define picow_access_point_h
#include <stdio.h>
#include <string.h>
//
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
//
#include "dhcpserver.h"
#include "dnsserver.h"
//
#define TCP_PORT 80
#define DEBUG_printf(...) do {} while(0)
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
#define CONTROL_PANEL_BODY "<html><body><form action='/commands'> Command:<input type='hidden' name='command' value='1'><input type='submit' value='Scan'></body></html>"
#define RESULTS_PAGE "<html><body><form action='/chooseap'><select>%s</select></form></body></html>"
//#define CONTROL_PANEL_BODY "<html><body><form action='/command'> Command:<select>%s</select></body></html>"
#define COMMAND "command=%d"
#define CONTROL_PANEL "/commands"
#define LED_GPIO 0
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://%s" CONTROL_PANEL "\n\n"
#define MAX_SSID_COUNT 20

extern bool testingg;
extern char gbuff[512];

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    bool complete;
    ip_addr_t gw;
    async_context_t *context;
} TCP_SERVER_T;

typedef struct TCP_CONNECT_STATE_T_ {
    struct tcp_pcb *pcb;
    int sent_len;
    char headers[128];
    char result[512];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;

static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err);
static void tcp_server_close(TCP_SERVER_T *state);
static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len);
static int test_server_content(const char *request, const char *params, char *result, size_t max_result_len);
err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb);
static void tcp_server_err(void *arg, err_t err);
static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);
static bool tcp_server_open(void *arg, const char *ap_name);
int setup_web_server();

#endif