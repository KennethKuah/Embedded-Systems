#ifndef SD_CARD_H
#define SD_CARD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include <pico/stdlib.h>
#include <pico/error.h>
//
#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

int init_sd();
int deinit_sd();

FIL* sd_fopen(const char* path, BYTE mode);
int sd_fclose(FIL* fil);

#endif