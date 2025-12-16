#!/bin/bash
#
# Flash filesystem
# ----------------------------------------------------------------------------------------------------------------------
echo "---- Flash Firmware & Filesystem (serial) ----"

pio run --target upload -e wt32-eth01_esptool
result=$?

if [[ ${result} -eq 0 ]]; then
  pio run --target uploadfs -e wt32-eth01_esptool
  result_fs=$?
fi

if [[ ${result} -eq 0 && ${result_fs} -eq 0 ]]; then
  echo "---- Flash SUCCESS ----"
  exit 0
else
  echo "---- Flash FAILED ----"
  exit 1
fi
