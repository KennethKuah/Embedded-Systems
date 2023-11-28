#ifndef I2C_BRIDGE_H
#define I2C_BRIDGE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include <pico/stdlib.h>
#include <pico/i2c_slave.h>
#include <hardware/i2c.h>
//
#include "b64.h"

#define DEBUG_I2C 1
// Serialization definitions
#define MAX_BUF_LEN 256
#define MAX_MESSAGE_SIZE 15000
#define DELIMITER ":"
// I2C definitions
#define I2C_MAX_BUF_LEN 512
#define I2C_PICO_1_ADDRESS 0x04
#define I2C_PICO_2_ADDRESS 0x17
#define I2C_BAUDRATE 100000 // 100 kHz

typedef unsigned char BYTE;

typedef struct I2CData {
    char *dst_ip;
    int port;
    BYTE *proto;
    BYTE *data;
    int data_len;
} i2c_data_t;

char *i2c_serialize(char *dst_ip, int port, BYTE *proto, BYTE *data,
                    int data_len);
i2c_data_t *i2c_deserialize(char *buf);
void init_i2c_pico_1();
void init_i2c_pico_2();
int i2c_send(char* msg);
void wait_for_data();
char* i2c_recv();

#endif
