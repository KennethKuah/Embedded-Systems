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

// Flag to set for skipping the WiFi Scan
#define SKIP_SCAN 0

#define CLIENT_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define SERVER_TASK_PRIORITY				( tskIDLE_PRIORITY + 2UL )
#define WIFI_SCAN_TASK_PRIORITY				( tskIDLE_PRIORITY + 3UL )

bool ap_configured = false;
bool comonplease = false;

void client_task(__unused void *params)
{
    // Wait will AP has been configured
    while (!ap_configured) {
        vTaskDelay(200);
    }
    
    // Upon receiving data from I2C channel, creates a socket to original 
    // source destination port and sends the data back to the client, 
    // in order to simulate the internet connection
    while(true) {
        char *serialized_data = i2c_recv();
        i2c_data_t* i2c_data = i2c_deserialize(serialized_data);
        free(serialized_data);

        int server_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(0);
        
        if (server_sock < 0) 
            printf("Failed to create socket\n");
        else
            printf("Socket created");
        char* dst_addr = i2c_data->tag;
        inet_aton(dst_addr, &client_addr.sin_addr);

        int sent_bytes =
        sendto(server_sock, i2c_data->data, i2c_data->data_len, 0, (struct sockaddr *)&client_addr,
                sizeof(client_addr));
        if (sent_bytes) {
            printf("[+] Successfully sent %d bytes\n", sent_bytes);
        }
        else {
            printf("[!] Failed to send");
        }
        free(i2c_data);
        close(server_sock);
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
    // Initialization
    stdio_init_all();
    init_i2c_pico_1();
    sleep_ms(2000);
    printf("Starting FreeRTOS on core 0:\n");

    vLaunch();
    return 0;
}
