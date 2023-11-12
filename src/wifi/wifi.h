#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "lwip/ip4.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <stdio.h>

int setup_wifi();
void wifi_est_socket(char *ip, int port);
