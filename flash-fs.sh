#!/bin/bash
#
# Flash filesystem
# ----------------------------------------------------------------------------------------------------------------------
echo "---- Build ----"
pio run --target upload
pio run --target uploadfs
echo "---- Build DONE ----"
