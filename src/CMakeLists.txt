add_executable(pico_1
        pico_1.c
        wifi/wifi.c
        access_point/picow_access_point.c
        dnsserver/dnsserver.c
        dhcpserver/dhcpserver.c
        i2c_helper/i2c.c
        common/b64.c
        webserver/picow_web.c
        )

target_include_directories(pico_1 PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/..
        )

target_compile_definitions(pico_1 PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
        LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
        LWIP_TIMEVAL_PRIVATE=0 # Use system-wide timeval definition
        )

# pull in common dependencies
target_link_libraries(pico_1
        pico_cyw43_arch_lwip_sys_freertos
        pico_stdlib
        hardware_gpio
        pico_lwip_iperf
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        pico_i2c_slave
        hardware_i2c
        )

add_executable(pico_2
        pico_2.c
)

# enable usb output
pico_enable_stdio_usb(pico_1 1)
pico_enable_stdio_usb(pico_2 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico_1)
pico_add_extra_outputs(pico_2)
