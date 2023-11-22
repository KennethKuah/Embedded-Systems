#include "wifi.h"

cyw43_ev_scan_result_t array_of_ssid[MAX_SSID_COUNT];
volatile int ARRAY_CTR = 0;
volatile bool timer_fired = false;
volatile uint64_t start_time = 0;
volatile bool timeout = false;
volatile int finished = 0;

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

cyw43_ev_scan_result_t * setup_wifi_scan(){
    printf("Entered WIFI Scan");
    if (cyw43_arch_init()) {
        printf("failed to initialise\n");
    }
    printf("After initialised\n");
    cyw43_arch_enable_sta_mode();
    printf("After cyw43\n");
    wifi_scan();
    cyw43_arch_deinit();

    return array_of_ssid;
}