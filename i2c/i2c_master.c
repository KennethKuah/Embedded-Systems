#include "pico/stdlib.h"
#include "hardware/i2c.h"

int main() {
    stdio_init_all();

    i2c_init(i2c0, 100000);  // Initialize I2C with a 100 kHz clock speed
    gpio_set_function(2, GPIO_FUNC_I2C);  // Set GPIO 2 as SDA
    gpio_set_function(3, GPIO_FUNC_I2C);  // Set GPIO 3 as SCL
    i2c_set_slave_mode(i2c0, false);  // Set as Master

    uint8_t slave_address = 0x08;  // Replace with your Slave's address

    while (1) {
        uint8_t data_to_send = 0x55;  // Data to send to the Slave
        i2c_write_blocking(i2c0, slave_address, &data_to_send, 1, false);
        sleep_ms(1000);
    }
}