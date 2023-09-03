#!/bin/bash

./stop.sh

pidfile="pidfile.txt"

if [ ! -s "$pidfile" ]; then
    nohup ./main & echo $! > "$pidfile"
fi