#!/bin/bash
echo "---- Monitor (websocket) ----"
curl --no-buffer -N -v ws://owif
echo "---- Monitoring DONE ----"
