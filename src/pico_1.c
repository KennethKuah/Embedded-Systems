#include <stdio.h>
//
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
//
#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"
//
#include "access_point/access_point.h"
#include "wifi/wifi.h"
#include "i2c_helper/i2c.h"

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)
#define PRINT_TASK_PRIORITY (tskIDLE_PRIORITY + 5UL)
#define mbaTASK_MESSAGE_BUFFER_SIZE       ( 60 )
static MessageBufferHandle_t list_of_ssid_buffer;

void scan_task(__unused void *params) {
    cyw43_ev_scan_result_t * ptr_to_ssid_array = setup_wifi_scan();

    while (true) {
        vTaskDelay(100);
        xMessageBufferSend(list_of_ssid_buffer, (void *)&ptr_to_ssid_array, sizeof(ptr_to_ssid_array), 0);
    }

    cyw43_arch_deinit();
}

void ap_task(__unused void *params) {
    cyw43_ev_scan_result_t * fReceivedData;
    size_t xReceivedBytes = 0;
    while(xReceivedBytes == 0){
        xReceivedBytes = xMessageBufferReceive(list_of_ssid_buffer, (void *)&fReceivedData, sizeof(fReceivedData), portMAX_DELAY);
        // printf("This is the scan result: %s\n", (cyw43_ev_scan_result_t *)fReceivedData[1].ssid);
    }
    for(int i = 0; i < 20; i++) {
        printf("%i. SSID: ", (i + 1));
        if (strlen(fReceivedData[i].ssid) == 0)
            printf("Hidden Network\t");
        else
            printf("%s\t", fReceivedData[i].ssid);
        printf("AUTH MODE: %u\n", fReceivedData[i].auth_mode); 
    }

    setup_ap(fReceivedData);

    while (true) {
        // not much to do as LED is in another task, and we're using RAW
        // (callback) lwIP API
        vTaskDelay(100);
    }

    cyw43_arch_deinit();

}

void vLaunch() {
    list_of_ssid_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    TaskHandle_t scanTask;
    xTaskCreate(scan_task, "scanWifiResults", configMINIMAL_STACK_SIZE, NULL,
                TEST_TASK_PRIORITY, &scanTask);
    
    TaskHandle_t apTask;
    xTaskCreate(ap_task, "TestAvgThread", configMINIMAL_STACK_SIZE, NULL, PRINT_TASK_PRIORITY, &apTask);

    vTaskStartScheduler();
}

int main() {
    // main body of code
    stdio_init_all();
    init_pico_1();
    sleep_ms(3000);
    const char *rtos_name = "FreeRTOS";
    printf("Starting %s on core 0:\n", rtos_name);

    vLaunch();

    return 0;
}
