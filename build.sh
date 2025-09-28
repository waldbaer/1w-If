#!/bin/bash
number_of_cpu_cores=$(nproc --all)
echo "---- Build with ${number_of_cpu_cores} cores ----"
pio run -j${number_of_cpu_cores}
echo "---- Build DONE ----"
