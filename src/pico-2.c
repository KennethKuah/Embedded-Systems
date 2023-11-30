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

void main_i2c_recv_task(__unused void *params) {
    setup_wifi();
    
    while (true) {
        printf("Waiting for I2C data...\n");
        char* serialized_data = i2c_recv();
        i2c_data_t *i2c_data = i2c_deserialize(serialized_data);
        
        // Pass the deserialized data (which contains the packet bytes)
        // to the net handler to query the internet for data
        // Handles sending data back to Pico 1
        net_handler_rx(i2c_data->data, i2c_data->data_len);
        free(serialized_data);
    }
}

void vLaunch() {

    TaskHandle_t task;
    xTaskCreate(main_i2c_recv_task, "MainThread", configMINIMAL_STACK_SIZE, NULL,
                MAIN_TASK_PRIORITY, &task);

    vTaskStartScheduler();
}

int main() {
    // Initialization
    stdio_init_all();
    init_i2c_pico_2();
    sleep_ms(2000);
    printf("Starting FreeRTOS on core 0:\n");

    vLaunch();
    return 0;
}
