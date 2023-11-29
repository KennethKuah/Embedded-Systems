#ifndef NET_MANAGER_H
#define NET_MANAGER_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "i2c_bridge/i2c_bridge.h"
#include "lwip/apps/lwiperf.h"
#include "lwip/autoip.h"
#include "lwip/def.h"
#include "lwip/icmp.h"
#include "lwip/igmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/init.h"
#include "lwip/ip.h"
#include "lwip/ip4_addr.h"
#include "lwip/ip4_frag.h"
#include "lwip/mem.h"
#include "lwip/netif.h"
#include "lwip/opt.h"
#include "lwip/priv/raw_priv.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/prot/ethernet.h"
#include "lwip/prot/iana.h"
#include "lwip/prot/ip4.h"
#include "lwip/prot/tcp.h"
#include "lwip/prot/udp.h"
#include "lwip/sockets.h"
#include "lwip/stats.h"
#include "lwip/udp.h"

#include "i2c_bridge.h"

#define MAX_CLIENTS 5

typedef unsigned char BYTE;
typedef struct eth_hdr eth_hdr_t;
typedef struct ip_hdr ip_hdr_t;
typedef struct eth_addr eth_addr_t;
typedef struct tcp_hdr tcp_hdr_t;
typedef struct udp_hdr udp_hdr_t;

typedef struct ServerConnection {
    // this will be local port on client
    int src_port;
    ip4_addr_t dst_addr;
    int dst_port;
    struct ServerConnection *next;
    struct ServerConnection *prev;
} conn_t;

// Clients are identified by IP address
// Connections are used to store active port mappings
typedef struct Client {
    eth_addr_t client_mac;
    ip4_addr_t client_addr;
    conn_t *conn_list;
} client_t;

extern eth_addr_t my_mac, gateway_mac, pico1_mac;
extern ip4_addr_t my_ip;

int setup_wifi();
client_t *new_client(eth_addr_t, ip4_addr_t);
client_t *search_client(ip4_addr_t);
conn_t *search_conn(client_t *, ip4_addr_t, int);
conn_t *insert_new_conn(client_t *, int, ip4_addr_t, int);
void remove_conn(client_t *, ip4_addr_t, int);
void print_conns(client_t *);
void test_conns();
void set_payload(struct pbuf *, const char *, u16_t);
void test_send_ethernet();
void net_handler_rx();
void extract_eth_ip(BYTE *, eth_hdr_t *, ip_hdr_t *);
void accept_callback();

#endif
