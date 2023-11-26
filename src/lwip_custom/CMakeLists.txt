set(PICO_LWIP_PATH ${PICO_SDK_PATH}/lib/lwip)
set (LWIP_INCLUDE_DIRS
    ${CMAKE_CURRENT_LIST_DIR}/include
    ${PICO_LWIP_PATH}/contrib
    ${PICO_LWIP_PATH}/contrib/ports/freertos/include
    ${CMAKE_CURRENT_LIST_DIR}/../net_config/
        )

pico_add_library(pico_lwip_core4_custom NOFLAG)
target_sources(pico_lwip_core4_custom INTERFACE
        ${PICO_LWIP_PATH}/src/core/ipv4/autoip.c
        ${PICO_LWIP_PATH}/src/core/ipv4/dhcp.c
        ${PICO_LWIP_PATH}/src/core/ipv4/etharp.c
        ${PICO_LWIP_PATH}/src/core/ipv4/icmp.c
        ${PICO_LWIP_PATH}/src/core/ipv4/igmp.c
        ${PICO_LWIP_PATH}/src/core/ipv4/ip4_frag.c
        # ${PICO_LWIP_PATH}/src/core/ipv4/ip4.c
        ${PICO_LWIP_PATH}/src/core/ipv4/ip4_addr.c

        # modified sources of pico_lwip_core4 interface
        ${CMAKE_CURRENT_LIST_DIR}/ip4.c
        )

pico_add_library(pico_lwip_core6_custom NOFLAG)
target_sources(pico_lwip_core6_custom INTERFACE
        ${PICO_LWIP_PATH}/src/core/ipv6/dhcp6.c
        ${PICO_LWIP_PATH}/src/core/ipv6/ethip6.c
        ${PICO_LWIP_PATH}/src/core/ipv6/icmp6.c
        ${PICO_LWIP_PATH}/src/core/ipv6/inet6.c
        # ${PICO_LWIP_PATH}/src/core/ipv6/ip6.c
        ${PICO_LWIP_PATH}/src/core/ipv6/ip6_addr.c
        ${PICO_LWIP_PATH}/src/core/ipv6/ip6_frag.c
        ${PICO_LWIP_PATH}/src/core/ipv6/mld6.c
        ${PICO_LWIP_PATH}/src/core/ipv6/nd6.c

        # modified sources of pico_lwip_core6 interface
        ${CMAKE_CURRENT_LIST_DIR}/ip6.c
        )

pico_add_library(pico_lwip_custom NOFLAG)
target_include_directories(pico_lwip_custom_headers INTERFACE ${LWIP_INCLUDE_DIRS})
target_sources(pico_lwip_custom INTERFACE
        # original source of pico_lwip_api interface
        ${PICO_LWIP_PATH}/src/api/api_lib.c
        ${PICO_LWIP_PATH}/src/api/api_msg.c
        ${PICO_LWIP_PATH}/src/api/err.c
        ${PICO_LWIP_PATH}/src/api/if_api.c
        ${PICO_LWIP_PATH}/src/api/netbuf.c
        ${PICO_LWIP_PATH}/src/api/netdb.c
        ${PICO_LWIP_PATH}/src/api/netifapi.c
        ${PICO_LWIP_PATH}/src/api/sockets.c
        # ${PICO_LWIP_PATH}/src/api/tcpip.c

        # modified sources of pico_lwip_api interface
        ${CMAKE_CURRENT_LIST_DIR}/tcpip.c
        )

pico_mirrored_target_link_libraries(pico_lwip_custom INTERFACE
        pico_async_context_base
        # pico_lwip interface (w/out pico_lwip_api for customization of source files)
        pico_lwip_core
        pico_lwip_core4_custom
        pico_lwip_core6_custom
        pico_lwip_core6
        pico_lwip_netif
        pico_lwip_sixlowpan
        pico_lwip_ppp
        # endof pico_lwip interface (w/out pico_lwip_api)
        pico_lwip_contrib_freertos
        pico_rand
        )

target_compile_definitions(pico_lwip_custom INTERFACE
        CYW43_LWIP=1
        LWIP_PROVIDE_ERRNO=1
        # now the default
        #PICO_LWIP_CUSTOM_LOCK_TCPIP_CORE=1 # we want to override the lwip locking mechanism to use our mutex
        )
