#include "net_config.h"
#include "dhcpserver.h"
#include "dnsserver.h"
#include "tcpserver.h"

// Default value to allow all authentication types.
int auth_scan_mode = (AUTH_OPEN | AUTH_WEP | AUTH_WPA_PSK | AUTH_WPA2_PSK | 
                        AUTH_WPA_WPA2_PSK | AUTH_WPA2_ENTERPRISE);
cyw43_ev_scan_result_t scanned_results[MAX_SCAN_RESULTS];
volatile int num_scanned = 0;
volatile uint64_t start_time = 0;
volatile bool timeout = false;
char* _ap_name;

// Initialize the cyw43 architecture and enable station mode
int init_wifi() {
    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43 Architecture\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    return 0;
}

// Sets wifi scan filter for the specified authentication mode(s).
// Two or mode modes can be set by passing in with the '|' operand.
// E.g. `set_auth_scan_mode(AUTH_OPEN | AUTH_WPA_WPA2_PSK)`
// \param mask the authorization type to use
void set_auth_scan_mode(int mask){
    auth_scan_mode = mask;
}

// Callback function for calculating the time passed since the start of scan.
// If the timeout has been reached, toggles the timeout boolean to true
bool timeout_callback(struct repeating_timer *timer) {
    uint64_t current_time = time_us_64();

    // Obtaining the elapsed time in seconds
    float elapsed_time = (float)(current_time - start_time) / 1000000;

    // If the scan has reached over timeout time, toggle timeout boolean 
    // and cancel the repeating timer
    if (elapsed_time >= SCAN_TIMEOUT){
        timeout = true;
        cancel_repeating_timer(timer);
    }
    return true;
}

// Adds the WiFi AP scanned results to array->`scanned_results`
static int add_scan_result(void *env, const cyw43_ev_scan_result_t *result) {
    if (result) {

        // Filters for the AUTH_OPEN or other AP authentication mode(s)
        if(result->auth_mode == 0 || result->auth_mode & auth_scan_mode) {

            // This is to prevent duplicate AP scan results
            for (int i = 0; i < num_scanned; i++) {
                if (strcmp(scanned_results[i].ssid, result->ssid) == 0)
                return 0;
            }
            
            // Add AP scan result to scanned_results array
            if(num_scanned < MAX_SCAN_RESULTS) {
                scanned_results[num_scanned] = *result;
                num_scanned++;
            }
        }
    }
    return 0;
}

// Performs the WiFi AP scan until the `scanned_results` is filled or until the `SCAN_TIMEOUT` has been reached. 
// Returns the pointer to `scanned_results`.
void perform_wifi_scan() {
    absolute_time_t scan_time = nil_time;
    bool scan_in_progress = false;
    struct repeating_timer timer;

    // Set the start time for the wifi scan
    start_time = time_us_64();
    // Add repeating timer to trigger timeout for wifi scan
    add_repeating_timer_ms(-1000, timeout_callback, NULL, &timer);

    while(true) {
        if (absolute_time_diff_us(get_absolute_time(), scan_time) < 0) {
            if (!scan_in_progress) {
                printf("Trying to start scan\n");
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

        if(num_scanned == MAX_SCAN_RESULTS){
            cancel_repeating_timer(&timer);
            printf("Scan finished\n");
            break;
        }

        if (timeout) {
            printf("Max timeout reached\n");
            break;
        }
    }
}

// Prints the wifi scan results to serial
void print_wifi_scan_results() {
    printf(" ===== WiFi Access Points visible nearby ===== \n");
    for(int i = 0; i < num_scanned; i++) {
        printf("%i. SSID: ", (i + 1));
        if (strlen(scanned_results[i].ssid) == 0)
            printf("Hidden Network\t");
        else
            printf("%s\t", scanned_results[i].ssid);
        printf("AUTH MODE: %s\n",
            scanned_results[i].auth_mode == AUTH_OPEN ? "No authorisation required (open)" :
            scanned_results[i].auth_mode == AUTH_WEP ? "WEP authorisation" :
            scanned_results[i].auth_mode == AUTH_WPA_PSK ? "WPA authorisation" :
            scanned_results[i].auth_mode == AUTH_WPA2_PSK ? "WPA2 authorisation (preferred)" :
            scanned_results[i].auth_mode == AUTH_WPA_WPA2_PSK ? "WPA2/WPA mixed authorisation" :
            scanned_results[i].auth_mode == AUTH_WPA2_ENTERPRISE ? "WPA2-ENTERPRISE authorisation" :
            "Unknown");
    }
    printf(" =========== End of Scan Results ============= \n");
}

// Waits for console/serial input for a valid `scanned_results` index. 
// Returns the SSID name of the selected index.
// \param ssid_aray the array of wifi scanned results
char* select_ssid() {
    char input_buffer[5] = {0};
    int buf_ptr = 0;
    int ssid_select = -1;

    print_wifi_scan_results();
    printf("Enter the SSID index you want to copy-> ");

    while (true) {
        char c = getchar();

        // Check for end of input or buffer limit
        if (c == '\r' || c == '\n' || buf_ptr == sizeof(input_buffer) - 1){
            input_buffer[buf_ptr] = '\0';
            ssid_select = atoi(input_buffer);
            
            // Validate the selected index
            if (ssid_select > 0 && ssid_select <= num_scanned) {
                ssid_select--;
                break;
            }

            // Reset buffer and print error message
            buf_ptr = 0;
            printf("Invalid input. Please enter a number between 1 and %d-> ", num_scanned);
        }

        input_buffer[buf_ptr] = c;
        buf_ptr++;
    }
    
    return scanned_results[ssid_select].ssid;
}

//  Enables Wi-Fi AP (Access point) mode.
// This enables the Wi-Fi in Access Point mode such that connections can be made to the device by other Wi-Fi clients
// \param ap_name the name for the access point
// \param password the password to use or NULL for no password.
void configure_ap(char* ap_name, char* password) {
    _ap_name = ap_name;
    cyw43_arch_enable_ap_mode(_ap_name, password, CYW43_AUTH_WPA2_AES_PSK);
}

// Initializes the DHCP, DNS services and binds the `TCP_PORT` for TCP connections.
// Continuously polls for client connections.
int run_server() {
    TCP_SERVER_T *state = tcp_server_init();
    if (!state) {
        DEBUG_printf("failed to allocate state\n");
        return 1;
    }

    ip4_addr_t mask;
    IP4_ADDR(ip_2_ip4(&state->gw), 192, 168, 4, 1);
    IP4_ADDR(ip_2_ip4(&mask), 255, 255, 255, 0);

    // Start the dhcp server
    dhcp_server_t dhcp_server;
    dhcp_server_init(&dhcp_server, &state->gw, &mask);

    // Start the dns server
    // dns_server_t dns_server;
    // dns_server_init(&dns_server, &state->gw);

    // if (!tcp_server_open(state, _ap_name)) {
    //     DEBUG_printf("failed to open server\n");
    //     return 1;
    // }
    printf("Try connecting to '%s'\n", _ap_name);
    while(!state->complete) {
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(1000));
    }
    // tcp_server_close(state);
    // dns_server_deinit(&dns_server);
    dhcp_server_deinit(&dhcp_server);
    cyw43_arch_deinit();
    return 0;
}
