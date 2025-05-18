#!/usr/bin/env bash

west build -b m5stack_core2/esp32/procpu app -d build-m5
west flash -d build-m5 --esp-baud-rate 460800

west build -b nrf52840dk/nrf52840 app -d build-arena
west flash -d build-arena