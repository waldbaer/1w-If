#!/bin/bash
echo "---- Flash Firmware (ota) ----"

pio run --target upload
result=$?

if [ ${result} -eq 0 ]; then
  echo "---- Flash SUCCESS ----"
else
  echo "---- Flash FAILED ----"
fi
exit ${result}
