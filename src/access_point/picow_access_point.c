#include "access_point.h"

static err_t tcp_close_client_connection(TCP_CONNECT_STATE_T *con_state, struct tcp_pcb *client_pcb, err_t close_err) {
    if (client_pcb) {
        assert(con_state && con_state->pcb == client_pcb);
        tcp_arg(client_pcb, NULL);
        tcp_poll(client_pcb, NULL, 0);
        tcp_sent(client_pcb, NULL);
        tcp_recv(client_pcb, NULL);
        tcp_err(client_pcb, NULL);
        err_t err = tcp_close(client_pcb);
        if (err != ERR_OK) {
            DEBUG_printf("close failed %d, calling abort\n", err);
            tcp_abort(client_pcb);
            close_err = ERR_ABRT;
        }
        if (con_state) {
            free(con_state);
        }
    }
    return close_err;
}

static void tcp_server_close(TCP_SERVER_T *state) {
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *pcb, u16_t len) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_sent %u\n", len);
    con_state->sent_len += len;
    if (con_state->sent_len >= con_state->header_len + con_state->result_len) {
        DEBUG_printf("all done\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    return ERR_OK;
}

static int test_server_content(const char *request, const char *params, char *result, size_t max_result_len) {
    int len = 0;
    if (strncmp(request, LED_TEST, sizeof(LED_TEST) - 1) == 0) {
        // Get the state of the led
        bool value;
        cyw43_gpio_get(&cyw43_state, LED_GPIO, &value);
        int led_state = value;

        // See if the user changed it
        if (params) {
            int led_param = sscanf(params, LED_PARAM, &led_state);
            if (led_param == 1) {
                if (led_state) {
                    // Turn led on
                    cyw43_gpio_set(&cyw43_state, 0, true);
                } else {
                    // Turn led off
                    cyw43_gpio_set(&cyw43_state, 0, false);
                }
            }
        }
        // Generate result
        if (led_state) {
            len = snprintf(result, max_result_len, LED_TEST_BODY, "ON", 0, "OFF");
        } else {
            len = snprintf(result, max_result_len, LED_TEST_BODY, "OFF", 1, "ON");
        }
    }
    return len;
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (!p) {
        DEBUG_printf("connection closed\n");
        return tcp_close_client_connection(con_state, pcb, ERR_OK);
    }
    assert(con_state && con_state->pcb == pcb);
    if (p->tot_len > 0) {
        DEBUG_printf("tcp_server_recv %d err %d\n", p->tot_len, err);
#if 0
        for (struct pbuf *q = p; q != NULL; q = q->next) {
            DEBUG_printf("in: %.*s\n", q->len, q->payload);
        }
#endif
        // Copy the request into the buffer
        pbuf_copy_partial(p, con_state->headers, p->tot_len > sizeof(con_state->headers) - 1 ? sizeof(con_state->headers) - 1 : p->tot_len, 0);

        // Handle GET request
        if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0) {
            char *request = con_state->headers + sizeof(HTTP_GET); // + space
            char *params = strchr(request, '?');
            if (params) {
                if (*params) {
                    char *space = strchr(request, ' ');
                    *params++ = 0;
                    if (space) {
                        *space = 0;
                    }
                } else {
                    params = NULL;
                }
            }

            // Generate content
            con_state->result_len = test_server_content(request, params, con_state->result, sizeof(con_state->result));
            DEBUG_printf("Request: %s?%s\n", request, params);
            DEBUG_printf("Result: %d\n", con_state->result_len);

            // Check we had enough buffer space
            if (con_state->result_len > sizeof(con_state->result) - 1) {
                DEBUG_printf("Too much result data %d\n", con_state->result_len);
                return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
            }

            // Generate web page
            if (con_state->result_len > 0) {
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
                    200, con_state->result_len);
                if (con_state->header_len > sizeof(con_state->headers) - 1) {
                    DEBUG_printf("Too much header data %d\n", con_state->header_len);
                    return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
                }
            } else {
                // Send redirect
                con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_REDIRECT,
                    ipaddr_ntoa(con_state->gw));
                DEBUG_printf("Sending redirect %s", con_state->headers);
            }

            // Send the headers to the client
            con_state->sent_len = 0;
            err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
            if (err != ERR_OK) {
                DEBUG_printf("failed to write header data %d\n", err);
                return tcp_close_client_connection(con_state, pcb, err);
            }

            // Send the body to the client
            if (con_state->result_len) {
                err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
                if (err != ERR_OK) {
                    DEBUG_printf("failed to write result data %d\n", err);
                    return tcp_close_client_connection(con_state, pcb, err);
                }
            }
        }
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t tcp_server_poll(void *arg, struct tcp_pcb *pcb) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    DEBUG_printf("tcp_server_poll_fn\n");
    return tcp_close_client_connection(con_state, pcb, ERR_OK); // Just disconnect clent?
}

static void tcp_server_err(void *arg, err_t err) {
    TCP_CONNECT_STATE_T *con_state = (TCP_CONNECT_STATE_T*)arg;
    if (err != ERR_ABRT) {
        DEBUG_printf("tcp_client_err_fn %d\n", err);
        tcp_close_client_connection(con_state, con_state->pcb, err);
    }
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        DEBUG_printf("failure in accept\n");
        return ERR_VAL;
    }
    DEBUG_printf("client connected\n");

    // Create the state for the connection
    TCP_CONNECT_STATE_T *con_state = calloc(1, sizeof(TCP_CONNECT_STATE_T));
    if (!con_state) {
        DEBUG_printf("failed to allocate connect state\n");
        return ERR_MEM;
    }
    con_state->pcb = client_pcb; // for checking
    con_state->gw = &state->gw;

    // setup connection to client
    tcp_arg(client_pcb, con_state);
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    return ERR_OK;
}

static bool tcp_server_open(void *arg, const char *ap_name) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, IP_ANY_TYPE, TCP_PORT);
    if (err) {
        DEBUG_printf("failed to bind to port %d\n",TCP_PORT);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }
    printf("Passed everything");
    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    DEBUG_printf("Server started on port %d\n", TCP_PORT);
    printf("Try connecting to '%s'\n", ap_name);
    return true;
}

// This function is used to calc elapsed time
bool repeating_timer_callback(struct repeating_timer *timer){
    char buffer[20];
    uint64_t current_time = time_us_64();
    // Obtaining the elapsed time in seconds
    float elapsed_time = (float)(current_time - start_time) / 1000000;
    // Format the float and store it in the buffer
    snprintf(buffer, sizeof(buffer), "%04.2f", elapsed_time);

    // Convert the formatted string back to a float
    float convertedFloat = strtof(buffer, NULL);
    if (convertedFloat >= 15.00){
        timeout = true;
    }
    return true;
}

// This function is used to fire the alarm. 
int64_t alarm_callback(alarm_id_t id, void *user_data) {
    timer_fired = true;
    printf("Timer %d fired!\n", (int) id);
    start_time = time_us_64();
    // Can return a value here in us to fire in the future
    return 0;
}

// Function is used to cancel timer
bool cancel_timer(struct repeating_timer *timer){
    timer_fired = false;
    return cancel_repeating_timer(timer);
}

static int add_scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    if (result){
        bool found = false;
        // This is to remove duplicates
        // if(result->auth_mode == 0){
        //     for (int i = 0; i < ARRAY_CTR; i++){
        //         if (strcmp(array_of_ssid[i].ssid, result->ssid) == 0)
        //             found = true;
        //     }
        //     if (!found) {
        //         if(ARRAY_CTR < MAX_SSID_COUNT){
        //             array_of_ssid[ARRAY_CTR] = *result;
        //             bool cancelled = cancel_timer(&timer);
        //             ARRAY_CTR++;
        //         }
        //     }
        // }
        for (int i = 0; i < ARRAY_CTR; i++){
            if (strcmp(array_of_ssid[i].ssid, result->ssid) == 0)
                found = true;
        }
        if (!found) {
            if(ARRAY_CTR < MAX_SSID_COUNT){
                array_of_ssid[ARRAY_CTR] = *result;
                // bool cancelled = cancel_timer(&timer);
                ARRAY_CTR++;
            }
        }
        // else{
        //     if(!timer_fired && finished == 0){
        //         add_alarm_in_ms(1000, alarm_callback, NULL, false);
        //         while (!timer_fired) {
        //             tight_loop_contents();
        //         }
        //         // Set to -1000 so that it runs and prints the elapsed time every 1 second
        //         add_repeating_timer_ms(-1000, repeating_timer_callback, NULL, &timer);
        //     }
        // }
    }
    return 0;
}

int wifi_scan(){
    absolute_time_t scan_time = nil_time;
    bool scan_in_progress = false;
    while(true){
        if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0) {
            if (!scan_in_progress) {
                cyw43_wifi_scan_options_t scan_options = {0};
                int err = cyw43_wifi_scan(&cyw43_state, &scan_options, NULL, add_scan_result);
                if (err == 0) {
                    printf("\nPerforming wifi scan\n");
                    scan_in_progress = true;
                } else {
                    printf("Failed to start scan: %d\n", err);
                    scan_time = make_timeout_time_ms(5000); // wait 5s and scan again
                }
            } else if (!cyw43_wifi_scan_active(&cyw43_state)) {
                scan_in_progress = false; 
            }
        }
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(scan_time);

        if(ARRAY_CTR == MAX_SSID_COUNT){
            printf("Scan finished\n");
            break;
        }
        // if (timeout)
        // {
        //     printf("max timout reached\n");
        //     finished = 1;
        //     bool cancelled = cancel_timer(&timer);
        //     break;
        // }
        
    }
    return 0;
}

int setup_ap() {
    stdio_usb_init();
    sleep_ms(3000);
    char input_buffer[8] = {0};
    int buf_ptr = 0;
    int ssid_select = -1;
    strcpy(ap_name, "picow_test"); // default AP name to connect to

    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return 1;
    }

    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        return 1;
    }
    sleep_ms(10000);
    cyw43_arch_enable_sta_mode();
    wifi_scan();

    for(int i = 0; i < ARRAY_CTR; i++) {
        printf("%i. SSID: ", (i + 1));
        if (strlen(array_of_ssid[i].ssid) == 0)
            printf("Hidden Network\t");
        else
            printf("%s\t", array_of_ssid[i].ssid);
        printf("AUTH MODE: %u\n", array_of_ssid[i].auth_mode);  
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
    strcpy(ap_name, array_of_ssid[ssid_select].ssid);
    const char *password = NULL;

    // cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);
    cyw43_arch_enable_ap_mode("picow_test", password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Start the dns server
    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);

    if (!tcp_server_open(state, ap_name)) {
        DEBUG_printf("failed to open server\n");
        return 1;
    }

    state->complete = false;
    while(!state->complete) {
        // the following #ifdef is only here so this same example can be used in multiple modes;
        // you do not need it in your code
#if PICO_CYW43_ARCH_POLL
        // if you are using pico_cyw43_arch_poll, then you must poll periodically from your
        // main loop (not from a timer interrupt) to check for Wi-Fi driver or lwIP work that needs to be done.
        cyw43_arch_poll();
        // you can poll as often as you like, however if you have nothing else to do you can
        // choose to sleep until either a specified time, or cyw43_arch_poll() has work to do:
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
#else
        // if you are not using pico_cyw43_arch_poll, then Wi-FI driver and lwIP work
        // is done via interrupt in the background. This sleep is just an example of some (blocking)
        // work you might be doing.
        sleep_ms(1000);
#endif
    }
    tcp_server_close(state);
    dns_server_deinit(&dns_server);
    dhcp_server_deinit(&dhcp_server);
    cyw43_arch_deinit();
    // for(int i = 0; i < 20; i++){
    //     free(array_of_ssid[i]);
    // }
    return 0;
}
