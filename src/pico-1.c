// Native C libraries
#include <stdio.h>
// Pico SDK C libraries
#include "pico/stdlib.h"
#include "pico/stdlib.h"
// FreeRTOS libraries
#include "FreeRTOS.h"
#include "task.h"
// custom drivers/helper libraries
#include "net_config.h"
#include "net_manager.h"
#include "i2c_bridge.h"

#define SKIP_SCAN 1

#define CLIENT_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define SERVER_TASK_PRIORITY				( tskIDLE_PRIORITY + 2UL )
#define WIFI_SCAN_TASK_PRIORITY				( tskIDLE_PRIORITY + 3UL )

bool ap_configured = false;
bool comonplease = false;

const uint8_t pppdata[] = {
    // 0xB8, 0x08, 0xCF, 0xA8, 0xB0, 0x9C, 0xF8, 0x3A, 0xDD, 0x45, 0xCF, 0x16, 
    // 0x08, 0x00, 0x45, 0x00, 0x00, 0x45, 0x97, 0x8D, 0x00, 0x00, 0x80, 0x11, 
    // 0x19, 0xB9, 0xC0, 0xA8, 0x04, 0x1, 0xC0, 0xA8, 0x04, 0x45, 0xD6, 0x49, 
    // 0x00, 0x35, 0x00, 0x31, 0x64, 0xBE, 0x29, 0xD4, 0x01, 0x00, 0x00, 0x01, 
    // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x77, 0x77, 0x77, 0x0F, 0x6D, 
    // 0x73, 0x66, 0x74, 0x63, 0x6F, 0x6E, 0x6E, 0x65, 0x63, 0x74, 0x74, 0x65, 
    // 0x73, 0x74, 0x03, 0x63, 0x6F, 0x6D, 0x00, 0x00, 0x01, 0x00, 0x01
    //0xD8, 0x3A, 0xDD, 0x57, 0xB2, 0xC8, 0xB8, 0x08, 0xCF, 0xA8, 0xB0, 0x9C, 0x08, 0x00, 0x45, 0x00, 0x01, 0xEC, 0xEC, 0xF0, 0x40, 0x00, 0x80, 0x06, 0x82, 0xB9, 0xC0, 0xA8, 0x04, 0x45, 0xC0, 0xA8, 0x04, 0x01, 0xE3, 0x73, 0x00, 0x50, 0xDB, 0x05, 0x27, 0xCF, 0x00, 0x00, 0x1C, 0x7D, 0x50, 0x18, 0xFA, 0xF0, 0xB3, 0xCD, 0x00, 0x00, 0x47, 0x45, 0x54, 0x20, 0x2F, 0x20, 0x48, 0x54, 0x54, 0x50, 0x2F, 0x31, 0x2E, 0x31, 0x0D, 0x0A, 0x48, 0x6F, 0x73, 0x74, 0x3A, 0x20, 0x31, 0x39, 0x32, 0x2E, 0x31, 0x36, 0x38, 0x2E, 0x34, 0x2E, 0x31, 0x0D, 0x0A, 0x43, 0x6F, 0x6E, 0x6E, 0x65, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x3A, 0x20, 0x6B, 0x65, 0x65, 0x70, 0x2D, 0x61, 0x6C, 0x69, 0x76, 0x65, 0x0D, 0x0A, 0x43, 0x61, 0x63, 0x68, 0x65, 0x2D, 0x43, 0x6F, 0x6E, 0x74, 0x72, 0x6F, 0x6C, 0x3A, 0x20, 0x6D, 0x61, 0x78, 0x2D, 0x61, 0x67, 0x65, 0x3D, 0x30, 0x0D, 0x0A, 0x55, 0x70, 0x67, 0x72, 0x61, 0x64, 0x65, 0x2D, 0x49, 0x6E, 0x73, 0x65, 0x63, 0x75, 0x72, 0x65, 0x2D, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x73, 0x3A, 0x20, 0x31, 0x0D, 0x0A, 0x55, 0x73, 0x65, 0x72, 0x2D, 0x41, 0x67, 0x65, 0x6E, 0x74, 0x3A, 0x20, 0x4D, 0x6F, 0x7A, 0x69, 0x6C, 0x6C, 0x61, 0x2F, 0x35, 0x2E, 0x30, 0x20, 0x28, 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x20, 0x4E, 0x54, 0x20, 0x31, 0x30, 0x2E, 0x30, 0x3B, 0x20, 0x57, 0x69, 0x6E, 0x36, 0x34, 0x3B, 0x20, 0x78, 0x36, 0x34, 0x29, 0x20, 0x41, 0x70, 0x70, 0x6C, 0x65, 0x57, 0x65, 0x62, 0x4B, 0x69, 0x74, 0x2F, 0x35, 0x33, 0x37, 0x2E, 0x33, 0x36, 0x20, 0x28, 0x4B, 0x48, 0x54, 0x4D, 0x4C, 0x2C, 0x20, 0x6C, 0x69, 0x6B, 0x65, 0x20, 0x47, 0x65, 0x63, 0x6B, 0x6F, 0x29, 0x20, 0x43, 0x68, 0x72, 0x6F, 0x6D, 0x65, 0x2F, 0x31, 0x31, 0x39, 0x2E, 0x30, 0x2E, 0x30, 0x2E, 0x30, 0x20, 0x53, 0x61, 0x66, 0x61, 0x72, 0x69, 0x2F, 0x35, 0x33, 0x37, 0x2E, 0x33, 0x36, 0x0D, 0x0A, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x3A, 0x20, 0x74, 0x65, 0x78, 0x74, 0x2F, 0x68, 0x74, 0x6D, 0x6C, 0x2C, 0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2F, 0x78, 0x68, 0x74, 0x6D, 0x6C, 0x2B, 0x78, 0x6D, 0x6C, 0x2C, 0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2F, 0x78, 0x6D, 0x6C, 0x3B, 0x71, 0x3D, 0x30, 0x2E, 0x39, 0x2C, 0x69, 0x6D, 0x61, 0x67, 0x65, 0x2F, 0x61, 0x76, 0x69, 0x66, 0x2C, 0x69, 0x6D, 0x61, 0x67, 0x65, 0x2F, 0x77, 0x65, 0x62, 0x70, 0x2C, 0x69, 0x6D, 0x61, 0x67, 0x65, 0x2F, 0x61, 0x70, 0x6E, 0x67, 0x2C, 0x2A, 0x2F, 0x2A, 0x3B, 0x71, 0x3D, 0x30, 0x2E, 0x38, 0x2C, 0x61, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x2F, 0x73, 0x69, 0x67, 0x6E, 0x65, 0x64, 0x2D, 0x65, 0x78, 0x63, 0x68, 0x61, 0x6E, 0x67, 0x65, 0x3B, 0x76, 0x3D, 0x62, 0x33, 0x3B, 0x71, 0x3D, 0x30, 0x2E, 0x37, 0x0D, 0x0A, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2D, 0x45, 0x6E, 0x63, 0x6F, 0x64, 0x69, 0x6E, 0x67, 0x3A, 0x20, 0x67, 0x7A, 0x69, 0x70, 0x2C, 0x20, 0x64, 0x65, 0x66, 0x6C, 0x61, 0x74, 0x65, 0x0D, 0x0A, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x2D, 0x4C, 0x61, 0x6E, 0x67, 0x75, 0x61, 0x67, 0x65, 0x3A, 0x20, 0x65, 0x6E, 0x2D, 0x55, 0x53, 0x2C, 0x65, 0x6E, 0x3B, 0x71, 0x3D, 0x30, 0x2E, 0x39, 0x0D, 0x0A, 0x0D, 0x0A
    ///0xb8, 0x08, 0xcf, 0xa8, 0xb0, 0x9c, 0xd8, 0x3a, 0xdd, 0x57, 0xb2, 0xc8, 0x08, 0x00, 0x45, 0x00, 0x00, 0x34, 0x20, 0x19, 0x40, 0x00, 0x80, 0x06, 0x51, 0x49, 0xc0, 0xa8, 0x04, 0x01, 0xc0, 0xa8, 0x04, 0x10, 0xf1, 0x49, 0x00, 0x50, 0x25, 0xd0, 0xd2, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x80, 0x02, 0xfa, 0xf0, 0x00, 0xe8, 0x00, 0x00, 0x02, 0x04, 0x05, 0xb4, 0x01, 0x03, 0x03, 0x08 , 0x1, 0x01, 0x04, 0x02    
};

uint16_t ppdata_length = sizeof(pppdata);

void please_task(__unused void *params)
{
    while(!comonplease)
    {
        vTaskDelay(100);
    }
    ip4_addr_t des;
    des.addr = 0xC0A80410;
    // Allocate a pbuf for the data
    struct pbuf* q = pbuf_alloc(PBUF_RAW, ppdata_length, PBUF_RAM);
    if (q == NULL) {
        // Failed to allocate pbuf, handle error or return NULL
        return;
    }

    // Copy data to the payload of the pbuf
    if (pbuf_take(q, pppdata, ppdata_length) != ERR_OK) {
        // Error copying data, free the pbuf and handle error or return NULL
        pbuf_free(q);
        return;
    }
    ip4_addr_t newsource;
    newsource.addr = 0xC0A80445;
    struct udp_pcb *up = udp_new();
    up->remote_ip = newsource;
    IP4_ADDR(&des, 192, 168, 4, 16);
    printf("yes\n");
    for(int i = 0;i < 20; i++)
    {
        //printf("CUSTOMDNS: %d", udp_sendto(up, q, &des, 6969));
        printf("CUSTOMDNS%d\n", cyw43_send_ethernet(&cyw43_state, CYW43_ITF_AP, ppdata_length, pppdata, false));
    }//printf("AfterDHCP: %d", nif->output(nif, q, &des));
    while(true)
    {
        vTaskDelay(100);
    }
}

void client_task(__unused void *params)
{
    vTaskDelay(500);
    int server_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(0);
    memset(&server_addr, 0, sizeof(server_addr));
    
    if (server_sock < 0) {
        printf("Failed to create socket\n");
    }

    printf("Socket created");

    while(true) {
        char *serialized_data = i2c_recv();
        i2c_data_t* i2c_data = i2c_deserialize(serialized_data);
        char* dst_addr = i2c_data->tag;
        inet_aton(dst_addr, &server_addr.sin_addr);

        int sent_bytes =
        sendto(server_sock, i2c_data->data, i2c_data->data_len, 0, (struct sockaddr *)&server_addr,
                sizeof(server_addr));
        if (sent_bytes) {
            printf("[+] Successfully sent %d bytes\n", sent_bytes);
        }
        else {
            printf("[!] Failed to send");
        }
        //cyw43_send_ethernet(&cyw43_state, CYW43_ITF_AP, i2c_data->data_len, i2c_data->data, false);
        // struct pbuf* p = pbuf_alloc(PBUF_RAW, ppdata_length, PBUF_RAM);
        // if (p == NULL) {
        //     // Failed to allocate pbuf, handle error or return NULL
        //     return;
        // }

        // Copy data to the payload of the pbuf
        // if (pbuf_take(p, i2c_data->data, i2c_data->data_len) != ERR_OK) {
        //     // Error copying data, free the pbuf and handle error or return NULL
        //     pbuf_free(p);
        //     return;
        // }
        // Access the TCP header by offsetting the payload pointer
        //struct tcp_hdr *tcphdr = (struct tcp_hdr *)((u8_t *)p->payload);

        // Print TCP header fields
        // printf("Source Port: %u\n", ntohs(tcphdr->src));
        // printf("Destination Port: %u\n", ntohs(tcphdr->dest));
        // printf("Sequence Number: %lu\n", ntohl(tcphdr->seqno));
        // printf("Acknowledgment Number: %lu\n", ntohl(tcphdr->ack_num));
        // Loop through the pbuf chain to access each buffer
        // for (struct pbuf *q = p; q != NULL; q = q->next) {
        //     // Print the payload (data) in each buffer (pbuf)
        //     for (u16_t i = 0; i < q->len; ++i) {
        //         // Access and print each byte of the payload
        //         printf("%02X ", ((u8_t *)q->payload)[i]);

        //         // Print a new line after every 16 bytes for better readability
        //         if ((i + 1) % 16 == 0) {
        //             printf("\n");
        //         }
        //     }
        // }

        // printf("\n"); // Print a newline at the end
        //cyw43_send_ethernet(&cyw43_state, CYW43_ITF_AP, )
        vTaskDelay(100);
    }
}

void wifi_scan_task(__unused void *params) {
    printf("Initializing WiFi drivers...\n");
    if (init_wifi()) {
        return;
    }
#if !SKIP_SCAN
    // Set the Wifi scan mode for open networks
    // set_auth_scan_mode(CYW43_AUTH_OPEN);

    // Perform the scan and select an AP name to copy
    perform_wifi_scan();
    char* ap_name = select_ssid();
    configure_ap(ap_name, NULL);
#else
    configure_ap("PICOW_TESTING", NULL);
#endif
    ap_configured = true;
    while(true) {
        // not much to do as this function is not called again
        vTaskDelay(10000);
    }
}

void server_task(__unused void *params) {
    // Wait will AP has been configured
    while (!ap_configured) {
        vTaskDelay(100);
    }
    
    // Start running the server
    run_server();
}

void vLaunch(void) {
    TaskHandle_t wifiScanTask;
    TaskHandle_t serverTask;
    TaskHandle_t clientTask;

    xTaskCreate(wifi_scan_task, "WiFiScanThread", configMINIMAL_STACK_SIZE, NULL, WIFI_SCAN_TASK_PRIORITY, &wifiScanTask);
    xTaskCreate(server_task, "ServerHandlingThread", configMINIMAL_STACK_SIZE, NULL, SERVER_TASK_PRIORITY, &serverTask);
    xTaskCreate(client_task, "ClientHandlingThread", configMINIMAL_STACK_SIZE, NULL, SERVER_TASK_PRIORITY, &clientTask);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main() {
    // main body of code
    stdio_init_all();
    init_i2c_pico_1();
    sleep_ms(3000);
    printf("Starting FreeRTOS on core 0:\n");

    vLaunch();
    return 0;
}
