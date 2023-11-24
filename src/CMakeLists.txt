# pico 1 source files
add_executable(pico-1
        pico-1.c
        common/b64.c
        i2c_bridge/i2c_bridge.c
        net_config/net_config.c
        net_config/dhcpserver/dhcpserver.c
        net_config/dnsserver/dnsserver.c
        net_config/tcpserver/tcpserver.c
)

# pico 1 include directories for referencing driver/helper header files
target_include_directories(pico-1 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/common
        ${CMAKE_CURRENT_LIST_DIR}/i2c_bridge
        ${CMAKE_CURRENT_LIST_DIR}/net_config
        ${CMAKE_CURRENT_LIST_DIR}/net_config/dhcpserver
        ${CMAKE_CURRENT_LIST_DIR}/net_config/dnsserver
        ${CMAKE_CURRENT_LIST_DIR}/net_config/tcpserver
)

# pico 2 target compile definitions
target_compile_definitions(pico-1 PRIVATE
)

# pico 1 common dependencies
target_link_libraries(pico-1
        pico_stdlib
        hardware_gpio
        hardware_i2c
        pico_cyw43_arch_lwip_sys_freertos
        pico_lwip_iperf
        pico_i2c_slave
        FreeRTOS-Kernel-Heap4
)

# pico 2 source files
add_executable(pico-2
        pico-2.c
        common/b64.c
        i2c_bridge/i2c_bridge.c
)

# pico 2 include directories for referencing driver/helper header files
target_include_directories(pico-2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/common
        ${CMAKE_CURRENT_LIST_DIR}/i2c_bridge
)

# pico 2 target compile definitions
target_compile_definitions(pico-2 PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

# pico 2 common dependencies
target_link_libraries(pico-2
        pico_stdlib
        hardware_gpio
        hardware_i2c
        pico_i2c_slave
)

# enable usb output
pico_enable_stdio_usb(pico-1 1)
pico_enable_stdio_usb(pico-2 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico-1)
pico_add_extra_outputs(pico-2)
