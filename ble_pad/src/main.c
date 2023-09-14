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


// handle button presses
static void gpio_callback(uint gpio, uint32_t events)
{
    switch(gpio)
    {
    case 18:
        send_data(0x9901);
        printf(" DOWN ");
        return;
    case 19:
        send_data(0x9902);
        printf(" UP ");
        return;
    case 20:
        send_data(0x9903);
        printf(" RIGHT ");
        return;
    case 21:
        send_data(0x9904);
        printf(" LEFT ");
        return;
    }
}

int main()
{
    stdio_init_all();

    printf("Hello, world!\n");

    // attaches the gpio_callback function to the GPIOs
    gpio_set_irq_enabled_with_callback(18, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(19, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(20, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(21, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // initialize ble server
    BleServer_Init();

    // start the scheduler
    vTaskStartScheduler();

    while(1) {};
}
