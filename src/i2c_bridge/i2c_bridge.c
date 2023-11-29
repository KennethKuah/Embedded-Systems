#include "i2c_bridge.h"

// The slave implements a 256 byte memory. To write a series of bytes, the master first
// writes the memory address, followed by the data. The address is automatically incremented
// for each byte transferred, looping back to 0 upon reaching the end. Reading is done
// sequentially from the current memory address.
static struct {
    uint8_t mem[MAX_BUF_LEN];
    uint8_t mem_address;
    bool mem_address_written;
} context;

// I2C Static Global variables
uint8_t I2C_MASTER_SDA_PIN; 
uint8_t I2C_MASTER_SCL_PIN;
uint8_t I2C_SLAVE_SDA_PIN;
uint8_t I2C_SLAVE_SCL_PIN;
i2c_inst_t* i2c_master_channel;
i2c_inst_t* i2c_slave_channel;
uint8_t master_address;
uint8_t slave_address;
// I2C ISR handling vars
volatile int data_index = 0;
volatile bool written = false;
volatile bool ack_to_send = false;
volatile bool acknowledged = false;
volatile bool created_dynamic_location = false;
const char ack[] = "ACK";
char received_len[8];
char* packet_data = NULL;

// Serialises data that is transmitted via I2C.
// Buffer is dynamically allocated, meaning it must be freed after its used
// Returns a string in the following format
// `tag:base64_data:base64_data_length`
// \param tag the type of data that is being sent (e.g. frame, dns);
// \param data data to be encoded in base64
// \param data_len length of data to be encoded
char *i2c_serialize(char *tag, BYTE *data, int data_len) {
    char data_encoded[MAX_MESSAGE_SIZE - 256];
    const char *delimiter = DELIMITER;
    int encoded_len = base64_encode(data_encoded, data, data_len);
    char *buf = (char *)calloc(MAX_MESSAGE_SIZE, sizeof(char));
    char data_len_str[10];
    sprintf(data_len_str, "%d", data_len);
    strcat(buf, tag);
    strcat(buf, delimiter);
    strcat(buf, data_encoded);
    strcat(buf, delimiter);
    strcat(buf, data_len_str);

    return buf;
}

// Deserializes data that is transmitted via I2C
// Expects a string in the following format
// `tag:base64_data:base64_data_length`
// \param buf serialized data received from I2C channel
i2c_data_t *i2c_deserialize(char *buf) {
    int idx = 0;
    int oset = 0;
    char tokens[3][MAX_MESSAGE_SIZE / 4];
    for (int i = 0; i <= strlen(buf); ++i) {
        if (buf[i] == ':') {
            // null terminate token
            tokens[idx][oset] = '\x00';
            idx++;
            oset = 0;
        }
        else {
            tokens[idx][oset++] = buf[i];
        }
    }

    char data_decoded[MAX_MESSAGE_SIZE - 256];
    char data_encoded[MAX_MESSAGE_SIZE - 256];

    strcpy(data_encoded, tokens[1]);
    int decoded_len =
        base64_decode(data_decoded, data_encoded, strlen(data_encoded));

    i2c_data_t *i2c_data = (i2c_data_t *)malloc(sizeof(i2c_data_t));

    if (i2c_data != NULL) {
        i2c_data->tag = tokens[0];
        i2c_data->data = data_decoded;
        i2c_data->data_len = decoded_len;
    }

    return i2c_data;
}

// Our handler is called from the I2C ISR, so it must complete quickly. Blocking calls /
// printing to stdio may interfere with interrupt handling.
static void i2c_slave_handler(i2c_inst_t* i2c, i2c_slave_event_t event) {
    switch (event) {
    case I2C_SLAVE_RECEIVE: // master has written some data
        if (!context.mem_address_written) {
            // writes always start with the memory address
            context.mem_address_written = true;
            data_index = 0;
        }
        uint8_t data = i2c_read_byte_raw(i2c);
        if (!created_dynamic_location)
            received_len[data_index++] = data;
        else
            packet_data[data_index++] = data;
        break;
    case I2C_SLAVE_REQUEST: // master is requesting data
        if (ack_to_send) {
            for (size_t i = 0; i < sizeof(ack); i++) {
                i2c_write_byte_raw(i2c, ack[i]);
            }
            ack_to_send = false;
        }
        break;
    case I2C_SLAVE_FINISH: // master has signalled Stop / Restart
        // Assuming the transmission ends with a '\0' to indicate the end of a string
        context.mem_address_written = false;
        written = true;
        ack_to_send = true;
        break;
    default:
        break;
    }
}

static void read_data_from_slave(i2c_inst_t *i2c, uint8_t *data, size_t data_size){
    // absolute_time_t curr_time = get_absolute_time();
    // absolute_time_t timeout = delayed_by_ms(curr_time, 2000);
    int bytes_written = 0;
    bytes_written = i2c_read_blocking(i2c, master_address, data, data_size, false);
    if (strcmp((const char*)data, ack) == 0) {
#if DEBUG_I2C
        printf("Acknowledgment received from slave: %s\n", data);
#endif
        acknowledged = true;
    } else {
        printf("Failed to receive acknowledgment from slave, please check your wiring!\n");
    }
}

static void send_data(i2c_inst_t* i2c, const uint8_t* data, size_t data_size) {
    for (size_t i = 0; i < data_size; i += MAX_BUF_LEN) {
        size_t chunk_size = data_size - i < MAX_BUF_LEN ? data_size - i : MAX_BUF_LEN;
        // Write in blocks of MAX_BUF_LEN bytes
        int bytes_written = i2c_write_blocking(i2c, master_address, data + i, chunk_size, false);
        if (bytes_written < 0) {
            // Handle error
            printf("Error writing to slave.\n");
            break;
        }
    }
}

void setup_master(i2c_inst_t *i2c_channel) {
    i2c_master_channel = i2c_channel;
    gpio_init(I2C_MASTER_SDA_PIN);
    gpio_set_function(I2C_MASTER_SDA_PIN, GPIO_FUNC_I2C);
    // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    gpio_pull_up(I2C_MASTER_SDA_PIN);

    gpio_init(I2C_MASTER_SCL_PIN);
    gpio_set_function(I2C_MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_MASTER_SCL_PIN);

    i2c_init(i2c_channel, I2C_BAUDRATE);
    i2c_set_slave_mode(i2c_channel, false, 0); 
}

void setup_slave(i2c_inst_t *i2c_channel, uint8_t *slave_address) {
    i2c_slave_channel = i2c_channel;
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c_channel, I2C_BAUDRATE);
    i2c_slave_init(i2c_channel, *slave_address, &i2c_slave_handler);
}

int i2c_send(char* msg) {
    int msg_len = strlen(msg);

    uint8_t buf_len[20];
    sprintf(buf_len, "%d", msg_len);
    int len = strlen(buf_len);
#if DEBUG_I2C
    printf("Sending length: %s\n", buf_len);
#endif
    // Send the length to slave
    i2c_write_blocking(i2c_master_channel, master_address, buf_len, len, false);
    // Read the acknowledgment from the slave
    uint8_t ack_buf[4] = {0}; // 4 bytes for "ACK" + null terminator
    read_data_from_slave(i2c_master_channel, ack_buf, sizeof(ack_buf) - 1);
    while(!acknowledged){
        tight_loop_contents();
    }
    acknowledged = false;
    // After acknowledgement from slave, send data
    uint8_t buf[msg_len];
    memcpy(buf, msg, msg_len);
#if DEBUG_I2C
    printf("Sending data: %s\n", msg);
#endif      
    // send_data(i2c_master_channel, buf, msg_len);
    i2c_write_blocking(i2c_master_channel, master_address, buf, msg_len, false);
    return 0;
}

char* i2c_recv() {
    bool finished_receiving = false;
    created_dynamic_location = false;
    int size_of_data = 0;
    while (!finished_receiving) {
        if (written) {
            // This will run when the master sends the slave the bytes for the size of the data that will be send over
            // Checking if dynamic location is being created already
            if (!created_dynamic_location) {
                received_len[data_index] = '\0';
                size_of_data = atoi(received_len);
#if DEBUG_I2C
                printf("Received length: %d\n", size_of_data);
#endif
                packet_data = (char*)malloc(sizeof(char) * (size_of_data));
                created_dynamic_location = true;
            } else {
#if DEBUG_I2C
                printf("Received data: %s", packet_data);
#endif          
                if (data_index >= size_of_data){
                    finished_receiving = true;
                    created_dynamic_location = false;
                }
            }
            written = false;
        }
    }
    return packet_data;
}

void init_i2c_pico_1() {
    I2C_MASTER_SDA_PIN = 4; 
    I2C_MASTER_SCL_PIN = 5;
    I2C_SLAVE_SDA_PIN = 6;
    I2C_SLAVE_SCL_PIN = 7;
    master_address = I2C_PICO_1_ADDRESS;
    slave_address = I2C_PICO_2_ADDRESS;
    setup_master(i2c1);
    setup_slave(i2c0, &slave_address);
}

void init_i2c_pico_2() {
    I2C_MASTER_SDA_PIN = 6;
    I2C_MASTER_SCL_PIN = 7;
    I2C_SLAVE_SDA_PIN = 4;
    I2C_SLAVE_SCL_PIN = 5;
    master_address = I2C_PICO_2_ADDRESS;
    slave_address = I2C_PICO_1_ADDRESS;
    setup_master(i2c0);
    setup_slave(i2c1, &slave_address);
}
