#!/bin/bash

./stop.sh
cd build
make
cd ../bin
./start.sh