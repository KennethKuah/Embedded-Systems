#ifndef I2C_BRIDGE_H
#define I2C_BRIDGE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include "b64.h"

#define MAX_BUF_LEN 256
#define MAX_MESSAGE_SIZE 30000
#define DELIMITER ":"

typedef unsigned char BYTE;

typedef struct {
    char *dst_ip;
    int port;
    char *proto;
    BYTE *data;
    int data_len;
} I2CData;

char *i2c_serialize(char *dst_ip, int port, char *proto, BYTE *data,
                    int data_len);
I2CData *i2c_deserialize(char *buf);

#endif
