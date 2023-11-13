#include "pico/stdlib.h"
#include "hardware/i2c.h"

int main() {
    stdio_init_all();

    i2c_init(i2c0, 100000);  // Initialize I2C with a 100 kHz clock speed
    gpio_set_function(2, GPIO_FUNC_I2C);  // Set GPIO 2 as SDA
    gpio_set_function(3, GPIO_FUNC_I2C);  // Set GPIO 3 as SCL
    i2c_set_slave_mode(i2c0, true);  // Set as Slave
    i2c_set_slave_address(i2c0, 0x08);  // Replace with a unique address

    while (1) {
        uint8_t received_data;
        uint8_t received_bytes;

        i2c_read_blocking(i2c0, &received_data, 1, false);
        printf("Received Data: 0x%02X\n", received_data);
        sleep_ms(1000);
    }
}