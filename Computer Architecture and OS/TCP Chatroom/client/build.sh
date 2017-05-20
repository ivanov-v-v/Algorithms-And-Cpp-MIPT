#!/bin/bash
# run as "$ . build.sh"

mkdir build
cd ./build
cmake ..
make &> error_log.txt

