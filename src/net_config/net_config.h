#ifndef NET_CONFIG_H
#define NET_CONFIG_H
#include <stdio.h>
#include <string.h>
//
#include "pico/cyw43_arch.h"

#define MAX_SCAN_RESULTS 10
#define SCAN_TIMEOUT 15.00 // float value in seconds
#define AUTH_OPEN (0)
#define AUTH_WEP (0x1)
#define AUTH_WPA_PSK (0x2)
#define AUTH_WPA2_PSK (0x3)
#define AUTH_WPA_WPA2_PSK (0x4)
#define AUTH_WPA2_ENTERPRISE (0x5)

int init_wifi();
void set_auth_scan_mode(int mask);
void perform_wifi_scan();
char* select_ssid();
void configure_ap(char* ap_name, char* password);
int run_server();

#endif
