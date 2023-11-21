add_executable(i2c_slave
    slave.c
    )
target_link_libraries(i2c_slave
    pico_i2c_slave
    hardware_i2c
    pico_stdlib
    )
pico_add_extra_outputs(i2c_slave)
pico_enable_stdio_usb(i2c_slave 1)
pico_enable_stdio_uart(i2c_slave 0)
# example_auto_set_url(i2c_slave)
