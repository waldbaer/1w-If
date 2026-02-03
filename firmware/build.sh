#!/bin/bash
number_of_cpu_cores=$(nproc --all)
echo "---- Build with ${number_of_cpu_cores} cores ----"

pio run -j${number_of_cpu_cores}
result=$?

if [ ${result} -eq 0 ]; then
  echo "---- Build SUCCESS ----"
else
  echo "---- Build FAILED ----"
fi
exit ${result}
