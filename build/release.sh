#!/bin/sh

if [ ! -d build.release ] ; then
    mkdir build.release
fi

cd build.release
cmake -DCMAKE_BUILD_TYPE=Release ../
cmake --build .
#./videocapture
./opengl_example
