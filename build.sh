#!/bin/bash

cmake -B "build/`uname`" -S .
cd "build/`uname`"
make -j8
gdb -ex run ./SphericImageProjector