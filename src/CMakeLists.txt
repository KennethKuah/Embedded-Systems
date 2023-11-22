add_executable(src
        main.c
        wifi/wifi.c
        common/b64.c
        )

target_include_directories(src PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/..
        )

target_compile_definitions(src PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
        LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
        LWIP_COMPAT_SOCKET=1
        LWIP_TIMEVAL_PRIVATE=0 # Use system-wide timeval definition
        )

# pull in common dependencies
target_link_libraries(
        src
        pico_cyw43_arch_lwip_sys_freertos
        pico_stdlib
        pico_lwip_iperf
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        )

# enable usb output
pico_enable_stdio_usb(src 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(src)
