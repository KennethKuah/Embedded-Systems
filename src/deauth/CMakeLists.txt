add_library(deauth INTERFACE)

target_sources(deauth INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/deauth.c
)