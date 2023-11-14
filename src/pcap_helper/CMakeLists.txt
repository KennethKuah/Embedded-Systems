add_library(Wifi_SPI INTERFACE)

pico_generate_pio_header(Wifi_SPI ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_pio.pio)

# Firmware file for CYW43439 or CYW4343W
if (${CHIP_4343W})
    message (STATUS "Building CYW4343W version")
    set (FW_FILE ${PROJECT_SOURCE_DIR}/include/picowi/firmware/fw_4343w.c)
    add_compile_options(-DCHIP_4343W)
else ()
    message (STATUS "Building Pico-W CYW43439 version")
    set (FW_FILE ${PROJECT_SOURCE_DIR}/include/picowi/firmware/fw_43439.c)
endif ()

target_sources(Wifi_SPI INTERFACE
    ${FW_FILE}
    ${CMAKE_CURRENT_LIST_DIR}/sd_card.c
    ${CMAKE_CURRENT_LIST_DIR}/picowi_init.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_pico.c  
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_wifi.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_ioctl.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_scan.c 
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_event.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_join.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_pio.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_ip.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_udp.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_dhcp.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_dns.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_net.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_tcp.c
    ${PROJECT_SOURCE_DIR}/include/picowi/lib/picowi_web.c
)
target_include_directories(Wifi_SPI INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
    ${PROJECT_SOURCE_DIR}/include/picowi/lib
)
target_link_libraries(Wifi_SPI INTERFACE
    pico_stdlib
    hardware_pio
    hardware_dma
)