#include "pico/stdlib.h"
#include "wifi/wifi.h"
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#define MAIN_TASK_PRIORITY (tskIDLE_PRIORITY + 2UL)

void main_task(__unused void *params)
{
    setup_wifi();
    test_conns();
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
    xTaskCreate(main_task, "MainThread", configMINIMAL_STACK_SIZE, NULL,
                MAIN_TASK_PRIORITY, &task);

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
