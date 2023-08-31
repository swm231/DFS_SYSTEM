#!/bin/bash

pidfile="pidfile.txt"

if [ -s "$pidfile" ]; then
    kill -9 "$(cat $pidfile)"
    > "$pidfile"
fi