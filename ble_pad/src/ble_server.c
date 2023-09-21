// Description: BLE server implementation for the Pico

#include <FreeRTOS.h>
#include <task.h>
#include "platform.h"
#include "ble_server.h"
#include "pico/cyw43_arch.h"
#include "pico/btstack_cyw43.h"
#include <stdio.h>
#include "btstack.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include "temp_sensor.h"

#define HEARTBEAT_PERIOD_MS 300
#define APP_AD_FLAGS 0x06
static uint8_t adv_data[] =
{
    /* Flags general discoverable */
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, APP_AD_FLAGS,
    /* Name */
    0x17, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME, 'P', 'I', 'C', 'O', ' ', '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0', ':', '0', '0',
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS, 0x1a, 0x18,
};
static const uint8_t adv_data_len = sizeof(adv_data);

static int le_notification_enabled = 0;
static hci_con_handle_t con_handle = HCI_CON_HANDLE_INVALID;
static uint16_t data_to_send = 0;

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    UNUSED(size);
    UNUSED(channel);
    bd_addr_t local_addr;
    if (packet_type != HCI_EVENT_PACKET) return;

    uint8_t event_type = hci_event_packet_get_type(packet);
    switch(event_type)
    {
    case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING)
        {
            return;
        }
        gap_local_bd_addr(local_addr);
        printf("BLE up and running on %s.\n", bd_addr_to_str(local_addr));

        /* setup advertisements */
        uint16_t adv_int_min = 800;
        uint16_t adv_int_max = 800;
        uint8_t adv_type = 0;
        bd_addr_t null_addr;
        memset(null_addr, 0, 6);
        gap_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
        assert(adv_data_len <= 31); // ble limitation
        gap_advertisements_set_data(adv_data_len, (uint8_t*) adv_data);
        gap_advertisements_enable(1);

        break;
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        printf("BLE Disconnected\n");
        break;
    case ATT_EVENT_CONNECTED:
        printf("BLE Connected\n");
        le_notification_enabled = 0;
        break;
    case ATT_EVENT_CAN_SEND_NOW:
        att_server_notify(con_handle, ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE, (uint8_t*)&data_to_send, sizeof(data_to_send));
        break;
    default:
        break;
    }
}

static uint16_t att_read_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t * buffer, uint16_t buffer_size)
{
    UNUSED(connection_handle);

    if (att_handle == ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_VALUE_HANDLE)
    {
        printf("Attribute read callback\n");
        return att_read_callback_handle_blob((const uint8_t *)&data_to_send, sizeof(data_to_send), offset, buffer, buffer_size);
    }
    return 0;
}

static int att_write_callback(hci_con_handle_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer, uint16_t buffer_size)
{
    UNUSED(transaction_mode);
    UNUSED(offset);
    UNUSED(buffer_size);

    if (att_handle != ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_TEMPERATURE_01_CLIENT_CONFIGURATION_HANDLE)
    {
        return 0;
    }

    le_notification_enabled = little_endian_read_16(buffer, 0) == GATT_CLIENT_CHARACTERISTICS_CONFIGURATION_NOTIFICATION;
    con_handle = connection_handle;
    if (le_notification_enabled)
    {
        printf("Notifications enabled (can send data). \n");
        att_server_request_can_send_now_event(con_handle);
    }
    else
    {
        printf("Notifications disabled. \n");
    }
    return 0;
}

static btstack_packet_callback_registration_t hci_event_callback_registration;

static void callback_toggle_led(void)   /* called every second */
{
    /* Invert the led */
    static int led_on = true;
    led_on = !led_on;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}

static btstack_timer_source_t heartbeat;

static void heartbeat_handler(struct btstack_timer_source *ts)
{
    callback_toggle_led();
    /* restart timer */
    btstack_run_loop_set_timer(ts, HEARTBEAT_PERIOD_MS);
    btstack_run_loop_add_timer(ts);
}

static void ble_server_setup(void)
{
    l2cap_init(); /* Set up L2CAP and register L2CAP with HCI layer */
    sm_init(); /* setup security manager */
    att_server_init(profile_data, att_read_callback, att_write_callback); /* setup attribute callbacks */

    /* inform about BTstack state */
    hci_event_callback_registration.callback = &packet_handler; /* setup callback for events */
    hci_add_event_handler(&hci_event_callback_registration); /* register callback handler */

    /* register for ATT event */
    att_server_register_packet_handler(packet_handler); /* register packet handler */

    /* setup timer */
    heartbeat.process = &heartbeat_handler;
    btstack_run_loop_set_timer(&heartbeat, HEARTBEAT_PERIOD_MS);
    btstack_run_loop_add_timer(&heartbeat);
    hci_power_control(HCI_POWER_ON); /* turn BLE on */
}

static void ble_server_task(void *pv)
{
    /* initialize CYW43 driver architecture (will enable BT if/because CYW43_ENABLE_BLUETOOTH == 1) */
    if (cyw43_arch_init())
    {
        printf("failed to initialize cyw43_arch\n");
        while(1) {}
    }

    ble_server_setup();

    while(1)
    {
        btstack_run_loop_execute(); /* does not return */
    }
}

void ble_server_init(void)
{
    if (xTaskCreate(
                ble_server_task,  /* pointer to the task */
                "BLEserver", /* task name for kernel awareness debugging */
                4096, /* task stack size */
                (void*)NULL, /* optional task startup argument */
                tskIDLE_PRIORITY+2,  /* initial priority */
                (TaskHandle_t*)NULL /* optional task handle to create */
            ) != pdPASS)
    {
        printf("Failed creating ble server task. \n");
        while(1) {}
    }
}

void ble_server_send(uint16_t value)
{
    data_to_send = value;
    if (le_notification_enabled)
    {
        att_server_request_can_send_now_event(con_handle);
    }
}
