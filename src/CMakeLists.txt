# add external repo subdirectory to build sd card interface
add_subdirectory(${PROJECT_SOURCE_DIR}/include/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)
# add subdirectory for custom lwip library
add_subdirectory(lwip_custom)

# pico 1 source files
add_executable(pico-1
        pico-1.c
        hw_config.c
        common/b64.c
        i2c_bridge/i2c_bridge.c
        net_config/net_config.c
        net_config/dhcpserver/dhcpserver.c
        net_config/dnsserver/dnsserver.c
        net_config/tcpserver/tcpserver.c
        pcap_helper/sd_card.c
        pcap_helper/pcap.c
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
        ${CMAKE_CURRENT_LIST_DIR}/pcap_helper
)

# pico 1 target compile definitions
target_compile_definitions(pico-1 PRIVATE
        IP_FORWARD=1
        SD_MOUNTED=0
        PCAP_OVERWRITE=0
)

# pico 1 common dependencies
target_link_libraries(pico-1
        pico_stdlib
        hardware_gpio
        hardware_i2c
        pico_cyw43_arch_sys_freertos
        pico_lwip_custom # modified lwip library
        pico_lwip_iperf
        pico_i2c_slave
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
        FatFs_SPI # SD Card library
)

# pico 2 source files
add_executable(pico-2
        pico-2.c
        common/b64.c
        i2c_bridge/i2c_bridge.c
        net_manager/net_manager.c
)

# pico 2 include directories for referencing driver/helper header files
target_include_directories(pico-2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/common
        ${CMAKE_CURRENT_LIST_DIR}/i2c_bridge
        ${CMAKE_CURRENT_LIST_DIR}/net_manager
)

# pico 2 target compile definitions
target_compile_definitions(pico-2 PRIVATE
        WIFI_SSID=\"${WIFI_SSID}\"
        WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
        LWIP_SOCKET=1           # we need the socket API (generally this would be in your lwipopts.h)
        LWIP_COMPAT_SOCKET=1
)

# pico 2 common dependencies
target_link_libraries(pico-2
        pico_stdlib
        hardware_i2c
        pico_cyw43_arch_lwip_sys_freertos
        pico_lwip_iperf
        pico_i2c_slave
        FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
)

# enable usb output
pico_enable_stdio_usb(pico-1 1)
pico_enable_stdio_usb(pico-2 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico-1)
pico_add_extra_outputs(pico-2)
