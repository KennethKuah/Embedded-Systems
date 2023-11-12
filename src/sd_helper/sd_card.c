#include <stdlib.h>
//
#include "sd_card.h"

sd_card_t* init_sd(){
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (FR_OK != fr)
        panic("f_mount error: %s (%d)\n", FRESULT_str(fr), fr);
    return pSD;
}

int sd_cleanup(sd_card_t *pSD){
    f_unmount(pSD->pcName);
    return 0;
}

FIL* sd_fopen(const char *const path, BYTE mode){
    FIL* fil = (FIL*)malloc(sizeof(FIL));
    
    FRESULT fr = f_open(fil, path, mode);
    if (FR_OK != fr && FR_EXIST != fr) {
        panic("f_open(%s) error: %s (%d)\n", path, FRESULT_str(fr), fr);
    }
    return fil;
}

int sd_fclose(FIL* fil){
    FRESULT fr = f_close(fil);
    if (fil)
        free(fil);
    if (FR_OK != fr) {
        panic("f_close error: %s (%d)\n", FRESULT_str(fr), fr);
    }
    return 0;
}
