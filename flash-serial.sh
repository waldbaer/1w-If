#!/bin/bash
echo "---- Flash Firmware (serial) ----"

pio run --target upload -e wt32-eth01_esptool
result=$?

if [ ${result} -eq 0 ]; then
  echo "---- Flash SUCCESS ----"
else
  echo "---- Flash FAILED ----"
fi
exit ${result}
