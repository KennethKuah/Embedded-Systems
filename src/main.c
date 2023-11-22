#include "pico/stdlib.h"
#include "wifi/wifi.h"
#include <stdio.h>

int main()
{
    // main body of code
    stdio_init_all();
    connect_to_gateway_ap();
    dns_req("raspberrypi.org");
    BYTE test[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
    char *data = i2c_serialize("testing.com", 8000, "TCP", test, sizeof(test));

    printf("Data: %s\n", data);
    I2CData *i2c_data = i2c_deserialize(
        "testing.com:8080:TCP:1000:4nH1gaScH9qOm0Bg7Yhybf/"
        "l2MbF8fjB5nddlJBLnK9lgFK8UR5XMLxckE00L2B/aDBNoOSfn5PFSPw/"
        "YmQFeRrinWP4HEfJBUCY6XHQH1dFj9MQ1DWjyBaDKBacK0xodnOot0DQWPB6iWlpFcEnfc"
        "KtcLV5V0t3VHsxAfgUpz7ebZL4N9hTm4jyG2qUVcNQ+p9/"
        "TJogdS8+AKeftW77iJ+F4LOIR5r52+fyStkQokpypcnXbMyhCjoIbEOjmICQ+5hHK776+"
        "Q4M8K1TBGhn5nkql3OptIcVccHTBoLcWMtHXnFccxjGDld+o3+"
        "GE7IVxIn9QM7aXBtu2RQWKHQu");

    printf("%s\n", i2c_data->dst_ip);
    printf("%d\n", i2c_data->port);

    for(int i = 0; i < i2c_data->data_len; ++i) {
        printf("%x ", i2c_data->data[i]);
    }
    while (true) {
        tight_loop_contents();
    }
    return 0;
}
