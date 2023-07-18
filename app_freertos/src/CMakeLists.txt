add_executable(app_freertos
        main.c
)

target_compile_definitions(app_freertos PRIVATE
PICO_DEFAULT_UART_TX_PIN=16
PICO_DEFAULT_UART_RX_PIN=17
)

target_include_directories(app_freertos PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
        )

target_link_libraries(app_freertos pico_stdlib hardware_adc hardware_uart freertos pico_cyw43_arch_lwip_threadsafe_background)

# enable usb output, disable uart output
#pico_enable_stdio_usb(blink 1)
#pico_enable_stdio_uart(blink 0)

pico_add_extra_outputs(app_freertos)


