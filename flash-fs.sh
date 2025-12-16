#!/bin/bash
#
# Flash filesystem
# ----------------------------------------------------------------------------------------------------------------------
echo "---- Flash Firmware & Filesystem (ota) ----"

pio run --target upload
result=$?

if [[ ${result} -eq 0 ]]; then
  pio run --target uploadfs
  result_fs=$?
fi

if [[ ${result} -eq 0 && ${result_fs} -eq 0 ]]; then
  echo "---- Flash SUCCESS ----"
  exit 0
else
  echo "---- Flash FAILED ----"
  exit 1
fi
