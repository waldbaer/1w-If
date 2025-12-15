#!/bin/bash
#
# Flash filesystem
# ----------------------------------------------------------------------------------------------------------------------
echo "---- Flash Firmware & Filesystem (serial) ----"
pio run --target upload -e wt32-eth01_esptool
pio run --target uploadfs -e wt32-eth01_esptool
echo "---- Flash DONE ----"
