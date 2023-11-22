#include "master.h"

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
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
        }
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
        // load from memory
        i2c_write_byte_raw(i2c, context.mem[context.mem_address]);
        context.mem_address++;
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        context.mem_address_written = false;
        received_data[data_index] = '\0';
        received = true;
        printf("Received from i2c0: %s\n", received_data);
        break;
    default:
        break;
    }
}

static void setup_slave(){
    printf("Setting up slave for i2c0...\n");
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
    i2c_slave_init(i2c0, I2C_CLIENT_ADDRESS, &i2c_slave_handler);
}

static void setup_master(){
    printf("Setting up master for i2c1...\n");
    gpio_init(I2C_MASTER_SDA_PIN);
    gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
    // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    gpio_pull_up(I2C_MASTER_SDA_PIN);

    gpio_init(I2C_MASTER_SCL_PIN);
    gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SCL_PIN);

    i2c_init(i2c1, I2C_BAUDRATE);
    i2c_set_slave_mode(i2c1, false, 0); 
}

static void send_slave(char * msg) {
    printf("Master starts to run (i2c1)...\n");
    for (uint8_t mem_address = 0;; mem_address = (mem_address + 32) % 256) {
        int count = 0;
        uint8_t msg_len = strlen(msg);

        // Now I will send the data
        uint8_t buf[32];
        buf[0] = mem_address;
        memcpy(buf + 1, msg, msg_len);
        sleep_ms(1000);
        count = i2c_write_blocking(i2c1, I2C_SLAVE_ADDRESS, buf, 1 + msg_len, false);
        if (count < 0) {
            puts("Couldn't write to slave, please check your wiring!");
            return;
        }
        hard_assert(count == 1 + msg_len);
        puts("");
        sleep_ms(2000);
    }
}

char * receive_master() {
    while (!received)
        sleep_ms(100);
    
    received = false;
    return received_data;
}


int master(char * packet_data) {
    stdio_init_all();
    sleep_ms(5000);
    setup_master();
    setup_slave();
    send_slave(packet_data);
    return 0;
}