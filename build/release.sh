#!/bin/sh

d=${PWD}

if [ ! -d build.release ] ; then
    mkdir build.release
fi

# set the install dir.
id=""
if [ "$(uname)" == "Darwin" ] ; then 
    id=./../../install/mac-clang-x86_64/bin/
fi

cd build.release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${id} ../
cmake --build . --target install

#./videocapture
#./opengl_example
#./api_example
./easy_opengl_example
