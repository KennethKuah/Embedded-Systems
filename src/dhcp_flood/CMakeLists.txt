add_library(dhcp_flood INTERFACE)

target_sources(dhcp_flood INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/dhcp_flood.c
)