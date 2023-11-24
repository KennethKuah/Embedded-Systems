#ifndef NET_CONFIG_H
#define NET_CONFIG_H
#include <stdio.h>
#include <string.h>
//
#include "pico/cyw43_arch.h"
#include "cyw43_ll.h"

#define MAX_SCAN_RESULTS 10
#define SCAN_TIMEOUT 15.00 // float value in seconds

int init_wifi();
void set_auth_scan_mode(int mask);
cyw43_ev_scan_result_t* wifi_scan();
char* select_ssid(cyw43_ev_scan_result_t * ssid_array);
void configure_ap(char* ap_name, char* password);
int run_server();

#endif
