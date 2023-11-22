#include <stdio.h>
#include "pico/stdlib.h"
#include "deauth/deauth.h"
#include "picowi_defs.h"
#include "picowi_pico.h"
#include "picowi_wifi.h"
#include "picowi_ioctl.h"
#include "picowi_event.h"
#include "picowi_join.h"
#include "picowi_ip.h"
#include "picowi_dhcp.h"
#include "picowi_net.h"
#include "picowi_tcp.h"
#include "picowi_web.h"
#include "picowi_init.h"
#define SSID "cat"
#define PASSWD "catcatcat"

int main()
{
    io_init();
    sleep_ms(5000);
    set_display_mode(DISP_INFO | DISP_JOIN | DISP_TCP_STATE | DISP_DATA);
    if (net_init() && net_join(SSID, PASSWD))
    {
        while (1)
        {
            sleep_ms(1000);
            deauth();
            net_event_poll();
            net_state_poll();
        }
    }
    // for (;;) {
    //     sleep_ms(3000);
    //     deauth();
    // }
    return 0;
}