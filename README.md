# Teensy 4.1 SWD Debugger

This project implements a CMSIS-DAP compliant SWD debugger using a Teensy 4.1 board. It leverages FreeRTOS for task management and the official ARM CMSIS-DAP library to provide debugging capabilities for ARM Cortex-M microcontrollers.

## Overview

- **Platform**: Teensy 4.1 (NXP i.MX RT1062)
- **Firmware Base**: FreeRTOS Kernel
- **Protocol**: CMSIS-DAP (v2)
- **Interface**: USB High Speed (480 Mbit/s)

## Dependencies & Submodules

This project relies on several key submodules:

- [Cores](https://github.com/PaulStoffregen/cores): Teensy 4.1 Board Support Package (BSP).
- [FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel): Real-time operating system kernel.
- [CMSIS-DAP](https://github.com/ARM-software/CMSIS-DAP): CMSIS-DAP debugger firmware source.
- [CMSIS_6](https://github.com/ARM-software/CMSIS_6): CMSIS Core headers.

## Hardware Configuration

### SWD Pinout

The SWD interface pins are configured in `device/inc/DAP_config.h`. The default configuration is:

| Signal | Teensy Pin | Description |
| :--- | :--- | :--- |
| **SWDIO** | 5 | Serial Wire Data Input/Output |
| **SWCLK** | 7 | Serial Wire Clock |
| **LED** | 13 | Status LED (Running) |

To modify these assignments, edit the macros in `device/inc/DAP_config.h`:

```c
#define PIN_SWDIO       5U
#define PIN_SWCLK       7U
#define PIN_LED_RUNNING 13U
```

## Build Project

## environment

```sh
sudo apt update
sudo apt install gcc-arm-none-eabi
```

### update submodule

```sh
git submodule update --init
```

### build

```sh
make
```

## Upload Firmware to Device

Please follow https://www.pjrc.com/teensy/loader.html.

## Connect USB to WSL2

Ref: https://learn.microsoft.com/zh-tw/windows/wsl/connect-usb

### Windows

```shell
usbipd list
usbipd bind --busid <busid>
```

```shell
usbipd attach --wsl --busid <busid>
```

```shell
usbipd detach --busid <busid>
```

### WSL

```shell
lsusb
```

## License

This project is licensed under the Apache License 2.0. See the [LICENSE](./LICENSE) file for details.
