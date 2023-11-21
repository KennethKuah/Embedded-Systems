#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>

static const uint I2C_SLAVE_ADDRESS = 0x04;
static const uint I2C_BAUDRATE = 100000; // 100 kHz
static const uint I2C_SLAVE_SDA_PIN = 6; // 
static const uint I2C_SLAVE_SCL_PIN = 7; // 

static struct
{
    uint8_t mem[256];
    uint8_t mem_address;
    bool mem_address_written;
} context;

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    static char received_data[256];
    static int data_index = 0;
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (!context.mem_address_written) {
            // writes always start with the memory address
            context.mem_address = i2c_read_byte_raw(i2c);
            context.mem_address_written = true;
            data_index = 0;
        } else {
            // Subsequent bytes are data
            uint8_t data = i2c_read_byte_raw(i2c);
            context.mem[context.mem_address] = data;

             // For plaintext processing, assuming ASCII characters are sent
            received_data[data_index++] = data;
            context.mem_address = (context.mem_address + 1) % sizeof(context.mem);

            // save into memory
            // context.mem[context.mem_address] = i2c_read_byte_raw(i2c);
            // context.mem_address++;
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
        // load from memory
        i2c_write_byte_raw(i2c, context.mem[context.mem_address]);
        context.mem_address++;
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        context.mem_address_written = false;
        // Assuming the transmission ends with a '\0' to indicate the end of a string
        received_data[data_index] = '\0'; // Null-terminate the string
        printf("Received: %s\n", received_data); // Process/print the complete string
        break;
    default:
        break;
    }
}

static void setup_slave() {
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c1, I2C_BAUDRATE);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c1, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
    //i2c_set_slave_mode(i2c1, true, I2C_SLAVE_ADDRESS); 
}

static void run_slave() {
    uint8_t msg_len = 32;
    uint8_t buf[32];
    while (1){
        // read all the bytes
        //int count = i2c_read_blocking(i2c1, I2C_SLAVE_ADDRESS, mem_addr, 1, false);
        if (i2c_get_read_available(i2c1) > 0){
            int count = i2c_read_blocking(i2c1, I2C_SLAVE_ADDRESS, buf, msg_len, false);
            buf[32] = '\0';
            printf("Read: '%s'\n", buf);
            puts("");
        }
        sleep_ms(500);
    }
}


int main(){
    stdio_init_all();
    setup_slave();
    run_slave();
    return 0;
}