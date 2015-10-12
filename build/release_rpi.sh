#!/bin/sh

set -x
d=${PWD}
triplet="arm-linux-gnueabihf"
extern_path=${d}/../extern/${triplet}
install_path=${d}/../install/${triplet}

if [ ! -d build.release.rpi ] ; then
    mkdir build.release.rpi
fi

cd build.release.rpi

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=${install_path} \
    -DCMAKE_TOOLCHAIN_FILE=${d}/ToolchainArm.cmake \
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



