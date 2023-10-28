add_executable(src
        main.c
        )

# pull in common dependencies
target_link_libraries(
        src
        pico_stdlib
        hardware_gpio
        )

# enable usb output
pico_enable_stdio_usb(src 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(src)
