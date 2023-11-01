#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"


int main() {
    stdio_init_all();
    
    char recvbuf[64];
    char recvchar;
    int index = 0;

    while(1)
    {
        index = 0;
        memset(recvbuf, 0, 64);
        while(1)
        {
            recvchar = getchar();
            if (recvchar == '\n')
            {
                recvbuf[index] = '\0';
                break;
            }

            recvbuf[index] = recvchar;
            index++;
        }
        printf("Received: %s", recvbuf);
    }
    return 0;
}
