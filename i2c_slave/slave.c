#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_BUF_LEN 256

static const uint I2C_SLAVE_ADDRESS = 0x04;
static const uint I2C_CLIENT_ADDRESS = 0x17;
static const uint I2C_BAUDRATE = 100000; // 100 kHz
static const uint I2C_MASTER_SDA_PIN = PICO_DEFAULT_I2C_SDA_PIN;
static const uint I2C_MASTER_SCL_PIN = PICO_DEFAULT_I2C_SCL_PIN;
static const uint I2C_SLAVE_SDA_PIN = 6; // 
static const uint I2C_SLAVE_SCL_PIN = 7; // 
volatile int data_index = 0;
volatile int written = 0;
char received_data[512];
volatile bool received = false;
// This needs to be a global variable is because I want to return it using another function
char * packet_data = NULL;

static struct
{
    uint8_t mem[MAX_BUF_LEN];
    uint8_t mem_address;
    bool mem_address_written;
} context;

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event) {
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (!context.mem_address_written) {
                // writes always start with the memory address
                context.mem_address_written = true;
                data_index = 0;
        } 
        uint8_t data = i2c_read_byte_raw(i2c);  
        received_data[data_index++] = data;
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
        // load from memory
        i2c_write_byte_raw(i2c, context.mem[context.mem_address]);
        context.mem_address++;
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        // Assuming the transmission ends with a '\0' to indicate the end of a string
        context.mem_address_written = false;
        written = 1;
        break;
    default:
        break;
    }
}

// Set up slave for i2c1
static void setup_slave() {
    printf("Setting up slave for i2c1...\n");
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c1, I2C_BAUDRATE);
    // configure I2C0 for slave mode
    i2c_slave_init(i2c1, I2C_SLAVE_ADDRESS, &i2c_slave_handler);
}

static void setup_master(){
    printf("Setting up master for i2c0...\n");
    gpio_init(I2C_MASTER_SDA_PIN);
    gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
    // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    gpio_pull_up(I2C_MASTER_SDA_PIN);

    gpio_init(I2C_MASTER_SCL_PIN);
    gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);
    i2c_set_slave_mode(i2c0, false, 0);
}

static void send_data(i2c_inst_t *i2c, const uint8_t slave_addr, const uint8_t *data, size_t data_size){
    for(size_t i = 0; i < data_size; i+= MAX_BUF_LEN){
        size_t chunk_size = data_size - i < MAX_BUF_LEN ? data_size - i : MAX_BUF_LEN;
        // Write in blocks of MAX_BUF_LEN bytes
        int bytes_written = i2c_write_blocking(i2c, slave_addr, data + i, chunk_size, false);
        if (bytes_written < 0) {
            // Handle error
            printf("Error writing to slave.\n");
            break;
        }
        sleep_ms(8000);
    }
}


static void run_master(char * sending_back){
    printf("Master starts to run (i2c0)...\n");
    int count = 0;
    int msg_len = strlen(sending_back);

    uint8_t buf_len[20];
    sprintf(buf_len, "%d", msg_len);
    int len = strlen(buf_len);
    printf("Sending the length\n");
    count = i2c_write_blocking(i2c0, I2C_CLIENT_ADDRESS, buf_len, len, false);
    sleep_ms(5000);

    uint8_t buf[msg_len];
    memcpy(buf, sending_back, msg_len);
    printf("Starting to send data...\n");
    send_data(i2c0, I2C_CLIENT_ADDRESS, buf, msg_len);
    puts("");
    sleep_ms(5000);
}

// Buffer is dynamically allocated, meaning it must be freed afer its used
static char * return_result(){
    while(!received)
        sleep_ms(100);
    
    received = false;
    return packet_data;
}

static void run_slave(char * sending_back) {
    printf("Running slave for i2c1...\n");
    bool created_dynamic_location = false;
    int size_of_data = 0;
    int ptr_in_packet_data = 0;
    while (1){
        if(written == 1){
            // This will run when the master sends the slave the bytes for the size of the data that will be send over
            // Checking if dynamic location is being created already
            if(!created_dynamic_location){
                received_data[data_index] = '\0';
                size_of_data = atoi(received_data);
                // +1 because we need an extra 1 space for the null terminating byte
                packet_data = (char *)malloc(sizeof(char) * (size_of_data + 1));
                created_dynamic_location = true;
                ptr_in_packet_data = 0;
            }
            else{
                // This should only run when the master sends the slave the data
                // The reason why data_index is used here is because data_index contains the number of bytes that the master has sent to the slave
                // So, using the same number of bytes, to write into the new dynamic buffer created by malloc
                memcpy(packet_data + ptr_in_packet_data, received_data, data_index);
                // Increment the pointer of ptr_in_packet_data so that we can memcpy the remaining bytes into the buffer
                ptr_in_packet_data += data_index;
                if(ptr_in_packet_data >= size_of_data){
                    packet_data[ptr_in_packet_data] = '\0';
                    printf("This is packet_data: %s\n", packet_data);
                    created_dynamic_location = false;
                    received = true;
                    // Sending back the response to master
                    run_master(sending_back);
                }

            }
            written = 0;
        }
        sleep_ms(500);
    }
}


int main(){
    stdio_init_all();
    setup_slave();
    setup_master();
    char sending_back[] = "HTTP/1.1 200 OK\r\n"
    "Date: Tue, 21 Nov 2023 12:45:26 GMT\r\n"
    "Server: Apache/2.4.1 (Unix)\r\n"
    "Content-Type: text/html; charset=UTF-8\r\n"
    "Content-Length: 1234\r\n"
    "Connection: close\r\n"
    "\r\n"
    "<!DOCTYPE html>\r\n"
    "<html>\r\n"
    "<head>\r\n"
    "    <title>Example Page</title>\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "    <h1>Hello, Master!</h1>\r\n"
    "    <p>This is an example page.</p>\r\n"
    "</body>\r\n"
    "</html>\r\n";
    run_slave(sending_back);
    return 0;
}