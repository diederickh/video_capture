#!/bin/bash
#set -x

# ----------------------------------------------------------------------- #
#                                I N F O 
# ----------------------------------------------------------------------- #
#
#  ********************************************************************** *
#
#  Get the latest version of this file from: 
#
#        https://gist.github.com/roxlu/1322204eabbd5d42c2d0
#
#  ********************************************************************** *
# This script can compile several libraries on Windows. We try to keep 
# everything as simple as possible in here, no functions, macros etc..  
#
# You need to install a couple of dependencies before we can use this 
# build script. 
# 
#   - NASM:                            install into ${PWD}/tools/nasm
#   - ActiveState perl for windows:    install into ${PWD}/tools/perl
# 
# ----------------------------------------------------------------------- #
#               B U I L D    S E T T I N G S 
# ----------------------------------------------------------------------- #

build_libuv=n
build_yasm=n
build_x264=n            # needs yasm, perl
build_videogenerator=n
build_lame=n
build_openssl=n
build_portaudio=n
build_libyuv=n
build_videocapture=n
build_rapidxml=n
build_glad=y
build_tinylib=y
build_glfw=y

# ----------------------------------------------------------------------- #
#                E N V I R O N M E N T  V A R I A B L E S 
# ----------------------------------------------------------------------- #

d=${PWD}
sd=${d}/sources
bd=${d}/../extern/win-vs2012-x86_64/
id=${d}/install/
perl_path=${d}/tools/perl/bin/
nasm_path=${d}/tools/nasm/
cygw_path=${d}/tools/cygwin/
msys_path=${d}/tools/msys/
prog_path="c:\\Program Files (x86)"
vs_path="${prog_path}\\Microsoft Visual Studio 12.0"
sd_win=$(echo "${sd}" | sed 's/^\///' | sed 's/\//\\/g' | sed 's/^./\0:/')
bd_win=$(echo "${bd}" | sed 's/^\///' | sed 's/\//\\/g' | sed 's/^./\0:/')
git_bash="${prog_path}\\Git\\bin\\sh.exe"

# Get home dir. 
cd ~
homedir=${PWD}
cd ${d}

if [ "${build_x264}" = "y" ] || [ "${build_openssl}" == "y" ] ; then 

    # Make sure we have perl installed.
    if [ ! -d ${perl_path} ] ; then
        echo "Please install ActiveState Perl in ${sd}/tools/perl"
        exit
    fi

    # Make sure we have nasm.
    if [ ! -d ${nasm_path} ] ; then
        echo "Please install nasm in ${sd}/tools/nasm"
        exit
    fi
    
    # Make sure we have cygwin
    if [ ! -d ${cygw_path} ] ; then
        echo "Please install cygwin into ${sd}/tools/cygwin"
        exit
    fi
fi 

export PATH=${cygw_path}/bin/:${perl_path}:${nasm_path}:${PATH}:${bd}/bin/:${sd}/gyp/

# ----------------------------------------------------------------------- #
#                          F U N C T I O N S  
# ----------------------------------------------------------------------- #

# download [dirname] [filename] [url]
function download() {
    name=${1}
    dirname=${2}
    url=${3}
    filename=$(basename "$url")
    extension="${filename##*.}"
    filename="${filename%.*}"
    
    if [ ! -d ${sd}/downloads ] ; then
        mkdir ${sd}/downloads
    fi
    if [ ! -d ${sd}/${name} ] ; then
        cd ${sd}
        if [ "${extension}" == "gz" ] || [ "${extension}" == "tgz" ] ; then
            if [ -f ${sd}/downloads/${name}.tar.gz ] ; then
                cp ${sd}/downloads/${name}.tar.gz ${sd}/
            else
                curl -o ${name}.tar.gz -L ${url}
            fi
            tar -zxvf ${name}.tar.gz
            mv ${dirname} ${name}
            if [ ! -f ${sd}/downloads/${name}.tar.gz ] ; then
                mv ${sd}/${name}.tar.gz ${sd}/downloads
            else
                rm ${sd}/${name}.tar.gz
            fi
        fi
    fi
}

# compile [dirname] [existcheck] [extraoptions]
function compile() {
    name=${1}
    installfile=${2}
    options=${3}
    if [ "${installfile}" = "" ] ; then
        cd ${sd}/${name}
        ./configure --prefix=${bd} ${options}
        make clean
        make
        make install
    elif [ ! -f ${bd}/${installfile} ] ; then
        cd ${sd}/${name}
        ./configure --prefix=${bd} ${options}
        make clean
        make
        make install
    fi
}

# ----------------------------------------------------------------------- #
#                 C H E C K   I F   U T I L S   E X I S T
# ----------------------------------------------------------------------- #
if ! [ -x "$(command -v python)" ] ; then
    echo ""
    echo ""
    echo "-------------------------------------------------"
    echo "It seems that you haven't installed Python"
    echo "or that it's not in your PATH variable, install"
    echo "python or fix your PATH variable first."
    echo "-------------------------------------------------"
    echo ""
    echo ""
    exit
fi


# ----------------------------------------------------------------------- #
#                D O W N L O A D   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #
if [ ! -d ${sd} ] ; then 
    mkdir -p ${sd}
fi

if [ ! -d ${bd} ] ; then
    mkdir -p ${bd}
fi

if [ ! -d ${bd}/bin ] ; then
    mkdir -p ${bd}/bin
fi

if [ ! -d ${bd}/src ] ; then 
    mkdir -p ${bd}/src
fi

if [ ! -d ${bd}/include ] ; then 
    mkdir -p ${bd}/include
fi

# Download the cmakefiles
if [ ! -d ${sd}/cmakefiles ] ; then
    mkdir ${sd}/cmakefiles
    cd ${sd}/cmakefiles
    git clone git@bitbucket.org:roxlu/cmake.git .
fi

# Download libuv
if [ "${build_libuv}" = "y" ] ; then 
    if [ ! -d ${sd}/libuv ] ; then
        cd ${sd}
        git clone https://github.com/joyent/libuv.git libuv
    fi

    # Download gyp for libuv
    if [ ! -d ${sd}/libuv/build/gyp ] ; then 
        cd ${sd}/libuv
        git clone https://git.chromium.org/external/gyp.git build/gyp
    fi

    # Copy the cmake file.
    cp ${sd}/cmakefiles/uv/CMakeLists.txt ${sd}/libuv/
fi

# Download yasm, needed for libvpx, x264
if [ "${build_yasm}" = "y" ] ; then
    if [ ! -f ${bd}/bin/yasm.exe ] ; then
        cd ${bd}/bin
        curl -o yasm.exe http://www.tortall.net/projects/yasm/releases/yasm-1.2.0-win64.exe
    fi
fi

# Download x264
if [ "${build_x264}" = "y" ] ; then
    if [ ! -d ${sd}/x264 ] ; then 
        cd ${sd}
        git clone --depth 1 --branch master git://git.videolan.org/x264.git
    fi

    # Copy the cmake file.
    if [ ! -f ${sd}/x264/CMakeLists.txt ] ; then
        cp ${sd}/cmakefiles/x264/CMakeLists.txt ${sd}/x264/
    fi
fi

# Download the video generator lib
if [ "${build_videogenerator}" = "y" ] ; then
    if [ ! -d ${sd}/video_generator ] ; then
        cd ${sd}
        git clone --depth 1 --branch master git@github.com:roxlu/video_generator.git
    fi
fi

# Download openssl
if [ "${build_openssl}" = "y" ] ; then
    if [ ! -d ${sd}/openssl ] ; then 
        cd ${sd}
        curl -o ssl.tar.gz https://www.openssl.org/source/openssl-1.0.1i.tar.gz
        tar -zxvf ssl.tar.gz
        mv openssl-1.0.1i openssl
    fi
fi

# Download liblame mp3 encoder
if [ "${build_lame}" = "y" ] ; then
    download lame lame-3.99.5 http://downloads.sourceforge.net/project/lame/lame/3.99/lame-3.99.5.tar.gz
fi

# Download portaudio
if [ "${build_portaudio}" = "y" ] ; then
    download portaudio pa_stable http://www.portaudio.com/archives/pa_stable_v19_20140130.tgz
fi

# Download rapidxml
if [ "${build_rapidxml}" = "y" ] ; then
    if [ ! -d ${sd}/rapidxml ] ; then 
        cd ${sd}
        curl -o rapidxml.zip -L "https://sourceforge.net/projects/rapidxml/files/rapidxml/rapidxml%201.13/rapidxml-1.13.zip/download"
        unzip rapidxml.zip
        mv rapidxml-1.13 rapidxml
    fi 
fi

# Download libyuv
if [ "${build_libyuv}" = "y" ] ; then
    if [ ! -d ${sd}/libyuv ] ; then 
        mkdir ${sd}/libyuv
        cd ${sd}/libyuv
        svn checkout http://libyuv.googlecode.com/svn/trunk/ .
        mv ${sd}/libyuv/CMakeLists.txt ${sd}/libyuv/CMakeLists.txt.bak
        cp ${sd}/cmakefiles/yuv/CMakeLists.txt ${sd}/libyuv/
    fi
fi

# Download video capture library
if [ "${build_videocapture}" = "y" ] ; then
    if [ ! -d ${sd}/video_capture ] ; then
        mkdir ${sd}//video_capture
        cd ${sd}/video_capture
        git clone http://github.com/roxlu/video_capture.git .
    fi
fi

# Download GLAD for GL
if [ "${build_glad}" = "y" ] ; then
    if [ ! -d ${sd}/glad ] ; then 
        cd ${sd}
        git clone --depth 1 --branch master https://github.com/Dav1dde/glad.git glad
    fi
fi

# Download the tinylib 
if [ "${build_tinylib}" = "y" ] ; then
    if [ ! -d ${sd}/tinylib ] ; then 
        mkdir ${sd}/tinylib
        cd ${sd}/tinylib
        git clone https://github.com/roxlu/tinylib.git .
    fi
fi

# Download GLFW for GL
if [ "${build_glfw}" = "y" ] ; then
    if [ ! -d ${sd}/glfw ] ; then 
        cd ${sd}
        git clone --depth 1 --branch master https://github.com/glfw/glfw.git glfw
    fi
fi

# ----------------------------------------------------------------------- #
#                C O M P I L E   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #

# Compile libuv
if [ "${build_libuv}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libuv.lib ] ; then
        cd ${sd}/libuv
        
        mkdir build.release 
        cd build.release
        cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${bd} ../
        cmake --build . --target install 
    fi
fi

# Compile openssl 
if [ "${build_openssl}" = "y" ] ; then
    if [ ! -f ${bd}/bin/libeay32.dll ] ; then 
        cd ${sd}/openssl

        if [ -f tmp.bat ] ; then
            rm tmp.bat
        fi    

        #perl Configure VC-WIN32 --prefix=${bd}

        echo "@echo off" >> tmp.bat
        echo "cd ${vs_path}\\VC\\bin\\x86_amd64\\" >> tmp.bat
        echo "call vcvarsx86_amd64.bat" >> tmp.bat
        echo "cd ${sd_win}\\openssl" >> tmp.bat
        echo "perl Configure VC-WIN64A --prefix=installed" >> tmp.bat
        echo "cd ${sd_win}\\openssl" >> tmp.bat
        echo "call ms\\do_win64a.bat" >> tmp.bat
        echo "nmake -f ${sd_win}\\openssl\\ms\\ntdll.mak" >> tmp.bat
        echo "nmake -f ${sd_win}\\openssl\\ms\\ntdll.mak install" >> tmp.bat
        cmd /k "${sd}/openssl/tmp.bat"
        cp -r ${sd}/openssl/installed/include/openssl ${bd}/include/
        cp -r ${sd}/openssl/installed/lib/*.lib ${bd}/lib/
        cp -r ${sd}/openssl/installed/lib/engines ${bd}/lib/
        cp -r ${sd}/openssl/installed/lib/engines ${bd}/lib/
        cp -r ${sd}/openssl/installed/bin/*.* ${bd}/bin/
   fi
fi

# Compile x264
if [ "${build_x264}" = "y" ] ; then 
    cd ${sd}/x264
    if [ ! -f ${bd}/lib/libx264.lib ] ; then
        cd ${sd}/x264
        
        mkdir build.release 
        cd build.release
        cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${bd} ../
        cmake --build . --config Release --target install
    fi
fi

# Compile the video generator
if [ "${build_videogenerator}" = "y" ] ; then
    if [ ! -f ${bd}/lib/videogenerator.lib ] ; then
        cd ${sd}/video_generator
        cd build
        if [ ! -d compiled ] ; then
            mkdir compiled
        fi
        cd compiled
        cmake -DCMAKE_INSTALL_PREFIX=${bd} -DCMAKE_BUILD_TYPE=Release ../
        cmake --build . --config Release --target install
    fi
fi

# Compile liblame
if [ "${build_lame}" = "y" ] ; then
    if [ ! -f ${bd}/lib/libmp3lame.lib ] ; then 
        cd ${sd}/lame
        if [ -d ${sd}/lame/build ] ; then
            rm -r build
            mkdir build
        fi

        cd $build
        cmake -DCMAKE_INSTALL_PREFIX=${bd} -DCMAKE_BUILD_TYPE=Release ../
        cmake --build . --config Release --target install
    fi
fi

# Compile portaudiox
if [ "${build_portaudio}" = "y" ] ; then
    if [ ! -f ${bd}/lib/portaudio_static_x86.lib ] ; then 
        cd ${sd}/portaudio
        if [ -d ${sd}/portaudio/build ] ; then
            rm -r ${sd}/portaudio/build
            mkdir ${sd}/portaudio/build
        fi
        cd ${sd}/portaudio/build
        cmake -DCMAKE_INSTALL_PREFIX=${bd} -DPA_DLL_LINK_WITH_STATIC_RUNTIME=True ../
        cmake --build . --config Release
        
        cp Release/portaudio_static_x86.lib ${bd}/lib/
        cp ${sd}/portaudio/include/portaudio.h ${bd}/include/
   fi
fi

# Compile video capture
if [ "${build_videocapture}" = "y" ] ; then
    if [ ! -f ${bd}/lib/videocapture.libs ] ; then
        cd ${sd}/video_capture/build
        if [ -d ${sd}/video_capture/build/build.release ] ; then
            rm -r build.release
        fi
        mkdir build.release
        cd build.release
        cmake -DCMAKE_INSTALL_PREFIX=${bd} -DCMAKE_BUILD_TYPE=Release -DUSE_OPENGL=False ..
        cmake --build . --target install --config Release
    fi
fi

# Compile libyuv
if [ "${build_libyuv}" = "y" ] ; then
    if [ ! -f ${bd}/lib/yuv.lib ] ; then 
        if [ -d ${sd}/libyuv/build ] ; then
            rm -rf ${sd}/libyuv/build 
        fi
        mkdir ${sd}/libyuv/build

        cd ${sd}/libyuv
        cd build

        cmake -DCMAKE_INSTALL_PREFIX=${bd} -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . --target install --config Release
   fi
fi

# Move rapid xml sources 
if [ "${build_rapidxml}" = "y" ] ; then
    if [ ! -f ${bd}/include/rapidxml_iterators.hpp ] ; then
        cd ${sd}/rapidxml
        cp rapidxml_iterators.hpp ${bd}/include/
        cp rapidxml_print.hpp ${bd}/include/
        cp rapidxml_utils.hpp ${bd}/include/
        cp rapidxml.hpp ${bd}/include/
    fi
fi

# Copy the GLAD sources + generate the C extension
if [ "${build_glad}" = "y" ] ; then 
    if [ ! -f ${bd}/src/glad.c ] ; then
        if [ ! -d ${bd}/src ] ; then
            mkdir ${bd}/src 
        fi
        cd ${sd}/glad
        python main.py --generator=c --out-path=gl --extensions GL_ARB_timer_query,GL_APPLE_rgb_422

        cp -r ${sd}/glad/gl/include/glad ${bd}/include/
        cp -r ${sd}/glad/gl/include/KHR ${bd}/include/
        cp ${sd}/glad/gl/src/glad.c ${bd}/src/
    fi
fi

# Compile glfw
if [ "${build_glfw}" = "y" ] ; then
    if [ ! -f ${bd}/lib/libglfw3.a ] ; then
        cd ${sd}/glfw
        if [ -d build ] ; then 
            rm -r build
        fi
        if [ ! -d build ] ; then
            mkdir build
        fi

        cfcopy=${CFLAGS}
        ldcopy=${LDFLAGS}
        export CFLAGS=""
        export LDFLAGS=""

        cd build
        cmake -DCMAKE_INSTALL_PREFIX=${bd} ..
        cmake --build . --target install

        export CFLAGS=${cfcopy}
        export LDFLAGS=${ldcopy}
    fi
fi
