#include "net_manager.h"

server_conn_t *conn_list = NULL;

int setup_wifi()
{
    if (cyw43_arch_init()) {
        printf("Failed to initialise\n");
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("Failed to connect.\n");
        exit(1);
    }
    else {
        printf("Connected.\n");
    }
}

void insert_new_conn(int server_sock, char *dst_ip, int port)
{
    server_conn_t *new_conn = malloc(sizeof(server_conn_t));

    new_conn->server_sock = server_sock;
    new_conn->dst_ip = dst_ip;
    new_conn->port = port;
    new_conn->next = conn_list;
    new_conn->prev = NULL;

    if (conn_list != NULL) {
        conn_list->prev = new_conn;
    }

    conn_list = new_conn;
}

void remove_conn(char *dst_ip, int port)
{
    server_conn_t *p_server_conn = conn_list;

    while (p_server_conn != NULL) {
        if ((strcmp(p_server_conn->dst_ip, dst_ip) == 0) &&
            p_server_conn->port == port) {
            printf("[+] Removing connection %s:%d\n", dst_ip, port);
            // if deleting head
            if (p_server_conn == conn_list) {
                conn_list = conn_list->next;
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

int search_conn(char *dst_ip, int port)
{
    server_conn_t *p_server_conn = conn_list;

    while (p_server_conn != NULL) {
        if ((strcmp(p_server_conn->dst_ip, dst_ip) == 0) &&
            p_server_conn->port == port) {
            return p_server_conn->server_sock;
        }
    }

    return -1;
}

void print_conns()
{
    server_conn_t *temp = conn_list;
    while (temp != NULL) {
        printf("%s:%d\n", temp->dst_ip, temp->port);
        temp = temp->next;
    }
}

void new_tcp_tunnel(char *server_ip, int port)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server_sock < 0) {
        printf("Unable to create socket\n");
    }

    struct sockaddr_in server_addr;
    unsigned long server_ip_numeric = inet_addr(server_ip);

    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = server_ip_numeric;

    if (connect(server_sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        printf("Failed to connect\n");
    }
    else {
        insert_new_conn(server_sock, server_ip, port);
        printf("[+] Adding connection %s:%d\n", server_ip, port);
    }
}

void send_tunnel(int server_sock, BYTE *buf, int buf_len)
{
    int bytes_sent = send(server_sock, buf, buf_len, 0);

    if (bytes_sent != buf_len) {
        printf("Send failed!\n");
    }
    else {
        printf("Sent %d bytes\n", bytes_sent);
    }
}

void test_conns()
{
    char request[1024];
    sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
            "34.223.124.45");

    new_tcp_tunnel("34.223.124.45", 80);

    int server_sock = search_conn("34.223.124.45", 80);

    if (server_sock != -1) {
        send_tunnel(server_sock, request, strlen(request));
    }

    BYTE test[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    char *data = i2c_serialize("testing.com", 8000, "TCP", test, sizeof(test));

    free(data);

    printf("Data: %s\n", data);

    i2c_data_t *i2c_data = i2c_deserialize("testing.com:8000:TCP:dGVzdA==:4");

    printf("%s\n", i2c_data->dst_ip);
    printf("%d\n", i2c_data->port);

    for(int i = 0; i < i2c_data->data_len; ++i) {
        printf("%x ", i2c_data->data[i]);
    }

    char *data2 = i2c_serialize("testing2.com", 8080, "UDP", test, sizeof(test));
    printf("%s\n", data2);
    free(data2);
}

int send_dns_req(BYTE *data, int data_len, BYTE *out_data)
{

    int server_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

    if (server_sock < 0) {
        printf("Failed to create socket!\n");
    }
    printf("socket created");

    struct sockaddr_in server_addr;
    unsigned long server_ip_numeric = inet_addr(DNS_IP);

    memset(&server_addr, '0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DNS_PORT);
    server_addr.sin_addr.s_addr = server_ip_numeric;

    if (connect(server_sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        printf("Failed to connect\n");
    }

    int bytes_sent = sendto(server_sock, data, data_len, MSG_WAITALL,
           (const struct sockaddr *)&server_addr, sizeof(server_addr));

    if(bytes_sent != data_len) {
        printf("Send failed\n");
    } else {
        printf("Sent %d bytes\n", bytes_sent);
    }

    socklen_t fromlen = sizeof(server_addr);
    int bytes_recv = recvfrom(server_sock, out_data, 512, MSG_WAITALL, (struct sockaddr *)&server_addr, &fromlen);

    close(server_sock);

    return bytes_recv;
}

void test_dns()
{
    BYTE data[58] = {0xba,0x87,0x01,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x12,0x63,0x6f,0x6e,0x6e,0x65,0x63,0x74,0x69,0x76,0x69,0x74,0x79,0x2d,0x63,0x68,0x65,0x63,0x6b,0x06,0x75,0x62,0x75,0x6e,0x74,0x75,0x03,0x63,0x6f,0x6d,0x00,0x00,0x01,0x00,0x01,0x00,0x00,0x29,0x05,0xc0,0x00,0x00,0x00,0x00,0x00,0x00};
    BYTE out_data[512];
    int bytes_recv = send_dns_req(data, 58, out_data);

    if(!bytes_recv) {
        printf("Query error\n");
    } else {
        printf("Received %d bytes\n", bytes_recv);

        for(int i = 0; i < bytes_recv; ++i) {
            printf("%x ", out_data[i]);
        }
    }
}