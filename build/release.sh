#!/bin/sh

if [ ! -d build.release ] ; then
    mkdir build.release
fi

cd build.release
cmake -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --target install
#./videocapture
#./opengl_example
./easy_opengl_example
