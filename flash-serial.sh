#!/bin/bash
echo "---- Flash Firmware (serial) ----"
pio run --target upload -e wt32-eth01_esptool
echo "---- Flash DONE ----"
