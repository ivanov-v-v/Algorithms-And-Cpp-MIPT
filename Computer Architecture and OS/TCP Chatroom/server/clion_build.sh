#!/bin/bash
# run as "$ . build.sh"

rm -rf build
mkdir cmake-build-debug
cd ./cmake-build-debug
cmake ..
make &> error_log.txt
subl error_log.txt

