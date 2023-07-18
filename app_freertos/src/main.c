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


#define UDP_PORT 4444
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "10.1.1.170"
#define BEACON_INTERVAL_MS 1000

struct udp_pcb* pcb = NULL;

void udp_send_packet(uint32_t value) {
    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr);

    int counter = 0;
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX+1, PBUF_RAM);
    char *req = (char *)p->payload;
    memset(req, 0, BEACON_MSG_LEN_MAX+1);
    snprintf(req, BEACON_MSG_LEN_MAX, "%d", value);
    err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
    pbuf_free(p);
    if (er != ERR_OK) {
        printf("Failed to send UDP packet! error=%d", er);
    } else {
        printf("Sent packet %d\n", counter);
        counter++;
    }
}

void gpio_callback(uint gpio, uint32_t events) {
    switch(gpio){
        case 18:
            udp_send_packet(1);
            printf("DOWN\n");
            return;
        case 19:
            udp_send_packet(2);
            printf("UP\n");
            return;
        case 20:
            udp_send_packet(3);
            printf("RIGHT\n");
            return;
        case 21:
            udp_send_packet(4);
            printf("LEFT\n");
            return;
    }
}

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

void udp_task() {
    struct udp_pcb* pcb = udp_new();

    ip_addr_t addr;
    ipaddr_aton(BEACON_TARGET, &addr);

    int counter = 0;
    while (true) {
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX+1, PBUF_RAM);
        char *req = (char *)p->payload;
        memset(req, 0, BEACON_MSG_LEN_MAX+1);
        snprintf(req, BEACON_MSG_LEN_MAX, "msg: %d\n", counter);
        err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
        pbuf_free(p);
        if (er != ERR_OK) {
            printf("Failed to send UDP packet! error=%d", er);
        } else {
            printf("Sent packet %d\n", counter);
            counter++;
        }

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

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed");
        while(1) {};
    }

    cyw43_arch_enable_sta_mode();

    printf("Connecting to Wi-Fi...\n");

    gpio_set_irq_enabled_with_callback(18, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(19, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(20, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(21, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    pcb = udp_new();

    if (cyw43_arch_wifi_connect_timeout_ms("Polinezja", "dupa1212", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else {
        printf("Connected.\n");
    }


    xTaskCreate(led_task, "LED_Task", 256, NULL, 1, NULL);
    xTaskCreate(temp_task, "TEMP_Task", 256, NULL, 1, NULL);
    xTaskCreate(rfid_task, "RFID_Task", 256, NULL, 1, NULL);
    //xTaskCreate(udp_task, "UDP_Task", 256, NULL, 1, NULL);
    vTaskStartScheduler();

    while(1){};
}
