# LogicAnalyzer-SWD-Debugger

## Teensy Development Envirnoment

https://www.pjrc.com/teensy/td_download.html

## Update Submodule

```bash
git submodule update --init
```

## compiler

```bash
sudo apt update
sudo apt install gcc-arm-none-eabi
```

## Simulate

```bash
sudo apt install qemu-system
```

## WSL2 connect USB

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

## CMSIS

https://github.com/ARM-software/CMSIS_6/tree/main
