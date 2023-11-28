// Native C libraries
#include <stdio.h>
// Pico SDK C libraries
#include "pico/stdlib.h"
// FreeRTOS libraries
#include "FreeRTOS.h"
#include "task.h"
// custom drivers/helper libraries
#include "net_config.h"
#include "i2c_bridge.h"
#include "pcap.h"
#include "netif/ethernet.h"
#include "pico/cyw43_arch.h"

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define SKIP_SCAN 1

#define WIFI_SCAN_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define SERVER_TASK_PRIORITY				( tskIDLE_PRIORITY + 2UL )

extern bool comonplease = false;
extern cyw43_t cyw43_state;

bool ap_configured = false;

const uint8_t pppdata[] = {
    // 0xB8, 0x08, 0xCF, 0xA8, 0xB0, 0x9C, 0xF8, 0x3A, 0xDD, 0x45, 0xCF, 0x16, 
    // 0x08, 0x00, 0x45, 0x00, 0x00, 0x45, 0x97, 0x8D, 0x00, 0x00, 0x80, 0x11, 
    // 0x19, 0xB9, 0xC0, 0xA8, 0x04, 0x1, 0xC0, 0xA8, 0x04, 0x45, 0xD6, 0x49, 
    // 0x00, 0x35, 0x00, 0x31, 0x64, 0xBE, 0x29, 0xD4, 0x01, 0x00, 0x00, 0x01, 
    // 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x77, 0x77, 0x77, 0x0F, 0x6D, 
    // 0x73, 0x66, 0x74, 0x63, 0x6F, 0x6E, 0x6E, 0x65, 0x63, 0x74, 0x74, 0x65, 
    // 0x73, 0x74, 0x03, 0x63, 0x6F, 0x6D, 0x00, 0x00, 0x01, 0x00, 0x01
    0xb8, 0x08, 0xcf, 0xa8, 0xb0, 0x9c, 0xd8, 0x3a, 0xdd, 0x57, 0xb2, 0xc8, 0x08, 0x00, 0x45, 0x00, 0x00, 0x34, 0x20, 0x19, 0x40, 0x00, 0x80, 0x06, 0x51, 0x49, 0xc0, 0xa8, 0x04, 0x01, 0xc0, 0xa8, 0x04, 0x10, 0xf1, 0x49, 0x00, 0x50, 0x25, 0xd0, 0xd2, 0x6b, 0x00, 0x00, 0x00, 0x00, 0x80, 0x02, 0xfa, 0xf0, 0x00, 0xe8, 0x00, 0x00, 0x02, 0x04, 0x05, 0xb4, 0x01, 0x03, 0x03, 0x08 , 0x1, 0x01, 0x04, 0x02    
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
    configure_ap("PICOYZ_TESTING", NULL);
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
    TaskHandle_t pTask;

    xTaskCreate(wifi_scan_task, "WiFiScanThread", configMINIMAL_STACK_SIZE, NULL, WIFI_SCAN_TASK_PRIORITY, &wifiScanTask);
    xTaskCreate(server_task, "ServerHandlingThread", configMINIMAL_STACK_SIZE, NULL, SERVER_TASK_PRIORITY, &serverTask);
    xTaskCreate(please_task, "PleaseHandlingThread", configMINIMAL_STACK_SIZE, NULL, SERVER_TASK_PRIORITY, &pTask);

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main() {
    // main body of code
    stdio_init_all();
    init_i2c_pico_1();
    if (SD_MOUNTED)
        set_pcap_file("capture.pcap");
    sleep_ms(3000);
    printf("Starting FreeRTOS on core 0:\n");

    vLaunch();
    return 0;
}

// int main() {
//     stdio_usb_init();
//     init_i2c_pico_1();
//     BYTE protocol = 0x16;
//     char* serialized_data = i2c_serialize("192.168.2.1", 85, &protocol, "hahaha", 6);
//     sleep_ms(1000);
//     while (true){
//         printf("Sending %s\n", serialized_data);
//         //send_i2c("Hello");
//         send_i2c(serialized_data);
//         sleep_ms(3000);
//     }

// }