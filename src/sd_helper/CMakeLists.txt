add_library(SDcard INTERFACE)

target_sources(SDcard INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/sd_card.c
)
