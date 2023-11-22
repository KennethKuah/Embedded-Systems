add_subdirectory(deauth)
add_subdirectory(pico-wi)

add_executable(src
        main.c
        )

target_include_directories(src PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )

# pull in common dependencies
target_link_libraries(
        src
        pico_stdlib
        hardware_gpio
        deauth
        Wifi_SPI
        )

# enable usb output
pico_enable_stdio_usb(src 1)
pico_enable_stdio_uart(src 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(src)
