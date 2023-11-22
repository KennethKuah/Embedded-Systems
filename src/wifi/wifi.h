#ifndef WIFI_H
#define WIFI_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "picowi_defs.h"
#include "picowi_pico.h"
#include "picowi_wifi.h"
#include "picowi_init.h"
#include "picowi_ioctl.h"
#include "picowi_event.h"
#include "picowi_join.h"
#include "picowi_ip.h"
#include "picowi_net.h"
#include "picowi_udp.h"
#include "picowi_tcp.h"
#include "picowi_dhcp.h"
#include "picowi_dns.h"
#include "common/b64.h"

typedef struct {
    char *dst_ip;
    int port;
    char *proto;
    BYTE *data;
    int data_len;
} I2CData;

#endif

#define SSID "the-aquarium"
#define PASSWD "withpassionandethics"
#define EVENT_POLL_USEC 10000
#define LOCAL_PORT 1234
#define MAX_MESSAGE_SIZE 30000
#define DELIMITER ":"

extern IPADDR my_ip, router_ip, dns_ip, zero_ip;
extern int dhcp_complete;
extern uint32_t dns_ticks, led_ticks, poll_ticks;

void connect_to_gateway_ap();
void dns_req(char *);
char *i2c_serialize(char *, int, char *, BYTE *, int);
I2CData *i2c_deserialize(char *);

