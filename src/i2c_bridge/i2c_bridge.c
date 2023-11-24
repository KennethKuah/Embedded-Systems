#include "i2c_bridge.h"

// Serialises data that is transmitted via I2C.
// Buffer is dynamically allocated, meaning it must be freed after its used
// Returns a string in the following format
// `dst_ip:port:protocol:base64_data`
// \param dst_ip destination IP string
// \param port source port of connection
// \param proto protocol string
// \param data data to be encoded in base64
// \param data_len length of data to be encoded
char *i2c_serialize(char *dst_ip, int port, char *proto, BYTE *data,
                    int data_len) {
    char data_encoded[MAX_MESSAGE_SIZE - 256];
    const char *delimiter = DELIMITER;
    int encoded_len = base64_encode(data_encoded, data, data_len);
    char *buf = (char *)calloc(MAX_MESSAGE_SIZE, sizeof(char));
    char port_str[10];
    sprintf(port_str, "%d", port);
    strcat(buf, dst_ip);
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
// `dst_ip:port:protocol:base64_data`
// \param buf serialized data received from I2C channel
I2CData *i2c_deserialize(char *buf) {
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
