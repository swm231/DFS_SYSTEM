#!/bin/bash

mkdir build
cd build
rm * -r
cmake ..
make
../bin/start.sh