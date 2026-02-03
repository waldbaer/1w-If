#!/bin/bash
# Monitor via websocket
#-------------------
readonly WS_URL="ws://owif"

echo "---- Monitor (websocket) ----"
curl --no-buffer -N -s "$WS_URL" | \
  sed -E 's/^\{"LOG":"//; s/"\}$//'
echo "---- Monitoring DONE ----"
