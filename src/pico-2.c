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
    // test_conns();
    while (true) {
        // not much to do as LED is in another task, and we're using RAW
        // (callback) lwIP API
        vTaskDelay(100);
    }

    cyw43_arch_deinit();
}

void dns_task(__unused void *params) {
    //setup_wifi();
    
    while (true) {
        char* serialized_data = i2c_recv();
        i2c_data_t *i2c_data = i2c_deserialize(serialized_data);
        free(serialized_data);
        
        BYTE out_data[512];

        printf("Received dns query: %d bytes\n", i2c_data->data_len);
        // for(int i = 0; i < i2c_data->data_len; ++i) {
        //     printf("%x ", i2c_data->data[i]);
        // }
        int bytes_received = send_dns_req(i2c_data->data, i2c_data->data_len, out_data);

        serialized_data = i2c_serialize("dns", out_data, bytes_received);
        printf("Sending dns response: %s\n", serialized_data);
        i2c_send(serialized_data);
        free(serialized_data);
    }
}

void vLaunch()
{

    TaskHandle_t task;
    xTaskCreate(dns_task, "MainThread", configMINIMAL_STACK_SIZE, NULL,
                MAIN_TASK_PRIORITY, &task);

    vTaskStartScheduler();
}

int main()
{
    // main body of code
    stdio_init_all();
    init_i2c_pico_2();
    sleep_ms(2000);
    printf("Starting FreeRTOS on core 0:\n");

    vLaunch();
    return 0;
}
