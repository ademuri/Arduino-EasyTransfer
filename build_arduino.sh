#!/bin/bash

set -euo pipefail

if [ "${1-}" == "-h" ] || [ "${1-}" == "--help" ]; then
  echo "usage: build_arduino.sh <board>"
  exit 0
fi

BOARDS="--board teensy31 --board adafruit_feather_m0 --board esp32dev --board uno"
if [ "${1-}" != "" ]; then
  BOARDS="$1"
fi


for dir in examples/*; do
  platformio ci ${BOARDS?} --lib=. "${dir?}"
done
