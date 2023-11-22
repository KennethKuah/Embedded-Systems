#include "wifi.h"

uint32_t led_ticks, poll_ticks, dns_ticks;

void connect_to_gateway_ap()
{
    bool ledon = false;
    MACADDR mac;

    add_event_handler(join_event_handler);
    add_event_handler(arp_event_handler);
    add_event_handler(dhcp_event_handler);
    add_event_handler(udp_event_handler);
    set_display_mode(DISP_INFO);
    io_init();
    printf("PicoWi DHCP client\n");
    if (!wifi_setup())
        printf("Error: SPI communication\n");
    else if (!wifi_init())
        printf("Error: can't initialise WiFi\n");
    else if (!join_start(SSID, PASSWD))
        printf("Error: can't start network join\n");
    else if (!ip_init(0))
        printf("Error: can't start IP stack\n");
    else {
        // Additional diagnostic display
        set_display_mode(DISP_INFO | DISP_JOIN | DISP_ARP | DISP_UDP |
                         DISP_DHCP | DISP_DNS);
        ustimeout(&led_ticks, 0);
        ustimeout(&poll_ticks, 0);
        while (dhcp_complete != 2) {
            // Toggle LED at 0.5 Hz if joined, 5 Hz if not
            if (ustimeout(&led_ticks, link_check() > 0 ? 1000000 : 100000))
                wifi_set_led(ledon = !ledon);
            // Get any events, poll the network-join state machine
            if (wifi_get_irq() || ustimeout(&poll_ticks, EVENT_POLL_USEC)) {
                event_poll();
                join_state_poll(SSID, PASSWD);
                ustimeout(&poll_ticks, 0);
            }
            // If link is up, poll DHCP state machine
            if (link_check() > 0)
                dhcp_poll();
            // When DHCP is complete, print IP addresses
            if (dhcp_complete == 1) {
                printf("DHCP complete, IP address ");
                print_ip_addr(my_ip);
                printf(" router ");
                print_ip_addr(router_ip);
                printf("\n");
                dhcp_complete = 2;
                ustimeout(&dns_ticks, 0);
            }
        }
    }
}

void dns_req(char *server_name)
{
    MACADDR mac;
    NET_SOCKET *usp = udp_sock_init(NULL, zero_ip, DNS_SERVER_PORT, LOCAL_PORT);
    int num_resps = 0;
    char temps[300];
    int oset = 0;
    IPADDR addr;

    while (!num_resps) {
        if (wifi_get_irq() || ustimeout(&poll_ticks, EVENT_POLL_USEC)) {
            event_poll();
            join_state_poll(SSID, PASSWD);
            ustimeout(&poll_ticks, 0);
        }

        if (dhcp_complete && ustimeout(&dns_ticks, 1000000)) {
            if (!ip_find_arp(router_ip, mac)) {
                ip_tx_arp(mac, router_ip, ARPREQ);
            }
            else {
                dns_tx(mac, dns_ip, LOCAL_PORT, server_name);

                num_resps = dns_num_resps(usp->rxdata, usp->rxlen);
                printf("Responses: %d\n", num_resps);
                for (int n = 0; n < dns_num_resps(usp->rxdata, usp->rxlen); n++)
                    printf("%s\n", dns_name_str(temps, usp->rxdata, usp->rxlen,
                                                &oset, 0, addr));
            }
        }
    }
}

// Serialises data that is transmitted via I2C
// Returns a string in the following format
// dst_ip:port:protocol:base64_data
// Buffer is dynamically allocated, meaning it must be freed after its used
//
char *i2c_serialize(char *dst, int port, char *proto, BYTE *data,
                    int data_len)
{
    char data_encoded[MAX_MESSAGE_SIZE - 256];
    const char *delimiter = ":";
    int encoded_len = base64_encode(data_encoded, data, data_len);
    char *buf = (char *)calloc(MAX_MESSAGE_SIZE, sizeof(char));
    char port_str[10];
    sprintf(port_str, "%d", port);
    strcat(buf, dst);
    strcat(buf, delimiter);
    strcat(buf, port_str);
    strcat(buf, delimiter);
    strcat(buf, proto);
    strcat(buf, delimiter);
    strcat(buf, data_encoded);

    return buf;
}

// Deserializes data that is transmitted via I2C
// Expects a string in the following format
// dst_ip:port:protocol:base64_data
I2CData *i2c_deserialize(char *buf)
{
    int idx = 0;
    int oset = 0;
    char tokens[5][MAX_MESSAGE_SIZE / 5];
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

    BYTE data_decoded[MAX_MESSAGE_SIZE - 256];
    char data_encoded[MAX_MESSAGE_SIZE - 256];
    strcpy(data_encoded, tokens[4]);

    int decoded_len =
        base64_decode(data_decoded, data_encoded, strlen(data_encoded));

    I2CData *i2c_data = (I2CData *)malloc(sizeof(I2CData));

    if(i2c_data != NULL) {
        i2c_data->dst_ip = tokens[0];
        char *end_ptr;
        i2c_data->port = strtol(tokens[1], &end_ptr, 10);
        i2c_data->proto = tokens[3];
        i2c_data->data = data_decoded;
        i2c_data->data_len = decoded_len;
    }

    return i2c_data;
}
