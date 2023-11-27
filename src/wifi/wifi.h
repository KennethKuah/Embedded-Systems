#ifndef WIFI_H
#define WIFI_H
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "dhcpserver.h"
#include "dnsserver.h"

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/init.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"

#include "common/b64.h"

typedef unsigned char   BYTE;

typedef struct ServerConnection{
    int server_sock;
    char *dst_ip;
    int port;
    struct ServerConnection *next;
    struct ServerConnection *prev;
} server_conn_t;


typedef struct I2CData{
    char *dst_ip;
    int port;
    char *proto;
    BYTE *data;
    int data_len;
} i2c_data_t;

typedef struct DNS_T {
    bool dns_request_sent;
    char *domain;
    ip_addr_t res;
    int completed;
} dns_t;

#define MAX_MESSAGE_SIZE 1000
#define AP_SSID "mypicow"
#define AP_PW "password"

extern server_conn_t *conn_list;

int setup_wifi();
int setup_ap();
void new_tcp_tunnel(char *, int);
void send_tunnel(int, BYTE *, int);
int search_conn(char *, int);
void insert_new_conn(int, char *, int);
void remove_conn(char *, int);
void print_conns();
void test_conns();
void test_dns();
char *i2c_serialize(char *, int, char *, BYTE *, int);
i2c_data_t *i2c_deserialize(char *);

#endif
