#!/bin/bash

cmake -B "build/`uname`" -S .
cd "build/`uname`"
make -j8
cd Release
gdb -ex run ./SphericImageProjector
#./SphericImageProjector