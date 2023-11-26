#include <stdio.h>
#include "deauth/deauth.h"
#include "dhcp_flood/dhcp_flood.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define SSID "SSID"
#define PASSWD "PASSWORD"

int main()
{    
    stdio_init_all();
    sleep_ms(5000);
    if (cyw43_arch_init())
    {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    for (;;) {
        if (joinAP(SSID, PASSWD)) {
            deauth();
            //send_dhcp_discover();
        }
    }
}

