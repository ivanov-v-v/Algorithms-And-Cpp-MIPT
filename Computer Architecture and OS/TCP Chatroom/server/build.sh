#!/bin/bash
# run as "$ . build.sh"

rm -rf build
mkdir build
cd ./build
cmake ..
make &> error_log.txt
subl error_log.txt

