#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define MAX_SSID_COUNT 20

// The reason why i made this a global variable is because i want to change it.
// char ap_name[33] = {0};
cyw43_ev_scan_result_t array_of_ssid[MAX_SSID_COUNT];
volatile int ARRAY_CTR = 0;
volatile bool timer_fired = false;
volatile uint64_t start_time = 0;
struct repeating_timer timer;
volatile bool timeout = false;
volatile int finished = 0;

char * setup_wifi_scan();