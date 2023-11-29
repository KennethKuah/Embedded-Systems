#include "net_manager.h"

eth_addr_t my_mac;
eth_addr_t gateway_mac = ETH_ADDR(0x5C, 0xA6, 0xE6, 0xFB, 0x24, 0x75);
eth_addr_t pico1_mac = ETH_ADDR(0xD8, 0x3A, 0xDD, 0x45, 0xC8, 0xB7);
ip4_addr_t my_ip;
client_t *clients[5];
int client_idx = 0;

int local_socks[MAX_CONCURRENT_CONNS];
int local_socks_idx = 0;

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
        for (int i = 0; i < cyw43_state.netif[0].hwaddr_len; ++i) {
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

void poll_sockets() {

    for(int i = 0; i < local_socks_idx; ++i) {
        char recv_buffer[MAX_RECV_BUFFER];

        memset(recv_buffer, 0, sizeof(recv_buffer));
        int recv_bytes = recv(local_socks[i], recv_buffer, sizeof(recv_buffer), 0);

        if(recv_bytes) {
            // i2c_send here
        }
    }
}

void net_handler_rx(BYTE *pkt, int pkt_len)
{
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

    ip_addr_t filter_ip;
    ipaddr_aton("34.223.124.45", &filter_ip);

    if (!ip4_addr_eq(&filter_ip, &dst_addr))
        return;

    // // Check if client exists
    // client_t *client = search_client(src_addr);
    // printf("Searched client\n");
    // if (!client) {
    //     client = new_client(ethernet_header.src, src_addr);
    // }
    // else {
    //     printf("[+] Found client %s\n", ip4addr_ntoa(&client->client_addr));
    // }

    // // Check if connection exists
    // conn_t *server_conn = search_conn(client, dst_addr, dst_port);
    // printf("Searched conn\n");
    // if (!server_conn) {
    //     server_conn = insert_new_conn(client, src_port, dst_addr, dst_port);
    // }
    // else {
    //     printf("[+] Found connection %s:%d\n",
    //            ip4addr_ntoa(&server_conn->dst_addr), dst_port);
    // }

    printf("Reached net_handler\n");

    for(int i = 0; i < pkt_len; ++i) {
        printf("%02x ", pkt[i]);
    }
    printf("\n");
    int data_len = pkt_len - sizeof(eth_hdr_t) - sizeof(ip_hdr_t);
    printf("Length: %d\n", data_len);

    printf("Allocated for data\n");

    ip_addr_copy(ip_header.src, my_ip);
    ethernet_header.dest = gateway_mac;
    ethernet_header.src = my_mac;

    for (int i = 0; i < data_len; ++i) {
        printf("%02x ", (pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t))[i]);
    }
    printf("\n");
    int server_sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

    BYTE *data = (BYTE *)malloc(pkt_len);

    if (server_sock < 0) {
        printf("[!] Failed to create socket\n");
    }
    memcpy(data, pkt + sizeof(eth_hdr_t) + sizeof(ip_hdr_t), data_len);
    printf("[+] Socket created\n");
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(0);
    inet_aton(ip4addr_ntoa(&dst_addr), &server_addr.sin_addr);

    int sent_bytes =
        sendto(server_sock, data, data_len, 0, (struct sockaddr *)&server_addr,
               sizeof(server_addr));
    if (sent_bytes) {
        printf("[+] Successfully sent %d bytes\n", sent_bytes);
    }
    else {
        printf("[!] Failed to send\n");
    }
    free(data);
    close(server_sock);

    int client_sock = socket(AF_INET, SOCK_RAW, IP_PROTO_TCP);

    if (client_sock < 0) {
        printf("[!] Failed to create client socket\n");
    }

    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(src_port);
    client_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    if(bind(client_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0) {
        printf("[!] Failed to bind client socket!\n");
    }

    printf("Socket bound\n");

    char recv_buffer[MAX_RECV_BUFFER];

    int recv_bytes = 0;
    while(true) {
        recv_bytes = recv(client_sock, recv_buffer, MAX_RECV_BUFFER, 0);
        if(recv_bytes)
            break;
    }

    printf("[+] Received %d\n bytes", recv_bytes);

    char *serialized_data = i2c_serialize(ip4addr_ntoa(&src_addr), recv_buffer + sizeof(ip_hdr_t), recv_bytes - sizeof(ip_hdr_t));
    i2c_send(serialized_data);
    free(serialized_data);
    
    close(client_sock);
}
