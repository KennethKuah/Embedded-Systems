#pragma once

#include "f_util.h"
#include "ff.h"
#include "hw_config.h"

#define BUFFER_SIZE 256

#ifdef __cplusplus
extern "C" {
#endif

sd_card_t* init_sd();
int sd_cleanup(sd_card_t* pSD);

FIL* sd_fopen(const char* path, BYTE mode);
int sd_fclose(FIL* fil);

#ifdef __cplusplus
}
#endif
