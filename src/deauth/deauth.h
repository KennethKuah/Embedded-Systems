#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "lwip/pbuf.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void set_payload(struct pbuf *p, const char *data, u16_t data_len);
    bool joinAP(char *ssid, char *password);
    void deauth();

#ifdef __cplusplus
}
#endif