#include "picow_access_point.h"
//#include "wifi_scan.h"

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
        //gbuff = con_state->headers 
        // Handle GET request
        strcpy(gbuff, con_state->headers);
        testingg = true;
        // if (strncmp(HTTP_GET, con_state->headers, sizeof(HTTP_GET) - 1) == 0) {
        //     char *request = con_state->headers + sizeof(HTTP_GET); // + space
        //     char *params = strchr(request, '?');
        //     if (params) {
        //         if (*params) {
        //             char *space = strchr(request, ' ');
        //             *params++ = 0;
        //             if (space) {
        //                 *space = 0;
        //             }
        //         } else {
        //             params = NULL;
        //         }
        //     }
        //     // Generate content
        //     con_state->result_len = test_server_content(request, params, con_state->result, sizeof(con_state->result));
        //     DEBUG_printf("Request: %s?%s\n", request, params);
        //     DEBUG_printf("Result: %d\n", con_state->result_len);

        //     // Check we had enough buffer space
        //     if (con_state->result_len > sizeof(con_state->result) - 1) {
        //         DEBUG_printf("Too much result data %d\n", con_state->result_len);
        //         return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
        //     }

        //     // Generate web page
        //     if (con_state->result_len > 0) {
        //         con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_HEADERS,
        //             200, con_state->result_len);
        //         if (con_state->header_len > sizeof(con_state->headers) - 1) {
        //             DEBUG_printf("Too much header data %d\n", con_state->header_len);
        //             return tcp_close_client_connection(con_state, pcb, ERR_CLSD);
        //         }
        //     }
        //      else {
        //         // Send redirect
        //         con_state->header_len = snprintf(con_state->headers, sizeof(con_state->headers), HTTP_RESPONSE_REDIRECT,
        //             ipaddr_ntoa(con_state->gw));
        //         DEBUG_printf("Sending redirect %s", con_state->headers);
        //     }

        //     // Send the headers to the client
        //     con_state->sent_len = 0;
        //     err_t err = tcp_write(pcb, con_state->headers, con_state->header_len, 0);
        //     if (err != ERR_OK) {
        //         DEBUG_printf("failed to write header data %d\n", err);
        //         return tcp_close_client_connection(con_state, pcb, err);
        //     }

        //     // Send the body to the client
        //     if (con_state->result_len) {
        //         err = tcp_write(pcb, con_state->result, con_state->result_len, 0);
        //         if (err != ERR_OK) {
        //             DEBUG_printf("failed to write result data %d\n", err);
        //             return tcp_close_client_connection(con_state, pcb, err);
        //         }
        //     }
        // }
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    return ERR_OK;
}

int setup_web_server() {
    stdio_init_all();

    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return 1;
    }

    if (cyw43_arch_init()) {
        DEBUG_printf("failed to initialise\n");
        return 1;
    }
    const char *password = NULL;

    // cyw43_arch_enable_ap_mode(ap_name, password, CYW43_AUTH_WPA2_AES_PSK);
    cyw43_arch_enable_ap_mode("picowtesting", password, CYW43_AUTH_WPA2_AES_PSK);

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Start the dns server
    dns_server_t dns_server;
    dns_server_init(&dns_server, &state->gw);

    // if (!tcp_server_open(state, ap_name)) {
    //     DEBUG_printf("failed to open server\n");
    //     return 1;
    // }
    
    if (!tcp_server_open(state, "picotesting")) {
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
