#include "access_point.h"
char* setup_ap(cyw43_ev_scan_result_t * ssid_array)
{
    char ap_ssid[33] = {0};
    char input_buffer[8] = {0};
    int buf_ptr = 0;
    int ssid_select = -1;

    printf("Enter the SSID index want to copy: ");
    while (true) {
        char c = getchar();

        // Remember to set the line ending to LF in serial monitor
        if (c == '\n' || buf_ptr == 8){
            input_buffer[buf_ptr] = '\0';
            ssid_select = atoi(input_buffer);
            if (ssid_select > 0 && ssid_select <= MAX_SSID_COUNT) {
                ssid_select--;
                break;
            }
            buf_ptr = 0;
            printf("You've entered %d\n", ssid_select);
            printf("The entered number is not in the range of 1 to %d.\n", MAX_SSID_COUNT);
        }

        input_buffer[buf_ptr] = c;
        buf_ptr++;
    }
    
    printf("Changing AP Name...\n");
    return ssid_array[ssid_select].ssid;
    strcpy(ap_ssid, ssid_array[ssid_select].ssid);
    const char *password = NULL;

    cyw43_arch_enable_ap_mode(ap_ssid, password, CYW43_AUTH_WPA2_AES_PSK);
    newap = true;
}

int setup_ap_old(cyw43_ev_scan_result_t * test_ssid) {
    const char * str = "HELLO UART!\n";
    char ap_ssid[33] = {0};
    stdio_usb_init();
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    sleep_ms(3000);
    char input_buffer[8] = {0};
    int buf_ptr = 0;
    int ssid_select = -1;
    strcpy(ap_ssid, "picow_test"); // default AP name to connect to

    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return 1;
    }

    // if (cyw43_arch_init()) {
    //     DEBUG_printf("failed to initialise\n");
    //     return 1;
    // }
    sleep_ms(10000);
    // cyw43_arch_enable_sta_mode();
    // wifi_scan();

    for(int i = 0; i < 20; i++) {
        printf("%i. SSID: ", (i + 1));
        if (strlen(test_ssid[i].ssid) == 0)
            printf("Hidden Network\t");
        else
            printf("%s\t", test_ssid[i].ssid);
        printf("AUTH MODE: %u\n", test_ssid[i].auth_mode);  
    }

    printf("Enter the SSID index want to copy: ");
    while (true) {
        char c = getchar();

        if (c == '\n' || buf_ptr == 8){
            input_buffer[buf_ptr] = '\0';
            ssid_select = atoi(input_buffer);
            if (ssid_select > 0 && ssid_select <= MAX_SSID_COUNT) {
                ssid_select--;
                break;
            }
            buf_ptr = 0;
            printf("You've entered %d\n", ssid_select);
            printf("The entered number is not in the range of 1 to %d.\n", MAX_SSID_COUNT);
        }

        input_buffer[buf_ptr] = c;
        buf_ptr++;
    }
    
    printf("Changing AP Name...\n");
    strcpy(ap_ssid, test_ssid[ssid_select].ssid);
    const char *password = NULL;

    // cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);
    cyw43_arch_enable_ap_mode("picow_test", password, CYW43_AUTH_WPA2_AES_PSK);

//     ip4_addr_t mask;
//     IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
//     IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

//     // Start the dhcp server
//     dhcp_server_t dhcp_server;
//     dhcp_server_init(&dhcp_server, &state->gw, &mask);

//     // Start the dns server
//     dns_server_t dns_server;
//     dns_server_init(&dns_server, &state->gw);

//     // This should check if the while loop has ended before ending the loop
//     while(*str){
//         // Sends a character at a time to the 
//         uart_write_blocking(UART_ID, (const uint8_t *)str, 1);
//         str++;
//     }


//     if (!tcp_server_open(state, ap_ssid)) {
//         DEBUG_printf("failed to open server\n");
//         return 1;
//     }

//     state->complete = false;
//     while(!state->complete) {
//         // the following #ifdef is only here so this same example can be used in multiple modes;
//         // you do not need it in your code
// #if PICO_CYW43_ARCH_POLL
//         // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
//         // main loop (not from a timer interrupt) to check for Wi-Fi driver or lwIP work that needs to be done.
//         cyw43_arch_poll();
//         // you can poll as often as you like, however if you have nothing else to do you can
//         // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
//         cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
// #else
//         // if you are not using pico_cyw43_arch_poll, then Wi-FI driver and lwIP work
//         // is done via interrupt in the background. This sleep is just an example of some (blocking)
//         // work you might be doing.
//         sleep_ms(1000);
// #endif
//     }
//     tcp_server_close(state);
//     dns_server_deinit(&dns_server);
//     dhcp_server_deinit(&dhcp_server);
    cyw43_arch_deinit();
    // for(int i = 0; i < 20; i++){
    //     free(array_of_ssid[i]);
    // }
    return 0;
}
