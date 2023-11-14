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

void wifi_task(__unused void *params){
    printf("Entered wifi task\n");
    setup_wifi_scan();
    cyw43_ev_scan_result_t * ptr_to_ssid_array;
    ptr_to_ssid_array = return_array();
    xMessageBufferSend(list_of_ssid_buffer, (void *) ptr_to_ssid_array, sizeof(ptr_to_ssid_array), 0);

    while(true){
        vTaskDelay(100);
    }

    cyw43_arch_deinit();
}

// void ap_task(__unused void *params)
// {
//     cyw43_ev_scan_result_t * fReceivedData;
//     xMessageBufferReceive(list_of_ssid_buffer, (void *)fReceivedData, sizeof(fReceivedData), portMAX_DELAY);
//     setup_ap(fReceivedData);

//     while (true) {
//         // not much to do as LED is in another task, and we're using RAW
//         // (callback) lwIP API
//         vTaskDelay(100);
//     }

//     cyw43_arch_deinit();
// }


void vLaunch()
{
    TaskHandle_t wifi;
    xTaskCreate(wifi_task, "WIFI_TASK", configMINIMAL_STACK_SIZE+256, NULL, TEST_TASK_PRIORITY, &wifi);

    // TaskHandle_t ap;
    // xTaskCreate(ap_task, "AP_TASK", configMINIMAL_STACK_SIZE, NULL, 5, &ap);

    // list_of_ssid_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    vTaskStartScheduler();
}

int main()
{
    // main body of code
    // stdio_usb_init();
    stdio_init_all();
    sleep_ms(10000);
    const char *rtos_name = "FreeRTOS";
    printf("Starting %s on core 0:\n", rtos_name);

    vLaunch();

    return 0;
}