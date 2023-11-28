// Native C libraries
#include <stdio.h>
// Pico SDK C libraries
#include "pico/stdlib.h"
// FreeRTOS libraries
#include "FreeRTOS.h"
#include "task.h"
// custom drivers/helper libraries
#include "net_manager.h"
#include "i2c_bridge.h"

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
    init_i2c_pico_2();
    printf("Starting FreeRTOS on core 0:\n");

    vLaunch();
    return 0;
}
