add_executable(i2c
    i2c.c
    )
target_link_libraries(i2c
    pico_i2c_slave
    hardware_i2c
    pico_stdlib
    )
pico_add_extra_outputs(i2c)
pico_enable_stdio_usb(i2c 1)
# example_auto_set_url(i2c)
