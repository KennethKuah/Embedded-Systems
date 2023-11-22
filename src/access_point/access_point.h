#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H
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
#include "dhcpserver/dhcpserver.h"
#include "dnsserver/dnsserver.h"
//
#define TCP_PORT 80
#define DEBUG_printf printf
#define POLL_TIME_S 5
#define HTTP_GET "GET"
#define HTTP_RESPONSE_HEADERS "HTTP/1.1 %d OK\nContent-Length: %d\nContent-Type: text/html; charset=utf-8\nConnection: close\n\n"
#define LED_TEST_BODY "<html><body><h1>Hello from Pico W.</h1><p>Led is %s</p><p><a href=\"?led=%d\">Turn led %s</a></body></html>"
#define LED_PARAM "led=%d"
#define LED_TEST "/ledtest"
#define LED_GPIO 0
#define HTTP_RESPONSE_REDIRECT "HTTP/1.1 302 Redirect\nLocation: http://%s" LED_TEST "\n\n"
#define MAX_SSID_COUNT 5
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

// The reason why i made this a global variable is because i want to change it.
// cyw43_ev_scan_result_t array_of_ssid[MAX_SSID_COUNT];
// volatile int ARRAY_CTR = 0;
// volatile bool timer_fired = false;
// volatile uint64_t start_time = 0;
// struct repeating_timer timer;
// volatile bool timeout = false;
// volatile int finished = 0;
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
    char result[256];
    int header_len;
    int result_len;
    ip_addr_t *gw;
} TCP_CONNECT_STATE_T;


int setup_ap(cyw43_ev_scan_result_t* test_ssid);

#endif