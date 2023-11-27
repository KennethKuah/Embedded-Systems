#include "wifi.h"

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

int setup_ap()
{
    if (cyw43_arch_init()) {
        printf("Failed to initialise\n");
    }

    cyw43_arch_enable_ap_mode(AP_SSID, AP_PW, CYW43_AUTH_WPA2_AES_PSK);

    ip_addr_t gateway;
    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&gateway), 192, 168, 9, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &gateway, &mask);

    // Start the dns server
    dns_server_t dns_server;
    dns_server_init(&dns_server, &gateway);
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

// Serialises data that is transmitted via I2C
// Returns a string in the following format
// dst_ip:port:protocol:base64_data:decoded_data_len
// Buffer is dynamically allocated, meaning it must be freed after its used
//
char *i2c_serialize(char *dst, int port, char *proto, BYTE *data, int data_len)
{
    char data_encoded[MAX_MESSAGE_SIZE - 256];
    const char *delimiter = ":";
    int encoded_len = base64_encode(data_encoded, data, data_len);
    char *buf = (char *)calloc(MAX_MESSAGE_SIZE, sizeof(char));
    char port_str[10];
    sprintf(port_str, "%d", port);
    char data_len_str[10];
    sprintf(data_len_str, "%d", data_len);
    strcat(buf, dst);
    strcat(buf, delimiter);
    strcat(buf, port_str);
    strcat(buf, delimiter);
    strcat(buf, proto);
    strcat(buf, delimiter);
    strcat(buf, data_encoded);
    strcat(buf, delimiter);
    strcat(buf, data_len_str);

    return buf;
}

// Deserializes data that is transmitted via I2C
// Expects a string in the following format
// dst_ip:port:protocol:base64_data:decoded_data_len
i2c_data_t *i2c_deserialize(char *buf)
{
    int idx = 0;
    int oset = 0;
    char tokens[5][MAX_MESSAGE_SIZE / 4];
    for (int i = 0; i <= strlen(buf); ++i) {
        if (buf[i] == ':') {
            // null terminate token
            tokens[idx][oset] = '\x00';
            idx++;
            oset = 0;
        }
        else {
            tokens[idx][oset++] = buf[i];
        }
    }

    char data_decoded[MAX_MESSAGE_SIZE - 256];
    char data_encoded[MAX_MESSAGE_SIZE - 256];

    strcpy(data_encoded, tokens[3]);
    int decoded_len =
        base64_decode(data_decoded, data_encoded, strlen(data_encoded));

    i2c_data_t *i2c_data = (i2c_data_t *)malloc(sizeof(i2c_data_t));

    if (i2c_data != NULL) {
        i2c_data->dst_ip = tokens[0];
        char *end_ptr;
        i2c_data->port = strtol(tokens[1], &end_ptr, 10);
        i2c_data->proto = tokens[3];
        i2c_data->data = data_decoded;
        i2c_data->data_len = decoded_len;
    }

    return i2c_data;
}

void test_conns()
{

    char request[1024];

    sprintf(request, "GET / HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n",
            "34.223.124.45");

    char *data =
        i2c_serialize("34.22.124.45", 80, "TCP", request, strlen(request));
    printf("[+] Serialised Data: %s\n", data);
    free(data);

    i2c_data_t *i2c_data =
        i2c_deserialize("34.22.124.45:80:TCP:"
                        "R0VUIC8gSFRUUC8xLjENCkhvc3Q6IDM0LjIyMy4xMjQuNDUNCkNvbm"
                        "5lY3Rpb246IGNsb3NlDQoNCg==:58");

    printf("[DESERIALIZED DATA]\n");
    printf("d_ip: %s\n", i2c_data->dst_ip);
    printf("d_port: %d\n", i2c_data->port);
    printf("decoded_data_length: %d\n", i2c_data->data_len);
    printf("data: \n");

    for (int i = 0; i < i2c_data->data_len; ++i) {
        printf("%c", i2c_data->data[i]);
    }

    new_tcp_tunnel("34.223.124.45", 80);

    int server_sock = search_conn("34.223.124.45", 80);

    if (server_sock != -1) {
        send_tunnel(server_sock, request, strlen(request));
    }

    char recv_buffer[1024];
    int recv_bytes;
    while (1) {
        recv_bytes = recv(server_sock, recv_buffer, 1024 - 1, 0);
        recv_buffer[recv_bytes] = '\0';
        printf("%s\n", recv_buffer);
        if (recv_bytes <= 0) {
            break;
        }
    }
}

void found_dns(const char *hostname, const ip_addr_t *ipaddr, void *arg)
{
    dns_t *state = (dns_t *)arg;
    if (ipaddr) {
        char *address = ipaddr_ntoa(ipaddr);
        printf("%s\n", address);
        state->completed = 1;
    }
    else {
        state->completed = -1;
    }
}

void test_dns()
{
    printf("Testing DNS...\n");
    ip_addr_t cached_addr;
    char *domain = "google.com";

    dns_t *state;
    state->domain = domain;
    state->completed = 0;

    printf("Querying %s\n", domain);

    while (!state->completed) {
        err_t query = dns_gethostbyname(domain, &cached_addr, found_dns, state);
        if (query == ERR_OK) {
            state->completed = 1;
        }
        else if (query != ERR_INPROGRESS) {
            state->completed = -1;
        }
    }
}
