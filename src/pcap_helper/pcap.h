#ifndef PCAP_H
#define PCAP_H
#include "sd_card.h"
#include "lwip/pbuf.h"

#define PCAP_VERSION_MAJOR 2
#define PCAP_VERSION_MINOR 4
#define DLT_EN10MB 1

//time header
struct pcap_timeval {
    int32_t tv_sec;
    int32_t tv_usec;
};

//pcap Global header
struct pcap_file_header {
    uint32_t magic;         //used to detect the file format itself and the byte ordering.
    uint16_t version_major; //version number
    uint16_t version_minor;
    int32_t thiszone; //GMT timezone offset
    uint32_t sigfigs;
    uint32_t snaplen;  //"snapshot length" for the capture (typically 65535 or even more, but might be limited by the user)
    uint32_t linktype; //link-layer header type, specifying the type of headers at the beginning of the packet.
};

//pcap packet header
struct pcap_sf_pkthdr {
    struct pcap_timeval ts; //he date and time when this packet was captured.
    uint32_t caplen;        //the number of bytes of packet data actually captured and saved in the file
    uint32_t len;           //the length of the packet as it appeared on the network when it was captured.
};

#define BUFFER_SIZE_HDR sizeof(struct pcap_sf_pkthdr)
#define BUFFER_SIZE_ETH 14
#define BUFFER_SIZE_PKT ((256 * 256) - 1)

int set_pcap_file(const char *const path);
void write_packet(struct pbuf *p);


#endif