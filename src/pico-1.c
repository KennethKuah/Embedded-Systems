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
#include "i2c_bridge.h"

#define SKIP_SCAN 0

#define WIFI_SCAN_TASK_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define SERVER_TASK_PRIORITY				( tskIDLE_PRIORITY + 2UL )

bool ap_configured = false;

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

    xTaskCreate(wifi_scan_task, "WiFiScanThread", configMINIMAL_STACK_SIZE, NULL, WIFI_SCAN_TASK_PRIORITY, &wifiScanTask);
    xTaskCreate(server_task, "ServerHandlingThread", configMINIMAL_STACK_SIZE, NULL, SERVER_TASK_PRIORITY, &serverTask);

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
