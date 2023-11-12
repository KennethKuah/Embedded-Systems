# Tell CMake where to find sd card driver source code
add_subdirectory(${PROJECT_SOURCE_DIR}/include/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src build)
add_subdirectory(sd_helper)

add_executable(src
        main.c
        hw_config.c
)

# Add the standard library and user defined libraries to the build
target_link_libraries(
        src
        pico_stdlib
        FatFs_SPI
        SDcard
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(src 1)
pico_enable_stdio_uart(src 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(src)
