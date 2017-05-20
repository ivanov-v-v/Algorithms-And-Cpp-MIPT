#!/bin/bash
# run as "$ . build.sh"

mkdir cmake-build-debug
cd ./cmake-build-debug
cmake ..
make &> error_log.txt

