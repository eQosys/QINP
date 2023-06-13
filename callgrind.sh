#!/bin/bash

./build.sh RelWithDebInfo

valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes --callgrind-out-file=/tmp/callgrind-qinp-out ./bin/RelWithDebInfo/QINP -v -k -r -e=examples/_symbols.json -c=examples/_comments.json -i=stdlib/ -o=/tmp/qinp-out.out examples/test.qnp

kcachegrind /tmp/callgrind-qinp-out