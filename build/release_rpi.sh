#!/bin/sh

set -x

. ./set_variables.sh

cd build.release

triplet="armv6-rpi-linux-gnueabi"
extern_path=${d}/../extern/${triplet}
install_path=${d}/../install/${triplet}
target=armv6-rpi-linux-gnueabi

export PATH=${HOME}/x-tools/${target}/bin:${PATH}

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${install_path} \
    -DCMAKE_C_COMPILER=${target}-gcc \
    -DCMAKE_CXX_COMPILER=${target}-g++ \
    -DUSE_OPENGL=Off \
    -DUSE_GENERATE_X86=Off \
    -DUSE_GENERATE_IPHONE=Off \
    -DUSE_GENERATE_IPHONE_SIMULATOR=Off \
    -DUSE_GENERATE_RPI=On \
    -DUSE_DECKLINK=Off \
    -DEXTERN_LIB_DIR=${extern_path}/lib \
    -DEXTERN_INC_DIR=${extern_path}/include \
    -DEXTERN_SRC_DIR=${extern_path}/src \
    -DTINYLIB_DIR=${d}/sources/tinylib/ \
    ../

cmake --build . --target install --config Release

cd ${install_path}/bin

#./api_example


