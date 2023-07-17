# rpi-pico

Example application that uses free-RTOS.
3 tasks are created:
- LED : blinks led
- RDFI: reads rfid tag from RFID Tag Reader connected via uart
- TEMP: reads temperature from the internal temperature sensor

## Setup

Clone the repository: `git clone git@github.com:mprse/rpi-pico.git`

Go to the `rpi-pico` directory: `cd rpi-pico`

Init all submodules: `git submodule update --init --recursive`

Create build directory:

```
cd app_freertos
mkdir build
cd build
```

Build the application:

```
cmake -DPICO_BOARD=pico_w -DPICO_SDK_PATH=<path_to_pico_sdk> ..
make
```

Program rpi-pico_w using `app_freertos.u2f`.

Example application prints to `uart1`, pin `16`.

`PC <-usb-> FT232 <-pin 16, GND-> RPI PICO_W`

Open terminal and configure serial connection:
- baudrate `115200`
- no parity bit
- `1` stop bit

You should be able to see the temperature read from the temp sensor.
