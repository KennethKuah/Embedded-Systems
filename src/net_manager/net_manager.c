#include "net_manager.h"

eth_addr_t my_mac;
eth_addr_t gateway_mac = ETH_ADDR(0xA6, 0x17, 0x1C, 0xEE, 0xB1, 0x6F);
eth_addr_t pico1_mac = ETH_ADDR(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
ip4_addr_t my_ip;
client_t *clients[5];
int client_idx = 0;

int setup_wifi()
{
    if (cyw43_arch_init()) {
        printf("[!] Failed to initialise\n");
    }
    cyw43_arch_enable_sta_mode();
    printf("[...] Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("[!] Failed to connect.\n");
        exit(1);
    }
    else {
        // Resolve necessary network info at setup-time
        my_ip = cyw43_state.netif[0].ip_addr;
        BYTE *hw_addr = cyw43_state.netif[0].hwaddr;
        for(int i = 0; i < cyw43_state.netif[0].hwaddr_len; ++i) {
            my_mac.addr[i] = hw_addr[i];
        }
        printf("[+] Connected with IP %s\n", ip4addr_ntoa(&my_ip));
    }
}

client_t *new_client(eth_addr_t client_mac, ip4_addr_t client_addr)
{
    if (client_idx + 1 > MAX_CLIENTS) {
        printf("[!] Max clients reached\n");
    }
    else {
        client_t *client = (client_t *)malloc(sizeof(client_t));

        client->client_mac = client_mac;
        client->client_addr = client_addr;
        client->conn_list = NULL;

        clients[client_idx] = client;

        printf("[+] Added new client %s\n", ip4addr_ntoa(&client->client_addr));

        client_idx += 1;
        return client;
    }

    return NULL;
}

client_t *search_client(ip4_addr_t client_addr)
{

    for (int i = 0; i <= client_idx; ++i) {
        if (ip4_addr_eq(&clients[i]->client_addr, &client_addr)) {
            return clients[i];
        }
    }

    return NULL;
}

conn_t *insert_new_conn(client_t *client, int src_port, ip4_addr_t dst_addr,
                        int dst_port)
{
    conn_t *new_conn = (conn_t *)malloc(sizeof(conn_t));

    new_conn->dst_addr = dst_addr;
    new_conn->src_port = src_port;
    new_conn->dst_port = dst_port;
    new_conn->next = client->conn_list;
    new_conn->prev = NULL;

    if (client->conn_list != NULL) {
        client->conn_list->prev = new_conn;
    }

    client->conn_list = new_conn;

    printf("[+] Added new connection %s:%d\n",
           ip4addr_ntoa(&client->conn_list->dst_addr),
           client->conn_list->dst_port);

    return new_conn;
}

void remove_conn(client_t *client, ip4_addr_t dst_addr, int port)
{
    conn_t *p_server_conn = client->conn_list;

    while (p_server_conn != NULL) {
        if ((ip4_addr_eq(&p_server_conn->dst_addr, &dst_addr)) &&
            p_server_conn->dst_port == port) {
            printf("[+] Removing connection %s:%d\n",
                   ip4addr_ntoa(&p_server_conn->dst_addr), port);
            // if deleting head
            if (p_server_conn == client->conn_list) {
                client->conn_list = client->conn_list->next;
            }
            else {
                p_server_conn->next->prev = p_server_conn->prev;
                p_server_conn->prev->next = p_server_conn->next;
            }
            free(p_server_conn);
            return;
        }
        p_server_conn = p_server_conn->next;
    }
}

conn_t *search_conn(client_t *client, ip4_addr_t dst_addr, int dst_port)
{
    conn_t *p_server_conn = client->conn_list;

    while (p_server_conn != NULL) {
        if ((ip4_addr_eq(&p_server_conn->dst_addr, &dst_addr)) &&
            p_server_conn->dst_port == dst_port) {
            return p_server_conn;
        }
        p_server_conn = p_server_conn->next;
    }

    return NULL;
}

void print_conns(client_t *client)
{
    conn_t *temp = client->conn_list;
    while (temp != NULL) {
        printf("%s:%d -> %s:%d\n", ip4addr_ntoa(&client->client_addr),
               temp->src_port, ip4addr_ntoa(&temp->dst_addr), temp->dst_port);
        temp = temp->next;
    }
}

void set_payload(struct pbuf *p, const char *data, u16_t data_len)
{
    if (p != NULL && data != NULL) {
        if (p->len >= data_len) {
            // Copy the data into the payload
            memcpy(p->payload, data, data_len);
        }
        else {
            printf("ERROR, INSUFFICIENT SIZE\n");
        }
    }
}

void test_send_ethernet()
{
    BYTE tcp_syn[98] = {
        0x5c, 0xe9, 0x1e, 0x77, 0xf4, 0xc4, 0xd8, 0x3a, 0xdd, 0x57, 0xb2,
        0xc8, 0x08, 0x00, 0x45, 0x00, 0x00, 0x54, 0xcb, 0xf3, 0x00, 0x00,
        0x40, 0x01, 0x2a, 0x8b, 0xc0, 0xa8, 0x01, 0xC2, 0xc0, 0xa8, 0x01,
        0x48, 0x08, 0x00, 0x6a, 0xd8, 0xd2, 0xea, 0x00, 0x00, 0x65, 0x66,
        0x89, 0x6d, 0x00, 0x08, 0xe0, 0x5d, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
        0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22,
        0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
        0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};

    struct pbuf *p = pbuf_alloc(PBUF_RAW, 98, PBUF_RAM);

    int d_err = 0;
    int success = 0;
    if (p != NULL) {
        set_payload(p, tcp_syn, sizeof(tcp_syn));

        for (int i = 0; i < 32; ++i) {
            d_err = cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, p->tot_len,
                                        (void *)p, true);

            if (!d_err) {
                success += 1;
            }
            printf("Successfully sent %d packets\n", success);
        }
    }
}

void extract_eth_ip(BYTE *pkt_buf, eth_hdr_t *eth_header, ip_hdr_t *ip_header)
{

    memcpy(eth_header, pkt_buf, sizeof(eth_hdr_t));
    memcpy(ip_header, pkt_buf + sizeof(eth_hdr_t), sizeof(ip_hdr_t));
}

void net_handler_rx()
{
    BYTE pkt[153] = {
        0xa6, 0x17, 0x1c, 0xee, 0xb1, 0x6f, 0x5c, 0xe9, 0x1e, 0x77, 0xf4, 0xc4,
        0x08, 0x00, 0x45, 0x02, 0x00, 0x8b, 0x00, 0x00, 0x40, 0x00, 0x40, 0x06,
        0x00, 0x83, 0xc0, 0xa8, 0x01, 0x92, 0xb9, 0x7d, 0xbe, 0x30, 0xd7, 0x2d,
        0x00, 0x50, 0x4f, 0x6a, 0xb8, 0x41, 0xe8, 0x84, 0xf6, 0x3a, 0x80, 0x18,
        0x08, 0x08, 0xbe, 0x08, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x6a, 0x03,
        0x12, 0x1c, 0x0b, 0x50, 0x6f, 0xc4, 0x47, 0x45, 0x54, 0x20, 0x2f, 0x20,
        0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, 0x0d, 0x0a, 0x48, 0x6f,
        0x73, 0x74, 0x3a, 0x20, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69,
        0x76, 0x69, 0x74, 0x79, 0x2d, 0x63, 0x68, 0x65, 0x63, 0x6b, 0x2e, 0x75,
        0x62, 0x75, 0x6e, 0x74, 0x75, 0x2e, 0x63, 0x6f, 0x6d, 0x0d, 0x0a, 0x41,
        0x63, 0x63, 0x65, 0x70, 0x74, 0x3a, 0x20, 0x2a, 0x2f, 0x2a, 0x0d, 0x0a,
        0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x3a, 0x20,
        0x63, 0x6c, 0x6f, 0x73, 0x65, 0x0d, 0x0a, 0x0d, 0x0a};

    eth_hdr_t ethernet_header;
    ip_hdr_t ip_header;
    extract_eth_ip(pkt, &ethernet_header, &ip_header);

    ip4_addr_t src_addr;
    ip4_addr_copy(src_addr, ip_header.src);

    ip4_addr_t dst_addr;
    ip4_addr_copy(dst_addr, ip_header.dest);

    int src_port;
    int dst_port;

    if (ip_header._proto == IP_PROTO_TCP) {
        tcp_hdr_t tcp_header;
        memcpy(&tcp_header, pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t),
               sizeof(tcp_hdr_t));

        src_port = htons(tcp_header.src);
        dst_port = htons(tcp_header.dest);
    }

    if (ip_header._proto == IP_PROTO_UDP) {
        udp_hdr_t udp_header;
        memcpy(&udp_header, pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t),
               sizeof(udp_hdr_t));

        src_port = htons(udp_header.src);
        dst_port = htons(udp_header.dest);
    }

    // Check if client exists
    client_t *client = search_client(src_addr);
    if (!client) {
        client = new_client(ethernet_header.src, src_addr);
    }
    else {
        printf("Found client %s\n", ip4addr_ntoa(&client->client_addr));
    }

    // Check if connection exists
    conn_t *server_conn = search_conn(client, dst_addr, dst_port);

    if (!server_conn) {
        server_conn = insert_new_conn(client, src_port, dst_addr, dst_port);
    }
    else {
        printf("Found connection %s:%d\n", ip4addr_ntoa(&server_conn->dst_addr),
               dst_port);
    }

    BYTE *forwarded_pkt = (BYTE *)malloc(sizeof(pkt));

    ip_addr_copy(ip_header.src, my_ip);
    ethernet_header.dest = gateway_mac;
    ethernet_header.src = my_mac;

    memcpy(forwarded_pkt, &ethernet_header, sizeof(eth_hdr_t));
    memcpy(forwarded_pkt + sizeof(eth_hdr_t), &ip_header, sizeof(ip_hdr_t));
    memcpy(forwarded_pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t),
           pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t),
           sizeof(pkt) - sizeof(eth_hdr_t) - sizeof(ip_hdr_t));

    for(int i = 0; i < sizeof(pkt); ++i) {
        printf("%02x ", forwarded_pkt[i]);
    }

    struct pbuf *packet_buf = pbuf_alloc(PBUF_RAW, sizeof(pkt), PBUF_RAM);

    set_payload(packet_buf, forwarded_pkt, sizeof(pkt));
    int d_err = cyw43_send_ethernet(&cyw43_state, CYW43_ITF_STA, packet_buf->tot_len, (void *)packet_buf, true);

    if(d_err == 0)
        printf("[+] Sent %d bytes\n", sizeof(pkt));
    
    pbuf_free(packet_buf);
    free(forwarded_pkt);

    accept_callback();
}

void accept_callback() {
    printf("Hello callback\n");
    
    BYTE pkt[153] = {
        0xa6, 0x17, 0x1c, 0xee, 0xb1, 0x6f, 0x5c, 0xe9, 0x1e, 0x77, 0xf4, 0xc4,
        0x08, 0x00, 0x45, 0x02, 0x00, 0x8b, 0x00, 0x00, 0x40, 0x00, 0x40, 0x06,
        0x00, 0x83, 0xc0, 0xa8, 0x01, 0x92, 0xb9, 0x7d, 0xbe, 0x30, 0xd7, 0x2d,
        0x00, 0x50, 0x4f, 0x6a, 0xb8, 0x41, 0xe8, 0x84, 0xf6, 0x3a, 0x80, 0x18,
        0x08, 0x08, 0xbe, 0x08, 0x00, 0x00, 0x01, 0x01, 0x08, 0x0a, 0x6a, 0x03,
        0x12, 0x1c, 0x0b, 0x50, 0x6f, 0xc4, 0x47, 0x45, 0x54, 0x20, 0x2f, 0x20,
        0x48, 0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, 0x0d, 0x0a, 0x48, 0x6f,
        0x73, 0x74, 0x3a, 0x20, 0x63, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69,
        0x76, 0x69, 0x74, 0x79, 0x2d, 0x63, 0x68, 0x65, 0x63, 0x6b, 0x2e, 0x75,
        0x62, 0x75, 0x6e, 0x74, 0x75, 0x2e, 0x63, 0x6f, 0x6d, 0x0d, 0x0a, 0x41,
        0x63, 0x63, 0x65, 0x70, 0x74, 0x3a, 0x20, 0x2a, 0x2f, 0x2a, 0x0d, 0x0a,
        0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x69, 0x6f, 0x6e, 0x3a, 0x20,
        0x63, 0x6c, 0x6f, 0x73, 0x65, 0x0d, 0x0a, 0x0d, 0x0a};

    // Send I2C here
    eth_hdr_t ethernet_header;
    ip_hdr_t ip_header;
    extract_eth_ip(pkt, &ethernet_header, &ip_header);

    ip4_addr_t src_addr;
    ip4_addr_copy(src_addr, ip_header.src);

    ip4_addr_t dst_addr;
    ip4_addr_copy(dst_addr, ip_header.dest);

    int src_port;
    int dst_port;

    if (ip_header._proto == IP_PROTO_TCP) {
        tcp_hdr_t tcp_header;
        memcpy(&tcp_header, pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t),
               sizeof(tcp_hdr_t));

        src_port = htons(tcp_header.src);
        dst_port = htons(tcp_header.dest);
    }

    if (ip_header._proto == IP_PROTO_UDP) {
        udp_hdr_t udp_header;
        memcpy(&udp_header, pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t),
               sizeof(udp_hdr_t));

        src_port = htons(udp_header.src);
        dst_port = htons(udp_header.dest);
    }

    // Client must already exist, otherwise don't forward
    client_t *client = search_client(dst_addr);
    if (!client) {
        // If no client, don't forward to I2C
        return;
    }
    else {
        printf("Found client %s\n", ip4addr_ntoa(&client->client_addr));
    }

    // Connection must already exist, otherwise no mapping
    conn_t *server_conn = search_conn(client, src_addr, src_port);

    if (!server_conn) {
        // If no connection, don't forward to I2C
        return;
    }

    else {
        printf("Found connection %s:%d\n", ip4addr_ntoa(&server_conn->dst_addr),
               dst_port);
    }

    BYTE *forwarded_pkt = (BYTE *)malloc(sizeof(pkt));

    ip_addr_copy(ip_header.src, my_ip);
    ethernet_header.dest = client->client_mac;
    ethernet_header.src = pico1_mac;

    memcpy(forwarded_pkt, &ethernet_header, sizeof(eth_hdr_t));
    memcpy(forwarded_pkt + sizeof(eth_hdr_t), &ip_header, sizeof(ip_hdr_t));
    memcpy(forwarded_pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t),
           pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t),
           sizeof(pkt) - sizeof(eth_hdr_t) - sizeof(ip_hdr_t));

    for(int i = 0; i < sizeof(pkt); ++i) {
        printf("%02x ", forwarded_pkt[i]);
    }

    free(forwarded_pkt);

    // send_i2c here
}
