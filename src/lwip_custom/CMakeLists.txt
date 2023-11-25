set(PICO_LWIP_PATH ${PICO_SDK_PATH}/lib/lwip)

pico_add_library(pico_lwip_custom NOFLAG)
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
        pico_lwip_core4
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
