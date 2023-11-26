#pragma once

#include <stdio.h>
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C"
{
#endif

    bool joinAP(char *ssid, char *password);
    void deauth();

#ifdef __cplusplus
}
#endif