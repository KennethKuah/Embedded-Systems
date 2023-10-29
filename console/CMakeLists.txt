add_executable(ProjConsole
        projconsole.c
        )

# pull in common dependencies
target_link_libraries(ProjConsole pico_stdlib)

# enable usb output, enable uart output
pico_enable_stdio_usb(ProjConsole 1)
pico_enable_stdio_uart(ProjConsole 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(ProjConsole)

# add url via pico_set_program_url
example_auto_set_url(ProjConsole)
