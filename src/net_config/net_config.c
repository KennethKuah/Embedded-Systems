#include "net_config.h"

// Default allow all authentication types.
// `CYW43_AUTH_OPEN`, `CYW43_AUTH_WPA_TKIP_PSK`, `CYW43_AUTH_WPA2_AES_PSK`, `CYW43_AUTH_WPA2_MIXED_PSK`
// Two or mode modes can be set by passing in with the '|' operand.
// E.g. `set_auth_scan_mode(CYW43_AUTH_OPEN | CYW43_AUTH_WPA_TKIP_PSK)`
int auth_scan_mode = (CYW43_AUTH_OPEN | CYW43_AUTH_WPA_TKIP_PSK |
        CYW43_AUTH_WPA2_AES_PSK | CYW43_AUTH_WPA2_MIXED_PSK);
volatile int num_scanned = 0;
volatile uint64_t start_time = 0;
volatile bool timeout = false;

// Initialize the cyw43 architecture and enable station mode
int init_wifi() {
    if (cyw43_arch_init()) {
        printf("Failed to initialize CYW43 Architecture\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    return 0;
}

// Sets wifi scan filter for the specified authentication mode(s)
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

        // Filters for the AP authentication mode(s)
        if(result->auth_mode & auth_scan_mode) {

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
// Returns the length of `scanned_results`.
int wifi_scan() {
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
    return num_scanned;
}

// Waits for console/serial input for a valid `scanned_results` index. 
// Returns the SSID name of the selected index.
char* select_ssid(cyw43_ev_scan_result_t * ssid_array) {
    char input_buffer[5] = {0};
    int buf_ptr = 0;
    int ssid_select = -1;

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
    
    return ssid_array[ssid_select].ssid;
}
