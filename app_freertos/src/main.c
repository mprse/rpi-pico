#include <FreeRTOS.h>
#include <task.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/uart.h"

#define UART_ID uart1
#define BAUD_RATE 9600
#define UART_TX_PIN 4
#define UART_RX_PIN 5
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

/* Choose 'C' for Celsius or 'F' for Fahrenheit. */
#define TEMPERATURE_UNITS 'C'

void init_temp_sensor(void)
{
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);
}

void init_rfid(void)
{
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);
    uart_set_fifo_enabled(UART_ID, true);
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;
    irq_set_enabled(UART_IRQ, false);
    uart_set_irq_enables(UART_ID, false, false);

    // flush the read buffer
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
    }
}

/* References for this implementation:
 * raspberry-pi-pico-c-sdk.pdf, Section '4.1.1. hardware_adc'
 * pico-examples/adc/adc_console/adc_console.c */
float read_onboard_temperature(const char unit) {

    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.36f / (1 << 12);

    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    if (unit == 'C') {
        return tempC;
    } else if (unit == 'F') {
        return tempC * 9 / 5 + 32;
    }

    return -1.0f;
}

void led_task()
{
    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        while(1) {};
    }

    while (true) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        vTaskDelay(100);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        vTaskDelay(100);
    }
}

void temp_task()
{
    init_temp_sensor();

    while (true) {
        float temperature = read_onboard_temperature(TEMPERATURE_UNITS);
        printf("Onboard temp sensor = %.02f %c\n", temperature, TEMPERATURE_UNITS);

        vTaskDelay(100);
    }
}

void rfid_task()
{
    init_rfid();

    while (true) {
        uint8_t buf[100];
        int cnt = 0;
        while(uart_is_readable(UART_ID)) {
            uint8_t ch = uart_getc(UART_ID);
            buf[cnt] = ch;
            cnt++;
        }

        printf("TAG(%d): ", cnt);
        for(int i = 0; i < cnt; i++) {
            printf("%c", buf[i]);
        }
        printf("\n");


        vTaskDelay(100);
    }
}

int main()
{
    stdio_init_all();

    printf("Hello, world!\n");

    xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
    xTaskCreate(temp_task, "TEMP_Task", 256, NULL, 1, NULL);
    xTaskCreate(rfid_task, "RFID_Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while(1){};
}
