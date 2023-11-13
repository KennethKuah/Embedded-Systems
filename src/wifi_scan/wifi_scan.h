#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define MAX_SSID_COUNT 20

// The reason why i made this a global variable is because i want to change it.
#ifndef MY_HEADER_H
#define MY_HEADER_H
extern cyw43_ev_scan_result_t array_of_ssid[MAX_SSID_COUNT];
extern volatile int ARRAY_CTR;
extern volatile bool timer_fired;
extern volatile uint64_t start_time;
extern struct repeating_timer timer;
extern volatile bool timeout;
extern volatile int finished;
#endif


cyw43_ev_scan_result_t * setup_wifi_scan();