#pragma once

#include <stdio.h>
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C"
{
#endif

    void generate_random_mac(uint8_t mac[6]);
    void generate_transaction_id(uint8_t tid[4]);
    void send_dhcp_discover();

    typedef struct
    {
        uint8_t dest_mac[6];
        uint8_t src_mac[6];
        uint16_t eth_type;
    } eth_header;

    typedef struct
    {
        uint8_t version;
        uint8_t diff_services_field;
        uint16_t tot_len;
        uint16_t id;
        uint16_t frag_offset;
        uint8_t ttl;
        uint8_t prot;
        uint16_t head_checksum;
        uint8_t src_ip[4];
        uint8_t dst_ip[4];
    } ip_header;

    typedef struct
    {
        uint16_t src_port;
        uint16_t dst_port;
        uint16_t len;
        uint16_t checksum;
    } udp_header;

    typedef struct
    {
        uint8_t msg_type;
        uint8_t hardware_type;
        uint8_t hardware_addr_len;
        uint8_t hops;
        uint8_t tx_id[4];
        uint16_t secs_elapsed;
        uint16_t bootp_flags;
        uint8_t client_ip[4];
        uint8_t your_ip[4];
        uint8_t server_ip[4];
        uint8_t relay_ip[4];
        uint8_t client_mac[6];
        uint8_t client_mac_padding[10];
        uint8_t server_host_name[64];
        uint8_t boot_file_name[128];
        uint8_t magic_cookie[4];
        uint8_t dhcp_msg_type[3];
        uint8_t max_dhcp_msg_size[4];
        uint8_t param_req_list[6];
        uint8_t end;
        uint8_t padding[54];
    } dhcp_discover_packet;

    typedef struct
    {
        eth_header eth;
        ip_header ip;
        udp_header udp;
        dhcp_discover_packet dhcp;
    } full_dhcp_packet;

#ifdef __cplusplus
}
#endif