#include "wifi.h"

int setup_wifi()
{
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
    }
    cyw43_arch_enable_sta_mode();
    printf("Connecting to Wi-Fi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        exit(1);
    }
    else {
        printf("Connected.\n");
    }
}

void wifi_est_socket(char *ip_addr, int port)
{
    int client_socket;
    struct sockaddr_in server_addr;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_addr);
    server_addr.sin_port = htons(port);

    connect(client_socket, (struct sockaddr *)&server_addr,
            sizeof(server_addr));

    printf("Connected to socket!\n");

    closesocket(client_socket);
}
