add_executable(ble_server
        ble_server.c
        main.c
        pad.c
)

target_compile_definitions(ble_server PRIVATE
PICO_DEFAULT_UART_TX_PIN=16
PICO_DEFAULT_UART_RX_PIN=17
NO_SYS=0
TCPIP_MBOX_SIZE=8
DEFAULT_THREAD_STACKSIZE=1024
TCPIP_THREAD_STACKSIZE=1024
)

target_include_directories(ble_server PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )

target_link_libraries(ble_server
                      freertos
                      pico_stdlib           # pico standard library
                      hardware_uart
                      hardware_adc          # internal temperature and ADC
                      pico_btstack_ble      # BLE stack
                      pico_btstack_cyw43    # BLE stack with CYW43
                      pico_cyw43_arch_lwip_sys_freertos  # full lwIP stack including blocking sockets, with NO_SYS=0
                )

# enable usb output, disable uart output
#pico_enable_stdio_usb(blink 1)
#pico_enable_stdio_uart(blink 0)

pico_btstack_make_gatt_header(ble_server PRIVATE "${CMAKE_CURRENT_LIST_DIR}/temp_sensor.gatt")

pico_add_extra_outputs(ble_server)


