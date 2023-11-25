#include "pcap.h"

char* fp;

// Sets the filepath to write to on the SD card
// \param path absolute filepath string
// \return 0 if the initialization is successful, an error code otherwise \see pico_error_codes
int set_pcap_file(const char *const path) {
    if (init_sd()){
        printf("Failed to initialize SD card.\n");
        return PICO_ERROR_GENERIC;
    }

    // Opens the file on the sd card
    fp = (char *)path;
    FIL* file = sd_fopen(fp, FA_CREATE_NEW | FA_WRITE);
    if (!file){
        printf("Failed to create pcap file. Check if it exists on SD card.\n");
        return PICO_ERROR_NOT_PERMITTED;
    }

    // PCAP file headers format
    struct pcap_file_header hdr;
    hdr.magic = 0xa1b2c3d4;
    hdr.version_major = PCAP_VERSION_MAJOR;
    hdr.version_minor = PCAP_VERSION_MINOR;
    hdr.thiszone = 0;
    hdr.snaplen = BUFFER_SIZE_PKT;
    hdr.sigfigs = 0;
    hdr.linktype = DLT_EN10MB;

    // Write PCAP headers to file and close to save the file
    TCHAR header[sizeof(struct pcap_file_header)];
    memcpy(header, &hdr, sizeof(hdr));
    f_write(file, (const TCHAR*)header, sizeof(header), NULL);
    sd_fclose(file);

    return PICO_OK;
}

// Writes packet data to pcap file
// \param data packet bytes to write
// \param len packet bytes length
void write_packet(uint8_t *data, int len) {
    // Open the file in append mode
    FIL* file = sd_fopen(fp, FA_OPEN_APPEND | FA_WRITE);

    // Timestamp for this packet.
    uint64_t timestamp_us = time_us_64();
    uint32_t seconds = (uint32_t) (timestamp_us / 1000000);
    uint32_t microseconds = (uint32_t) (timestamp_us % 1000000);
    
    // Set out PCAP packet header fields.
    struct pcap_sf_pkthdr pkthdr;
    pkthdr.ts.tv_sec = (int32_t) seconds;
    pkthdr.ts.tv_usec = (int32_t) microseconds;
    pkthdr.caplen = len;
    pkthdr.len = len;

    // Write the PCAP packet header fields to file followed by data packet bytes
    TCHAR header[BUFFER_SIZE_HDR];
    memcpy(header, &pkthdr, BUFFER_SIZE_HDR);
    f_write(file, (const TCHAR*)header, BUFFER_SIZE_HDR, NULL);
    f_write(file, data, len, NULL);

    // Close the file handle to save output of file, if abruptly disconnected. 
    sd_fclose(file);
}