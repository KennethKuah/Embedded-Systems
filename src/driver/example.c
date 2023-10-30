#include <stdio.h>
#include <string.h>
//
#include "pico/stdlib.h"
//
#include "driver/sd_card.h"
#include "ff.h"

const char *const filepath = "filename.txt";

int write_example(){
    char buf[BUFFER_SIZE] = {0};
    int bufPtr = 0;

    FIL *file = sd_fopen(filepath, FA_OPEN_APPEND | FA_WRITE);

    printf("Enter string to write to the file (type '\\q' to quit): ");
    while (true) {
        char c = getchar();
        buf[bufPtr] = c;
        bufPtr++;

        if (strncmp(buf, "\\q", 2) == 0){
            break;
        }

        if (bufPtr == BUFFER_SIZE - 1 || c == '\n'){
            buf[bufPtr] = '\0'; 
            printf("Writing the following string to file:\n%s", buf);
            f_printf(file, buf);
            bufPtr = 0;
        }
    }
    printf("Quiting... closing file...\n");
    sd_fclose(file);
    return 0;
}

int read_example(){
    char buf[BUFFER_SIZE] = {0};
    int bytesRead = 0;

    FIL *file = sd_fopen(filepath, FA_OPEN_EXISTING | FA_READ);

    printf("File contents:\n");
    // returns a non-zero value if the read/write 
    // pointer has reached end of the file
    while (f_eof(file) == 0){
        // read operation continues until a '\n' or 
        // buffer is filled with len - 1 characters
        printf("%s", f_gets(buf, sizeof(buf), file));
    }

    sd_fclose(file);
    return 0;
}

void printhelp(){
    printf("Commands:\n");
    printf("w\t: write to %s\n", filepath);
    printf("r\t: read from %s\n", filepath);
    printf("q\t: quit and unmount sd card");
}

int main() {
    // Before running, change serial line ending to CRLF mode
    stdio_usb_init();
    sd_card_t *sd_card = init_sd();
    printf("SD Card Initialized...\n");
    bool exit = false;

    printhelp();
    while (!exit) {
        char c = getchar();
        switch (c) {
            case 'w':
                write_example();
                break;
            case 'r':
                read_example();
                break;
            case 'h':
                printhelp();
                break;
            case 'q':
                exit = true;
                break;
            case '\n':
            case '\r':
                break;
            default:
                printf("\nUnrecognised command: %c\n", c);
                printhelp();
                break;
        }
    }

    printf("Quiting... Dismounting SD Card...\n");
    sd_cleanup(sd_card);
    
    for (;;); // do nothing
}
