#include "pico/stdlib.h"
#include "wifi/wifi.h"
#include "pico/cyw43_arch.h"
#include "access_point/access_point.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "message_buffer.h"

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)
#define PRINT_TASK_PRIORITY (tskIDLE_PRIORITY + 5UL)
#define mbaTASK_MESSAGE_BUFFER_SIZE       ( 60 )
static MessageBufferHandle_t list_of_ssid_buffer;

void main_task(__unused void *params)
{
    
    cyw43_ev_scan_result_t * ptr_to_ssid_array = setup_wifi_scan();
    // printf("This is the scan result: %s\n", ptr_to_ssid_array[1].ssid);
    // for(int i = 0; i < 20; i++) {
    //     printf("%i. SSID: ", (i + 1));
    //     if (strlen(ptr_to_ssid_array[i].ssid) == 0)
    //         printf("Hidden Network\t");
    //     else
    //         printf("%s\t", ptr_to_ssid_array[i].ssid);
    //     printf("AUTH MODE: %u\n", ptr_to_ssid_array[i].auth_mode); 
    // }

    // xMessageBufferSend(list_of_ssid_buffer, (void *)ptr_to_ssid_array, sizeof(*ptr_to_ssid_array), 0);

    while (true) {
        // not much to do as LED is in another task, and we're using RAW
        // (callback) lwIP API
        vTaskDelay(100);
        xMessageBufferSend(list_of_ssid_buffer, (void *)&ptr_to_ssid_array, sizeof(ptr_to_ssid_array), 0);
    }

    cyw43_arch_deinit();
}

void avg_task(__unused void *params) {
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

// void access_task(__unused void *params)
// {
//     setup_ap();
// }

void vLaunch()
{
    list_of_ssid_buffer = xMessageBufferCreate(mbaTASK_MESSAGE_BUFFER_SIZE);

    TaskHandle_t task;
    xTaskCreate(main_task, "TestMainThread", configMINIMAL_STACK_SIZE, NULL,
                TEST_TASK_PRIORITY, &task);
    
    TaskHandle_t avgtask;
    xTaskCreate(avg_task, "TestAvgThread", configMINIMAL_STACK_SIZE, NULL, PRINT_TASK_PRIORITY, &avgtask);
    // TaskHandle_t access_point_task;
    // xTaskCreate(access_task, "AccessPointThread", configMINIMAL_STACK_SIZE, NULL,
    // 5, &access_point_task);
    
    

    vTaskStartScheduler();
}

int main()
{
    // main body of code
    stdio_init_all();
    sleep_ms(5000);
    const char *rtos_name = "FreeRTOS";
    printf("Starting %s on core 0:\n", rtos_name);

    vLaunch();

    return 0;
}
