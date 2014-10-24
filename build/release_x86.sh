#!/bin/sh

set -x

. ./set_variables.sh

cd build.release

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_OPENGL=On \
    -DUSE_GENERATE_X86=On \
    -DUSE_GENERATE_IPHONE=Off \
    -DUSE_GENERATE_IPHONE_SIMULATOR=Off \
    -DUSE_GENERATE_RPI=Off \
    -DUSE_DECKLINK=Off \
    -DEXTERN_LIB_DIR=${extern_path}/lib \
    -DEXTERN_INC_DIR=${extern_path}/include \
    -DEXTERN_SRC_DIR=${extern_path}/src \
    -DTINYLIB_DIR=${d}/sources/tinylib/ \
    -DCMAKE_INSTALL_PREFIX=${install_path} \
    ../

cmake --build . --target install --config Release

cd ${install_path}/bin

./easy_opengl_example

#./videocapture
#./opengl_example
#./api_example
#./easy_opengl_example
#./api_example
#./decklink_example


