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
#include "queue.h"
#include "pad.h"

#define DOWN_BUTTON_PIN 3
#define UP_BUTTON_PIN 4
#define RIGHT_BUTTON_PIN 5
#define LEFT_BUTTON_PIN 2

#define PAUSE_BUTTON_PIN 0
#define AUX_BUTTON_PIN 1

static QueueHandle_t pad_queue = NULL;
static pad_event_handler_t pad_event_handler = NULL;

static void pad_queue_data(uint16_t data)
{
    if (pad_queue != NULL)
    {
        xQueueSend(pad_queue, &data, 0);
    }
}

// handle button presses
static void gpio_callback(uint gpio, uint32_t events)
{

    if (events & GPIO_IRQ_EDGE_RISE) {
        switch(gpio)
        {
        case DOWN_BUTTON_PIN:
            pad_queue_data(0x9901);
            return;
        case UP_BUTTON_PIN:
            pad_queue_data(0x9902);
            return;
        case RIGHT_BUTTON_PIN:
            pad_queue_data(0x9903);
            return;
        case LEFT_BUTTON_PIN:
            pad_queue_data(0x9904);
            return;
        case PAUSE_BUTTON_PIN:
            printf("Pause\n");
            pad_queue_data(0x9905);
            return;
        case AUX_BUTTON_PIN:
            pad_queue_data(0x9906);
            return;
        }
    } else if (events & GPIO_IRQ_EDGE_FALL) {
        switch(gpio)
        {
        case DOWN_BUTTON_PIN:
            pad_queue_data(0x8801);
            return;
        case UP_BUTTON_PIN:
            pad_queue_data(0x8802);
            return;
        case RIGHT_BUTTON_PIN:
            pad_queue_data(0x8803);
            return;
        case LEFT_BUTTON_PIN:
            pad_queue_data(0x8804);
            return;
        case PAUSE_BUTTON_PIN:
            printf("Pause\n");
            pad_queue_data(0x8805);
            return;
        case AUX_BUTTON_PIN:
            pad_queue_data(0x8806);
            return;
        }
    }
}

static void pad_task(void* pv)
{
    uint16_t data;
    while(1)
    {
        if (xQueueReceive(pad_queue, &data, portMAX_DELAY) == pdTRUE)
        {
            pad_event_handler(data);
        }
    }
}

void pad_init(pad_event_handler_t event_handler)
{
    configASSERT( event_handler );

    // Set event handler
    pad_event_handler = event_handler;

    // Init PAD queue
    pad_queue = xQueueCreate(10, sizeof(uint16_t));

    // Attaches the gpio_callback function to the GPIOs
    gpio_set_irq_enabled_with_callback(DOWN_BUTTON_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(UP_BUTTON_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(RIGHT_BUTTON_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(LEFT_BUTTON_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(PAUSE_BUTTON_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(AUX_BUTTON_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);

    // Start the task
    if (xTaskCreate(pad_task, "PAD_Task", 1024, NULL, tskIDLE_PRIORITY+3, NULL) != pdPASS)
    {
        printf("Failed to create PAD task\n");
        while(1);
    }
}