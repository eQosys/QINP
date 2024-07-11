#!/bin/bash

clear
./build.sh Debug && QINP_STDLIB=./stdlib/ ./bin/Debug/qinp "$@"