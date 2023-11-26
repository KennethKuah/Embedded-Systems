#include "i2c.h"

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
        finishedReceiving = true;
        break;
    default:
        break;
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

void setup_slave(i2c_inst_t *i2c_channel) {
    i2c_slave_channel = i2c_channel;
    gpio_init(I2C_SLAVE_SDA_PIN);
    gpio_set_function(I2C_SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SDA_PIN);

    gpio_init(I2C_SLAVE_SCL_PIN);
    gpio_set_function(I2C_SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SLAVE_SCL_PIN);

    i2c_init(i2c_channel, I2C_BAUDRATE);
    i2c_slave_init(i2c_channel, I2C_CLIENT_ADDRESS, &i2c_slave_handler);
}

int send_slave(char * msg) {
    int total_bytes_written = 0;
    uint8_t pending_msg_len = strlen(msg);
    uint8_t mem_address = 0;

    while (pending_msg_len > 0) {
        uint8_t buf[MAX_BUF_LEN];
        buf[0] = mem_address;

        int bytes_written = 0;
        if (pending_msg_len > (MAX_BUF_LEN - 1)){
            memcpy(buf + 1, msg, MAX_BUF_LEN - 1);
            msg += MAX_BUF_LEN - 1;
            bytes_written = i2c_write_blocking(i2c_master_channel, I2C_SLAVE_ADDRESS, buf, MAX_BUF_LEN, false);
        } else { 
            memcpy(buf + 1, msg, pending_msg_len);
            msg += pending_msg_len;
            bytes_written = i2c_write_blocking(i2c_master_channel, I2C_SLAVE_ADDRESS, buf, pending_msg_len + 1, false);
        }
        if (bytes_written < 0) {
            puts("Couldn't write to slave, please check your wiring!");
        }
        mem_address = (mem_address + 32) % 256;
        pending_msg_len = strlen(msg);
        total_bytes_written += bytes_written;
    }
    return total_bytes_written;
}

char* recv_from_master() {
    while (!finishedReceiving)
        sleep_ms(100);
    
    finishedReceiving = false;
    return received_data;
}

void init_pico_1() {
    setup_master(i2c1);
    setup_slave(i2c0);
}

void init_pico_2() {
    setup_master(i2c0);
    setup_slave(i2c1);
}

// Serialises data that is transmitted via I2C
// Returns a string in the following format
// dst_ip:port:protocol:base64_data
// Buffer is dynamically allocated, meaning it must be freed after its used
//
char *i2c_serialize(char *dst, int port, char *proto, BYTE *data,
                    int data_len)
{
    char data_encoded[MAX_MESSAGE_SIZE - 256];
    const char *delimiter = DELIMITER;
    int encoded_len = base64_encode(data_encoded, data, data_len);
    char *buf = (char *)calloc(MAX_MESSAGE_SIZE, sizeof(char));
    char port_str[10];
    sprintf(port_str, "%d", port);
    strcat(buf, dst);
    strcat(buf, delimiter);
    strcat(buf, port_str);
    strcat(buf, delimiter);
    strcat(buf, proto);
    strcat(buf, delimiter);
    strcat(buf, data_encoded);

    return buf;
}

// Deserializes data that is transmitted via I2C
// Expects a string in the following format
// dst_ip:port:protocol:base64_data
I2CData *i2c_deserialize(char *buf)
{
    int idx = 0;
    int oset = 0;
    char tokens[5][MAX_MESSAGE_SIZE / 5];
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

    BYTE data_decoded[MAX_MESSAGE_SIZE - 256];
    char data_encoded[MAX_MESSAGE_SIZE - 256];
    strcpy(data_encoded, tokens[4]);

    int decoded_len =
        base64_decode(data_decoded, data_encoded, strlen(data_encoded));

    I2CData *i2c_data = (I2CData *)malloc(sizeof(I2CData));

    if(i2c_data != NULL) {
        i2c_data->dst_ip = tokens[0];
        char *end_ptr;
        i2c_data->port = strtol(tokens[1], &end_ptr, 10);
        i2c_data->proto = tokens[3];
        i2c_data->data = data_decoded;
        i2c_data->data_len = decoded_len;
    }

    return i2c_data;
}
