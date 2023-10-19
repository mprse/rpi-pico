# -*- coding: utf-8 -*-
"""
Notifications
-------------

This module enables notifications from a BLE device acting as a game pad.
Incomming commands are used to simulate key presses.

"""

import argparse
import asyncio
import logging
import pyautogui

from bleak import BleakClient, BleakScanner
from bleak.backends.characteristic import BleakGATTCharacteristic

BLE_CONNECTING = 1
BLE_CONNECTED = 2
BLE_DISCONNECTED = 3
BLE_ERROR = 4

class BLE_PAD:
    def __init__(self, status_queue, cmd_queue) -> None:
        self.DEVICE_ADDRESS = "28:CD:C1:09:6B:A4"
        self.DEVICE_PAD_UUID = "00002a6e-0000-1000-8000-00805f9b34fb"
        self.running = False
        self.logger = logging.getLogger(__name__)
        log_level = logging.DEBUG
        logging.basicConfig(
            level=log_level,
            format="%(asctime)-15s %(name)-8s %(levelname)s: %(message)s",
        )
        self.status_queue = status_queue
        self.cmd_queue = cmd_queue
        self.status_queue.put(BLE_DISCONNECTED)

    # handle disconnection
    def disconnect_callback(self, client: BleakClient):
        self.status_queue.put(BLE_DISCONNECTED)
        self.running = False
        print("BLE: disconnected")

    def notification_handler(self, characteristic: BleakGATTCharacteristic, data: bytearray):
        """Simple notification handler which prints the data received."""
        #print("%s: %d", characteristic.description, data[0])
        if data[1] == 0x99:
            if data[0] == 1:
                pyautogui.keyDown('down')
            elif data[0] == 2:
                pyautogui.keyDown('up')
            elif data[0] == 3:
                pyautogui.keyDown('right')
            elif data[0] == 4:
                pyautogui.keyDown('left')
            elif data[0] == 5:
                pyautogui.keyDown('pause')

        if data[1] == 0x88:
            if data[0] == 1:
                pyautogui.keyUp('down')
            elif data[0] == 2:
                pyautogui.keyUp('up')
            elif data[0] == 3:
                pyautogui.keyUp('right')
            elif data[0] == 4:
                pyautogui.keyUp('left')
            elif data[0] == 5:
                pyautogui.keyUp('pause')

    def stop(self) -> None:
        self.running = False

    def start(self) -> None:
        self.running = True
        asyncio.run(self.run())

    def is_running(self) -> bool:
        #print("BLE: is_running: ", self.running)
        return self.running

    async def run(self):
        #print("BLE: starting BLE scan...")

        device = None

        while(device == None and self.running):
            device = await BleakScanner.find_device_by_address(
                self.DEVICE_ADDRESS, cb=dict(use_bdaddr = True)
            )
            if(self.cmd_queue.empty() == False):
                cmd = self.cmd_queue.get()
                if cmd == "stop":
                    self.stop()


        if device is None:
            #print("BLE: Could not find device with address '%s'", self.DEVICE_ADDRESS)
            #self.status_queue.put(BLE_ERROR)
            return

        print("BLE: connecting to BLE device...")
        self.status_queue.put(BLE_CONNECTING)

        async with BleakClient(device, disconnected_callback=self.disconnect_callback) as client:
            print("BLE: Connected")
            self.status_queue.put(BLE_CONNECTED)
            # send ack to server

            await client.start_notify(self.DEVICE_PAD_UUID, self.notification_handler)
            # sleep forever
            while(self.running):
                await asyncio.sleep(1)
                if(self.cmd_queue.empty() == False):
                    cmd = self.cmd_queue.get()
                    if cmd == "stop":
                        self.stop()
                    else:
                        # convert int to byte array
                        score = int(cmd)
                        score_bytes = score.to_bytes(4, byteorder='little')
                        # add 0xAB as first byte (score marker)
                        score_bytes = bytearray([0xAB]) + score_bytes
                        await client.write_gatt_char(self.DEVICE_PAD_UUID, score_bytes, response=True)

            print("BLE: Bye Bye")
            if client.is_connected:
                await client.stop_notify(self.DEVICE_PAD_UUID)
