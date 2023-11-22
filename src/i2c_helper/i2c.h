#ifndef I2C_HEADER_H
#define I2C_HEADER_H
#include <hardware/i2c.h>
#include <pico/i2c_slave.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_BUF_LEN 2048

static const uint I2C_SLAVE_ADDRESS = 0x04;
static const uint I2C_CLIENT_ADDRESS = 0x17;
static const uint I2C_BAUDRATE = 100000; // 100 kHz

static const uint I2C_SLAVE_SDA_PIN = PICO_DEFAULT_I2C_SDA_PIN; // 4
static const uint I2C_SLAVE_SCL_PIN = PICO_DEFAULT_I2C_SCL_PIN; // 5
static const uint I2C_MASTER_SDA_PIN = 6;
static const uint I2C_MASTER_SCL_PIN = 7;
static char received_data[MAX_BUF_LEN];
static i2c_inst_t *i2c_master_channel;
static i2c_inst_t *i2c_slave_channel;
static bool finishedReceiving = false;

// The slave implements a 256 byte memory. To write a series of bytes, the master first
// writes the memory address, followed by the data. The address is automatically incremented
// for each byte transferred, looping back to 0 upon reaching the end. Reading is done
// sequentially from the current memory address.
static struct
{
    uint8_t mem[256];
    uint8_t mem_address;
    bool mem_address_written;
} context;

int master(char * packet_data);
char * recv_from_master();
#endif