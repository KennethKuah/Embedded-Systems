#include "pico/stdlib.h"
#include "access_point/access_point.h"
#include "wifi_scan/wifi_scan.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)
#define mbaTASK_MESSAGE_BUFFER_SIZE       ( 60 )
static MessageBufferHandle_t list_of_ssid_buffer;


void ap_task(__unused void *params)
{
    setup_ap();

    while (true) {
        // not much to do as LED is in another task, and we're using RAW
        // (callback) lwIP API
        vTaskDelay(100);
    }

    cyw43_arch_deinit();
}

void wifi_task(__unused void *params){
    char * ptr_to_ssid_array;
    ptr_to_ssid_array = setup_wifi_scan();
    xMessageBufferSend(list_of_ssid_buffer, (void *) ptr_to_ssid_array, sizeof(ptr_to_ssid_array), 0);

    while(true){
        vTaskDelay(100);
    }

    cyw43_arch_deinit();
}

void vLaunch()
{
    TaskHandle_t wifi;
    xTaskCreate(wifi_task, "WIFI_TASK", configMINIMAL_STACK_SIZE, NULL, 5, &wifi);

    TaskHandle_t ap;
    xTaskCreate(ap_task, "AP_TASK", configMINIMAL_STACK_SIZE, NULL, TEST_TASK_PRIORITY, &ap);

    list_of_ssid_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    vTaskStartScheduler();
}

int main()
{
    // main body of code
    stdio_init_all();

    const char *rtos_name = "FreeRTOS";
    printf("Starting %s on core 0:\n", rtos_name);

    vLaunch();

    return 0;
}