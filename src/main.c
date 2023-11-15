#include "pico/stdlib.h"
#include "wifi/wifi.h"
#include "access_point/access_point.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#define TEST_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)

void main_task(__unused void *params)
{
    setup_wifi_scan();

    while (true) {
        // not much to do as LED is in another task, and we're using RAW
        // (callback) lwIP API
        vTaskDelay(100);
    }

    cyw43_arch_deinit();
}

void vLaunch()
{

    TaskHandle_t task;
    xTaskCreate(main_task, "TestMainThread", configMINIMAL_STACK_SIZE, NULL,
                TEST_TASK_PRIORITY, &task);
    
    

    vTaskStartScheduler();
}

int main()
{
    // main body of code
    stdio_init_all();
    sleep_ms(10000);
    const char *rtos_name = "FreeRTOS";
    printf("Starting %s on core 0:\n", rtos_name);

    vLaunch();

    return 0;
}
