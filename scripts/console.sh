#!/usr/bin/env bash

west build -b m5stack_core2/esp32/procpu app -d build-m5 -DMY_UUID="0xc1, 0xff, 0x23, 0x7b" && west flash -d build-m5 --esp-baud-rate 460800 && screen /dev/tty.usbserial-54240316031 115200