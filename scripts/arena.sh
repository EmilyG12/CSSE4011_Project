#!/usr/bin/env bash

west build -b nrf52840dk/nrf52840 app -d build-arena -DMY_UUID="0xe1, 0xfd, 0x11, 0xdc" && west flash -d build-arena && screen /dev/tty.usbmodem0010502333861 115200