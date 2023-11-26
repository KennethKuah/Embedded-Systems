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

// Serialization definitions
#define MAX_BUF_LEN 256
#define MAX_MESSAGE_SIZE 30000
#define DELIMITER ":"
// I2C definitions
#define I2C_MAX_BUF_LEN 512
#define I2C_PICO_1_ADDRESS 0x04
#define I2C_PICO_2_ADDRESS 0x17
#define I2C_BAUDRATE 100000 // 100 kHz
#define I2C_SLAVE_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN // 4
#define I2C_SLAVE_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN // 5
#define I2C_MASTER_SDA_PIN 6
#define I2C_MASTER_SCL_PIN 7

typedef unsigned char BYTE;

typedef struct {
    char *dst_ip;
    int port;
    BYTE *proto;
    BYTE *data;
    int data_len;
} I2CData;

char *i2c_serialize(char *dst_ip, int port, BYTE *proto, BYTE *data,
                    int data_len);
I2CData *i2c_deserialize(char *buf);
void init_i2c_pico_1();
void init_i2c_pico_2();
int send_i2c(char* msg);
void wait_for_data();
char* recv_i2c();

#endif
