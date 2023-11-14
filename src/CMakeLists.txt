# add_executable(src
#         access_point/picow_access_point.c
#         dhcpserver/dhcpserver.c
#         dnsserver/dnsserver.c
#         )

# target_include_directories(src PRIVATE
#         ${CMAKE_CURRENT_LIST_DIR}
#         ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
#         ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
#         ${CMAKE_CURRENT_LIST_DIR}/dnsserver
#         )

# target_link_libraries(src
#         pico_cyw43_arch_lwip_threadsafe_background
#         pico_stdlib
#         )

# pico_add_extra_outputs(picow_access_point_background)
# pico_enable_stdio_usb(picow_access_point_background 1)
# pico_enable_stdio_uart(picow_access_point_background 1)

add_executable(src
        main.c
        wifi_scan/wifi_scan.c
        access_point/picow_access_point.c
        dhcpserver/dhcpserver.c
        dnsserver/dnsserver.c
        )
target_include_directories(src PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        ${CMAKE_CURRENT_LIST_DIR}/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/dnsserver
)
target_compile_definitions(src PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        NO_SYS=0            # don't want NO_SYS (generally this would be in your lwipopts.h)
        LWIP_SOCKET=1       # we need the socket API (generally this would be in your lwipopts.h)
        LWIP_TIMEVAL_PRIVATE=0 # Use system-wide timeval definition
)
target_link_libraries(
        src
        pico_cyw43_arch_lwip_sys_freertos
        pico_stdlib
        hardware_gpio
        pico_lwip_iperf
        FreeRTOS-Kernel-Heap4
)

pico_enable_stdio_usb(src 1)
pico_add_extra_outputs(src)




