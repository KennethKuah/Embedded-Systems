#ifndef NET_MANAGER_H
#define NET_MANAGER_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/init.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"

#include "i2c_bridge.h"

#define DNS_IP "8.8.8.8"
#define DNS_PORT 53

typedef unsigned char   BYTE;

typedef struct ServerConnection{
    int server_sock;
    char *dst_ip;
    int port;
    struct ServerConnection *next;
    struct ServerConnection *prev;
} server_conn_t;

extern server_conn_t *conn_list;

int setup_wifi();
void new_tcp_tunnel(char *, int);
void send_tunnel(int, BYTE *, int);
int search_conn(char *, int);
void insert_new_conn(int, char *, int);
void remove_conn(char *, int);
void print_conns();
void test_conns();
int send_dns_req(BYTE *, int, BYTE *);
void test_dns();

#endif
