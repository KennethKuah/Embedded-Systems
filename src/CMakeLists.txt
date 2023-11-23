# pico 1 source files
add_executable(pico-1
        pico-1.c
        net_config/net_config.c
)

# pico 1 include directories for referencing driver/helper header files
target_include_directories(pico-1 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/net_config
)

# pico 2 target compile definitions
target_compile_definitions(pico-1 PRIVATE
)

# pico 1 common dependencies
target_link_libraries(pico-1
        pico_stdlib
        hardware_gpio
        pico_cyw43_arch_lwip_sys_freertos
        pico_lwip_iperf
        FreeRTOS-Kernel-Heap4
)

# pico 2 source files
add_executable(pico-2
        pico-2.c
)

# pico 2 include directories for referencing driver/helper header files
target_include_directories(pico-2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
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
)

# enable usb output
pico_enable_stdio_usb(pico-1 1)
pico_enable_stdio_usb(pico-2 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(pico-1)
pico_add_extra_outputs(pico-2)
