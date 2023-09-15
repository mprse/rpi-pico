#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "ble_server.h"
#include "pad.h"

int main()
{
    stdio_init_all();

    printf("Hello, world!\n");

    // initialize pad
    pad_init(ble_server_send);

    // initialize ble server
    ble_server_init();

    // start the scheduler
    vTaskStartScheduler();

    while(1) {};
}
