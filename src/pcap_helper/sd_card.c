#include "sd_card.h"

sd_card_t* pSD;

// Initializes the SD card that is mounted to the Maker Pi Pico Board
// \return 0 if the initialization is successful, an error code otherwise
int init_sd() {
    pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (FR_OK != fr) {
        printf("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
        return PICO_ERROR_GENERIC;
    }
    return PICO_OK;
}

// Unmounts the SD card on the Maker Pi Pico Board
int deinit_sd(){
    f_unmount(pSD->pcName);
    return PICO_OK;
}

// Opens the file on the SD card
// \param path absolute filepath to file
// \param mode File access mode
// \return File handle if open is successful, NULL otherwise
FIL* sd_fopen(const char *const path, BYTE mode) {
    FIL* fil = (FIL*)malloc(sizeof(FIL));
    
    FRESULT fr = f_open(fil, path, mode);
#if PCAP_OVERWRITE
    if (FR_OK != fr && FR_EXIST != fr) {
#else
    if (FR_OK != fr) {
#endif
        printf("f_open(%s) error: %s (%d)\n", path, FRESULT_str(fr), fr);
        return NULL;
    }
    return fil;
}

// Closes the file handle
// \return 0 if file close is successful, an error code otherwise
int sd_fclose(FIL* fil) {
    FRESULT fr = f_close(fil);
    if (fil)
        free(fil);
    if (FR_OK != fr) {
        printf("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
        return PICO_ERROR_GENERIC;
    }
    return PICO_OK;
}
