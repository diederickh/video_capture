#!/bin/bash

# ----------------------------------------------------------------------- #
#                                I N F O 
# ----------------------------------------------------------------------- #
#
#
#  ********************************************************************** *
#
#  Get the latest version of this file from: 
#
#        https://gist.github.com/roxlu/1322204eabbd5d42c2d0
#
#  ********************************************************************** *
#
#
#  This file can be used to easily setup a development environment that 
#  to libraries from a local development directory. This is very usefull when 
#  you don't want to depend on the systems version of the installed libraries
#  and make your application compile on different OSses easily.
#  
#  Select what libraries you want to compile below and execute it. 
#  
#
#
# ----------------------------------------------------------------------- #
#               B U I L D    S E T T I N G S 
# ----------------------------------------------------------------------- #

build_m4=n
build_autoconf=n        # needs an updated m4 
build_libtool=n
build_automake=n
build_pkgconfig=n
build_gtkdoc=n         
build_pixman=n
build_gettext=n
build_libxml=n
build_ffi=n              
build_fontconfig=n      # needs freetype, libxml
build_libpng=n
build_libjpg=n
build_colm=n
build_ragel=n           # needs colm
build_harfbuzz=n        # needs ragel
build_freetype=n
build_glib=n            # needs ffi 
build_cairo=n           # needs png, freetype, harfbuzz 
build_pango=n           # needs glib, ffi, gtkdoc, fontconfig, freetype, libxml, harfbuzz
build_libz=n
build_yasm=n
build_libuv=n
build_mongoose=n
build_netskeleton=n
build_sslwrapper=n
build_rapidxml=n
build_glad=y
build_glfw=y
build_tinylib=y
build_videocapture=n
build_imagemagick=n
build_graphicsmagick=n
build_libav=n
build_microprofile=n
build_ogg=n
build_theora=n
build_vorbis=n
build_rxpplayer=n       # ogg,theora,vorbis player
build_tracker=n         # openGL/openCV based video tracking
build_opencv=n
build_curl=n
build_jannson=n
build_x264=n            # needs yasm 
build_flvmeta=n         
build_videogenerator=n
build_nasm=n           
build_lame=n            # needs nasm, mp3 encoding
build_portaudio=n
build_libyuv=n           
build_nanovg=n
build_liblo=n           # needs autotools/make, OSC implementation.
build_remoxly=n         # needs tinylib

# ----------------------------------------------------------------------- #
#                E N V I R O N M E N T  V A R I A B L E S 
# ----------------------------------------------------------------------- #

set -x

is_mac=n
is_linux=n
tri_arch=""
tri_compiler=""
tri_platform=""
tri_triplet=""

if [ "$(uname)" = "Darwin" ]; then
    is_mac=y
    tri_platform="mac"
    tri_arch="x86_64"
    tri_compiler="clang"
elif [ "$(expr substr $(uname -s) 1 5)" = "Linux" ]; then
    is_linux=y
    tri_platform="linux"
    tri_arch="x86_64"
    tri_compiler="gcc"
elif [ "$(expr substr $(uname -s) 1 10)" = "MINGW32_NT" ]; then
    echo "Windows not yet supported."
    exit
fi

tri_triplet="${tri_platform}-${tri_compiler}-${tri_arch}"

d=${PWD}
sd=${d}/sources
bd=${d}/../extern/${tri_triplet}
id=${d}/../extern/${tri_triplet}/include

cflagsorig=${CFLAGS}
ldflagsorig=${LDFLAGS}
pathorig=${PATH}
export PATH=${bd}bin/:${sd}/gyp/:${PATH}
export CFLAGS="-I${bd}/include"
export LDFLAGS="-L${bd}/lib"
cfcopy=${CFLAGS}
ldcopy=${LDFLAGS}
pathcopy=${PATH}


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
    echo ${extension}
    
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
#                D O W N L O A D   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #
if [ ! -d ${sd} ] ; then 
    mkdir -p ${sd}
fi

if [ ! -d ${bd} ] ; then
    mkdir -p ${bd}
fi

if [ ! -d ${bd}/src ] ; then 
    mkdir -p ${bd}/src
fi

if [ ! -d ${bd}/include ] ; then 
    mkdir -p ${bd}/include
fi

# Download some cmake files that are used to comnpile libs that have broken build systems.
if [ ! -d ${sd}/cmake ] ; then
    mkdir ${sd}/cmake
    cd ${sd}/cmake
    git clone --depth 1 --branch master git@bitbucket.org:roxlu/cmake.git .
fi

# Download m4
if [ "${build_m4}" = "y" ] ; then
    if [ ! -d ${sd}/m4 ] ; then
        cd ${sd}
        curl -o m4.tar.gz http://ftp.gnu.org/gnu/m4/m4-1.4.17.tar.gz
        tar -zxvf m4.tar.gz 
        mv m4-1.4.17 m4
     fi
fi 

# Download autoconf and friends
if [ "${build_autoconf}" = "y" ] ; then 
    if [ ! -d ${sd}/autoconf ] ; then 
        cd ${sd}
        curl -o autoconf.tar.gz http://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
        tar -zxvf autoconf.tar.gz
        mv autoconf-2.69 autoconf
    fi 
fi

# Download libtool
if [ "${build_libtool}" = "y" ] ; then
    if [ ! -d ${sd}/libtool ] ; then
        cd ${sd}
        curl -o libtool.tar.gz http://ftp.gnu.org/gnu/libtool/libtool-2.4.2.tar.gz
        tar -zxvf libtool.tar.gz
        mv libtool-2.4.2 libtool
    fi
fi

# Download automake
if [ "${build_automake}" = "y" ] ; then 
    if [ ! -d ${sd}/automake ] ; then
        cd ${sd}
        curl -o automake.tar.gz http://ftp.gnu.org/gnu/automake/automake-1.14.tar.gz
        tar -zxvf automake.tar.gz
        mv automake-1.14 automake
    fi
fi

if [ "${build_libuv}" = "y" ] ; then 
    # Download libuv
    if [ ! -d ${sd}/libuv ] ; then
        cd ${sd}
        git clone https://github.com/joyent/libuv.git libuv
    fi

    # Download gyp for libuv
    if [ ! -d ${sd}/libuv/build/gyp ] ; then 
        cd ${sd}/libuv
        git clone https://git.chromium.org/external/gyp.git build/gyp
    fi
fi

# Download libz
if [ "${build_libz}" = "y" ] ; then
    if [ ! -d ${sd}/zlib ] ; then
        cd ${sd}
        if [ ! -f libz.tar.gz ] ; then
            curl -o libz.tar.gz http://zlib.net/zlib-1.2.8.tar.gz
            tar -zxvf libz.tar.gz
        fi
        mv zlib-1.2.8 zlib
    fi
fi

# Download mongoose (signaling)
if [ "${build_mongoose}" = "y" ] ; then
    if [ ! -d ${sd}/mongoose ] ; then 
        cd ${sd}
        git clone https://github.com/cesanta/mongoose.git mongoose
    fi    
    
    if [ ! -f ${bd}/src/mongoose.c ] ; then
        cp ${sd}/mongoose/mongoose.c ${bd}/src/
        cp ${sd}/mongoose/mongoose.h ${bd}/include/
    fi
fi

# Download net_skeleton (signaling)
if [ "${build_netskeleton}" = "y" ] ; then 
    if [ ! -d ${sd}/net_skeleton ] ; then 
        cd ${sd}
        git clone https://github.com/cesanta/net_skeleton.git net_skeleton
    fi

    if [ ! -f ${bd}/src/net_skeleton.c ] ; then
        cp ${sd}/net_skeleton/net_skeleton.c ${bd}/src/
        cp ${sd}/net_skeleton/net_skeleton.h ${bd}/include/
    fi
fi

# Download ssl_wrapper (signaling)
if [ "${build_sslwrapper}" = "y" ] ; then

    if [ ! -d ${sd}/ssl_wrapper ] ; then 
        cd ${sd}
        git clone https://github.com/cesanta/ssl_wrapper.git ssl_wrapper
    fi

    if [ ! -f ${bd}/src/ssl_wrapper.c ] ; then
        cp ${sd}/ssl_wrapper/ssl_wrapper.c ${bd}/src/
        cp ${sd}/ssl_wrapper/ssl_wrapper.h ${bd}/include/
    fi
fi

# Download libpng
if [ "${build_libpng}" = "y" ] ; then
    if [ ! -d ${sd}/libpng ] ; then 
        cd ${sd}
        if [ ! -f libpng.tar.gz ] ; then 
            curl -o libpng.tar.gz -L http://prdownloads.sourceforge.net/libpng/libpng-1.2.51.tar.gz?download
            tar -zxvf libpng.tar.gz
        fi
        mv libpng-1.2.51 libpng
    fi
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

# Download libjpg
if [ "${build_libjpg}" = "y" ] ; then
    if [ ! -d ${sd}/libjpeg ] ; then 
        cd ${sd}
        curl -o jpeg.tar.gz http://www.ijg.org/files/jpegsrc.v9a.tar.gz
        tar -zxvf jpeg.tar.gz
        mv jpeg-9a libjpeg
    fi 
fi

# Download GLAD for GL
if [ "${build_glad}" = "y" ] ; then
    if [ ! -d ${sd}/glad ] ; then 
        cd ${sd}
        git clone --depth 1 --branch master https://github.com/Dav1dde/glad.git glad
    fi
fi

# Download GLFW for GL
if [ "${build_glfw}" = "y" ] ; then
    if [ ! -d ${sd}/glfw ] ; then 
        cd ${sd}
        git clone --depth 1 --branch master https://github.com/glfw/glfw.git glfw
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

# Download video capture library
if [ "${build_videocapture}" = "y" ] ; then
    if [ ! -d ${sd}/video_capture ] ; then
        mkdir ${sd}//video_capture
        cd ${sd}/video_capture
        git clone http://github.com/roxlu/video_capture.git .
    fi
fi

# Download ImageMagick
if [ "${build_imagemagick}" = "y" ] ; then
    if [ ! -d ${sd}/imagemagick ] ; then
        cd ${sd}
        curl -o imagemagick.tar.gz ftp://ftp.imagemagick.org/pub/ImageMagick/binaries/ImageMagick-x86_64-apple-darwin13.2.0.tar.gz
        tar -zxvf imagemagick.tar.gz
        mv ImageMagick-6.8.9 imagemagick
    fi

    # Fix ImageMagick dylibs + install
    if [ ! -f ${id}/imagemagick/convert ] ; then
        if [ ! -d ${id} ] ; then 
            mkdir ${id}
        fi
        if [ ! -d ${id}/lib ] ; then 
            mkdir ${id}/lib
        fi
        if [ ! -d ${id}/imagemagick ] ; then 
            mkdir ${id}/imagemagick
        fi

        # fix dylib paths for imagemagick apps
        cd ${sd}/imagemagick/lib
        for dylib in `ls -1 *.dylib`; do
            for app in ${sd}/imagemagick/bin/* ; do 
                install_name_tool -change "/ImageMagick-6.8.9/lib/${dylib}" "@executable_path/../lib/${dylib}" ${app}
                cp ${app} ${id}/imagemagick/
            done
        done
        
        # fix dylib paths for the dylibs themself + copy them
        cd ${sd}/imagemagick/lib
        for dylib_a in `ls -1 *.dylib`; do
            for dylib_b in `ls -1 *.dylib`; do
                if [ "${dylib_a}" == "${dylib_b}" ] ; then
                    echo "${dylib_a} == ${dylib_b}"
                else
                    install_name_tool -change /ImageMagick-6.8.9/lib/${dylib_b} "@executable_path/../lib/${dylib_b}" ${dylib_a}
                fi
            done
            cp ${sd}/imagemagick/lib/${dylib_a} ${id}/lib
        done
    fi
fi

# Download GraphicsMagick
if [ "${build_graphicsmagick}" = "y" ] ; then
    if [ ! -d ${sd}/graphicsmagick ] ; then
        cd ${sd}
        curl -L -o gm.tar.gz http://downloads.sourceforge.net/project/graphicsmagick/graphicsmagick/1.3.20/GraphicsMagick-1.3.20.tar.gz
        tar -zxvf gm.tar.gz
        mv GraphicsMagick-1.3.20 graphicsmagick
    fi
fi

# Download libav
if [ "${build_libav}" = "y" ] ; then
    if [ ! -d ${sd}/libav ] ; then
        cd ${sd}
        git clone git://git.libav.org/libav.git libav
    fi
fi

# Download yasm, needed for libvpx, x264
if [ "${build_yasm}" = "y" ] ; then
    if [ ! -d ${sd}/yasm ] ; then
        cd ${sd}
        curl -o yasm.tar.gz http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
        tar -zxvf yasm.tar.gz
        mv yasm-1.3.0 yasm
    fi
fi

# Download microprofile
if [ "${build_microprofile}" = "y" ] ; then
    if [ ! -d ${sd}/microprofile ] ; then
        cd ${sd}
        hg clone https://bitbucket.org/jonasmeyer/microprofile 
    fi
fi

# Download libogg 
if [ "${build_ogg}" = "y" ] ; then
    if [ ! -d ${sd}/libogg ] ; then
        cd ${sd}
        curl -o libogg.tar.gz http://downloads.xiph.org/releases/ogg/libogg-1.3.1.tar.gz
        tar -zxvf libogg.tar.gz
        mv libogg-1.3.1 libogg
    fi
fi

# Download theora
if [ "${build_theora}" = "y" ] ; then
    if [ ! -d ${sd}/theora ] ; then
        cd ${sd}
        svn co http://svn.xiph.org/trunk/theora
    fi     
fi

# Downoad vorbis 
if [ "${build_vorbis}" = "y" ] ; then
    if [ ! -d ${sd}/vorbis ] ; then
        cd ${sd}
        curl -o vorbis.tar.gz http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.3.tar.gz
        tar -zxvf vorbis.tar.gz
        mv libvorbis-1.3.3 vorbis
    fi
fi

# Download rxp_player for video playback
if [ "${build_rxpplayer}" = "y" ] ; then
    if [ ! -d ${sd}/rxp_player ] ; then 
        cd ${sd}
        git clone https://github.com/roxlu/rxp_player.git
    fi 
fi

# Download the tracker lib
if [ "${build_tracker}" = "y" ] ; then 
    if [ ! -d ${d}/../extern/tracker ] ; then 
        cd ${d}/../extern/
        git clone https://github.com/roxlu/tracker.git
    fi 
fi

# Download opencv for block tracking
if [ "${build_opencv}" = "y" ] ; then
    if [ ! -d ${sd}/opencv ] ; then 
        cd ${sd}
        if [ ! -f opencv.zip ] ; then
            curl -L -o opencv.zip https://github.com/Itseez/opencv/archive/3.0.0-alpha.zip
        fi
        unzip opencv.zip
        mv opencv-3.0.0-alpha opencv
    fi
fi

# Download pkg-config
if [ "${build_pkgconfig}" = "y" ] ; then
    
    if [ ! -d ${sd}/pkgconfig ] ; then 
        cd ${sd}
        curl -o pkg.tar.gz http://pkgconfig.freedesktop.org/releases/pkg-config-0.28.tar.gz
        tar -zxvf pkg.tar.gz
        mv pkg-config-0.28 pkgconfig
    fi
fi

# Download pixman 
if [ "${build_pixman}" = "y" ] ; then
    if [ ! -d ${sd}/pixman ] ; then
        cd ${sd}
        curl -o pixman.tar.gz http://cairographics.org/releases/pixman-0.32.6.tar.gz
        tar -zxvf pixman.tar.gz
        mv ${sd}/pixman-0.32.6 ${sd}/pixman
    fi 
fi

# Download cairo 
if [ "${build_gettext}" = "y" ] ; then 
    if [ ! -d ${sd}/gettext ] ; then
        cd ${sd}
        curl -o gettext.tar.xv http://ftp.gnu.org/pub/gnu/gettext/gettext-0.19.2.tar.xz
        tar -xvf gettext.tar.xv 
        mv gettext-0.19.2 gettext
    fi 
fi

# Download cairo
if [ "${build_cairo}" = "y" ] ; then
    if [ ! -d ${sd}/cairo ] ; then 
        cd ${sd}
        git clone --depth 1 --branch master git://anongit.freedesktop.org/git/cairo
    fi 
fi

# Download freetype
if [ "${build_freetype}" = "y" ] ; then 
    if [ ! -d ${sd}/freetype ] ; then
        cd ${sd}
        git clone --depth 1 --branch master git://git.sv.nongnu.org/freetype/freetype2.git
    fi
fi

# Download libcurl 
if [ "${build_curl}" = "y" ] ; then
    if [ ! -d ${sd}/curl ] ; then
        cd ${sd}
        curl -o curl.tar.gz http://curl.haxx.se/download/curl-7.37.1.tar.gz
        tar -zxvf curl.tar.gz
        mv curl-7.37.1 curl
    fi
fi

# Download jansson 
if [ "${build_jansson}" = "y" ] ; then
    if [ ! -d ${sd}/jansson ] ; then
        cd ${sd}
        curl -o jans.tar.gz http://www.digip.org/jansson/releases/jansson-2.6.tar.gz
        tar -zxvf jans.tar.gz
        mv jansson-2.6 jansson
    fi
fi

# Download gtkdoc
if [ "${build_gtkdoc}" = "y" ] ; then 
    if [ ! -d ${sd}/gtkdoc ] ; then 
        cd ${sd}
        mkdir gtkdoc
        cd gtkdoc 
        git clone --depth 1 --branch master git://git.gnome.org/gtk-doc .
    fi
fi

# Download pango
if [ "${build_pango}" = "y" ] ; then
    if [ "${is_mac}" = "y" ] ; then
        if [ ! -d ${sd}/pango ] ; then
            cd ${sd}
            git clone --depth 1 --branch master git://git.gnome.org/pango
        fi
    else
        # git version does not work on Linux. GTK doc check fails.
        cd ${sd} 
        if [ ! -d ${sd}/pango ] ; then
            curl -o pango.tar.xz http://ftp.gnome.org/pub/GNOME/sources/pango/1.36/pango-1.36.7.tar.xz
            tar -xvf pango.tar.xz
            mv pango-1.36.7 pango
        fi
    fi
fi


# Download glib
if [ "${build_glib}" = "y" ] ; then
    if [ ! -d ${sd}/glib ] ; then
        cd ${sd}
        mkdir glib
        cd glib
        git clone --depth 1 --branch master git://git.gnome.org/glib .
    fi
fi

# Download ffi
if [ "${build_ffi}" = "y" ] ; then
    if [ ! -d ${sd}/ffi ] ; then
        curl -o ffi.tar.gz ftp://sourceware.org/pub/libffi/libffi-3.1.tar.gz
        tar -zxvf ffi.tar.gz
        mv libffi-3.1 ffi
    fi
fi

# Download libxml
if [ "${build_libxml}" = "y" ] ; then
    if [ ! -d ${sd}/libxml ] ; then
        curl -o libxml.tar.gz ftp://xmlsoft.org/libxml2/libxml2-2.9.1.tar.gz
        tar -zxvf libxml.tar.gz
        mv libxml2-2.9.1 libxml
    fi
fi

# Download colm (needed by ragel)
if [ "${build_colm}" = "y" ] ; then

    if [ ! -d ${sd}/colm ] ; then 
        cd ${sd}
        git clone --depth 1 --branch master https://github.com/ehdtee/colm.git 
    fi

fi

# Download ragel (needed by harfbuzz)
if [ "${build_ragel}" = "y" ] ; then
    if [ ! -d ${sd}/ragel ] ; then
        cd ${sd}
        git clone --depth 1 --branch master https://github.com/ehdtee/ragel.git 
    fi
fi

# Download harfbuzz 
if [ "${build_harfbuzz}" = "y" ] ; then 
    
    if [ ! -d ${sd}/harfbuzz ] ; then
        cd ${sd}
        curl -o hb.tar.bz2 http://www.freedesktop.org/software/harfbuzz/release/harfbuzz-0.9.35.tar.bz2
        bunzip2 hb.tar.bz2
        tar -xvf hb.tar
        mv harfbuzz-0.9.35 harfbuzz

        # git fails on indic with colm 0.13
        # git clone https://github.com/behdad/harfbuzz.git
    fi
fi

# Download fontconfig
if [ "${build_fontconfig}" = "y" ] ; then
    if [ ! -d ${sd}/fontconfig ] ; then
        curl -o fontconfig.tar.gz http://www.freedesktop.org/software/fontconfig/release/fontconfig-2.11.1.tar.gz
        tar -zxvf fontconfig.tar.gz
        mv fontconfig-2.11.1 fontconfig
    fi
fi

# Download x264
if [ "${build_x264}" = "y" ] ; then
    if [ ! -d ${sd}/x264 ] ; then 
        cd ${sd}
        git clone --depth 1 --branch master git://git.videolan.org/x264.git
    fi
fi

# Download flvmeta
if [ "${build_flvmeta}" = "y" ] ; then
    if [ ! -d ${sd}/flvmeta ] ; then 
        cd ${sd}
        curl -o flv.tar.gz -L http://www.flvmeta.com/download.php?file=flvmeta-1.1.2.tar.gz
        tar -xvf flv.tar.gz
        mv flvmeta-1.1.2 flvmeta
    fi
fi

# Download the video generator lib
if [ "${build_videogenerator}" = "y" ] ; then
    if [ ! -d ${sd}/video_generator ] ; then
        cd ${sd}
        git clone --depth 1 --branch master https://github.com/roxlu/video_generator.git
    fi
fi

# Download nasm
if [ "${build_nasm}" = "y" ] ; then
    download nasm nasm-2.11.05 http://www.nasm.us/pub/nasm/releasebuilds/2.11.05/nasm-2.11.05.tar.gz
fi

# Download liblame mp3 encoder
if [ "${build_lame}" = "y" ] ; then
    download lame lame-3.99.5 http://downloads.sourceforge.net/project/lame/lame/3.99/lame-3.99.5.tar.gz
fi

# Download portaudio
if [ "${build_portaudio}" = "y" ] ; then
    download portaudio pa_stable http://www.portaudio.com/archives/pa_stable_v19_20140130.tgz
fi

# Download libyuv
if [ "${build_libyuv}" = "y" ] ; then
    if [ ! -d ${sd}/libyuv ] ; then 
        mkdir ${sd}/libyuv
        cd ${sd}/libyuv
        svn checkout http://libyuv.googlecode.com/svn/trunk/ .
    fi
fi

# Download nanovg
if [ "${build_nanovg}" = "y" ] ; then
    if [ ! -d ${sd}/nanovg ] ; then
        mkdir ${sd}/nanovg
        cd ${sd}/nanovg
        git clone --depth 1 --branch master https://github.com/memononen/nanovg.git .
    fi
fi

# Download liblo 
if [ "${build_liblo}" = "y" ] ; then
    if [  ! -d ${sd}/liblo ] ; then
        mkdir ${sd}/liblo
        cd ${sd}/liblo
        git clone --depth 1 --branch master git://liblo.git.sourceforge.net/gitroot/liblo/liblo .
    fi
fi

# Download remoxly gui library
if [ "${build_remoxly}" = "y" ] ; then
    if [ ! -d ${sd}/remoxly ] ; then 
        mkdir ${sd}/remoxly
        cd ${sd}/remoxly
        git clone git@github.com:roxlu/remoxly.git .
    fi
fi

# Cleanup some files we don't need anymore.
if [ -f ${sd}/autoconf.tar.gz ] ; then
    rm ${sd}/autoconf.tar.gz
fi
if [ -f ${sd}/automake.tar.gz ] ; then
    rm ${sd}/automake.tar.gz
fi
if [ -f ${sd}/libtool.tar.gz ] ; then
    rm ${sd}/libtool.tar.gz 
fi
if [ -f ${sd}/libz.tar.gz ] ; then
    rm ${sd}/libz.tar.gz 
fi
if [ -f ${sd}/libpng.tar.gz ] ; then 
    rm ${sd}/libpng.tar.gz
fi
if [ -f ${sd}/jpeg.tar.gz ] ; then
    rm ${sd}/jpeg.tar.gz
fi 
if [ -f ${sd}/imagemagick.tar.gz ] ; then 
    rm ${sd}/imagemagick.tar.gz
fi 
if [ -f ${sd}/yasm.tar.gz ] ; then 
    rm ${sd}/yasm.tar.gz
fi 
if [ -f ${sd}/rapidxml.zip ] ; then
    rm ${sd}/rapidxml.zip
fi
if [ -f ${sd}/libogg.tar.gz ] ; then
    rm ${sd}/libogg.tar.gz
fi
if [ -f ${sd}/theora.zip ] ; then
    rm ${sd}/theora.zip
fi
if [ -f ${sd}/vorbis.tar.gz ] ; then
    rm ${sd}/vorbis.tar.gz
fi
if [ -f ${sd}/opencv.zip ] ; then
    rm ${sd}/opencv.zip
fi 
if [ -f ${sd}/gettext.tar.gz ] ; then
    rm ${sd}/gettext.tar.gz
fi
if [ -f ${sd}/pkg.tar.gz ] ; then
    rm ${sd}/pkg.tar.gz 
fi
if [ -f ${sd}/curl.tar.gz ] ; then
    rm ${sd}/curl.tar.gz
fi
if [ -f ${sd}/jans.tar.gz ] ; then
    rm ${sd}/jans.tar.gz
fi
if [ -f ${sd}/gm.tar.gz ] ; then
    rm ${sd}/gm.tar.gz
fi
if [ -f ${sd}/pixman.tar.gz ] ; then
    rm ${sd}/pixman.tar.gz
fi
if [ -f ${sd}/cairo.xz ] ; then 
    rm ${sd}/cairo.xz
fi
if [ -f ${sd}/colm.tar.gz ] ; then
    rm ${sd}/colm.tar.gz
fi
if [ -f ${sd}/docbookxsl.tar.gz ] ; then
    rm ${sd}/docbookxsl.tar.gz
fi
if [ -f ${sd}/docbookxsl.tar ] ; then
    rm ${sd}/docbookxsl.tar
fi
if [ -f ${sd}/fontconfig.tar.gz ] ; then
    rm ${sd}/fontconfig.tar.gz
fi
if [ -f ${sd}/hb.tar ] ; then
    rm ${sd}/hb.tar
fi
if [ -f ${sd}/libxml.tar.gz ] ; then
    rm ${sd}/libxml.tar.gz
fi
if [ -f ${sd}/ffi.tar.gz ] ; then
    rm ${sd}/ffi.tar.gz 
fi
if [ -f ${sd}/flv.tar.gz ] ; then
    rm ${sd}/flv.tar.gz
fi
if [ -f ${sd}/lame.tar.gz ] ; then
    rm ${sd}/lame.tar.gz
fi
if [ -f ${sd}/m4.tar.gz ] ; then
    rm ${sd}/m4.tar.gz
fi


# ----------------------------------------------------------------------- #
#                C O M P I L E   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #

# Compile m4 
if [ "${build_m4}" = "y" ] ; then
    if [ ! -f ${bd}/bin/m4 ] ; then
        cd ${sd}/m4
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Compile autoconf
if [ "${build_autoconf}" = "y" ] ; then
    if [ ! -f ${bd}/bin/autoconf ] ; then
        cd ${sd}/autoconf
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Compile libtool
if [ "${build_libtool}" = "y" ] ; then 
    if [ ! -f ${bd}/bin/libtool ] ; then
        cd ${sd}/libtool
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Compile automake 
if [ "${build_automake}" = "y" ] ; then
    if [ ! -f ${bd}/bin/automake ] ; then 
        cd ${sd}/automake
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Build gtkdoc, needs some manual changes  
if [ "${build_gtkdoc}" = "y" ] ; then 

    if [ ! -d ${sd}/docbookxsl ] ; then
        cd ${sd}
        curl -o docbookxsl.tar.bz2 -L http://downloads.sourceforge.net/project/docbook/docbook-xsl/1.78.1/docbook-xsl-1.78.1.tar.bz2
        bunzip2 docbookxsl.tar.bz2
        tar -xvf docbookxsl.tar
        mv docbook-xsl-1.78.1 docbookxsl
    fi

    if [ ! -f ${bd}/bin/gtkdocize ] ; then 
        cd ${sd}/gtkdoc
        if [ ! -f ./configure ] ; then
            ./autogen.sh
        fi

        if [ !  -f ${sd}/gtkdoc/catalog.xml ] ; then

            # Downlaod the catalog
            curl -o catalog.xml http://www.docbook.org/xml/4.3/catalog.xml

            # Add the nextCatalog line 
            cmd="sed -i.bak 's|:catalog\">|:catalog\"><nextCatalog catalog=\"${sd}/docbookxsl\/catalog.xml\"\/>|g' catalog.xml"
            eval ${cmd}

            # I had to add this line in the <catalog> section 
            # <nextCatalog catalog="/Users/roxlu/Downloads/docbook-xsl-1.78.1/catalog.xml"/>
        fi

        ./configure --prefix=${bd} --with-xml-catalog=${sd}/gtkdoc/catalog.xml
        make 
        make install
    fi
fi

# Compile pkg config
if [ "${build_pkgconfig}" = "y" ] ; then 
    if [ ! -f ${bd}/bin/pkg-config ] ; then 
        cd ${sd}/pkgconfig
        ./configure --prefix=${bd} --with-internal-glib
        make
        make install
    fi
fi

# Compile gettext
if [ "${build_gettext}" = "y" ] ; then
    if [ ! -f ${bd}/bin/gettext ] ; then
        cd ${sd}/gettext
        ./configure --prefix=${bd} --enable-static=yes
        make 
        make install
    fi
fi

# Compile libuv
if [ "${build_libuv}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libuv.a ] ; then
        cd ${sd}/libuv
        if [ "${is_linux}" = "y" ] ; then
            if [ ! -f ./configure ] ; then
                ./autogen.sh
            fi
            ./configure --prefix=${bd}
            make 
            make install
        else
            ./gyp_uv.py -f xcode
            xcodebuild -ARCHS="x86_64" -project uv.xcodeproj -configuration Release -target All
            cp ${sd}/libuv/build/Release/libuv.a ${bd}/lib/
            cp ${sd}/libuv/include/*.h ${bd}/include/
        fi
    fi
fi

# Compile zlib
if [ "${build_libz}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libz.a ] ; then 
        cd ${sd}/zlib
        ./configure --prefix=${bd} --static --64
        make
        make install
    fi
fi

# Compile libpng
if [ "${build_libpng}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libpng.a ] ; then 
        cd ${sd}/libpng
        ./configure --enable-static=yes --enable-shared=no --prefix=${bd}
        make
        make install
    fi
fi

# Compile libjpeg
if [ "${build_libjpg}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libjpeg.a ] ; then 
        cd ${sd}/libjpeg
        ./configure --prefix=${bd}
        make 
        make install
    fi
fi

# Copy the GLAD sources + generate the C extension
if [ "${build_glad}" = "y" ] ; then 
    if [ ! -f ${bd}/src/glad.c ] ; then
        if [ ! -d ${bd}/src ] ; then
            mkdir ${bd}/src 
        fi
        cd ${sd}/glad
        if [ -f /usr/bin/python2 ] ; then
            python2 main.py --generator=c --out-path=gl --extensions GL_ARB_timer_query,GL_APPLE_rgb_422
        else
            python main.py --generator=c --out-path=gl --extensions GL_ARB_timer_query,GL_APPLE_rgb_422
        fi
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

# Compile yasm
if [ "${build_yasm}" = "y" ] ; then 
    if [ ! -f ${bd}/bin/yasm ] ; then
        cd ${sd}/yasm
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Compile nasm
if [ "${build_nasm}" = "y" ] ; then
    compile nasm bin/nasm
fi

# Compile libav 
if [ "${build_libav}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libavcodec.a ] ; then
        cd ${sd}/libav
        ./configure --prefix=${bd} --enable-gpl 
        make
        make install
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

# Move the microprofiler
if [ "${build_microprofile}" = "y" ] ; then 
    if [ ! -f ${bd}/include/microprofile.h ] ; then
        cp ${sd}/microprofile/microprofile.h ${bd}/include
        cp ${sd}/microprofile/demo/ui/microprofile.cpp ${bd}/src
    fi
fi

# Compile ogg
if [ "${build_ogg}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libogg.a ] ; then
        cd ${sd}/libogg
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Compile vorbis
if [ "${build_vorbis}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libvorbis.a ] ; then
        cd ${sd}/vorbis
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Compile libtheora
if [ "${build_theora}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libtheora.a ] ; then 
        cd ${sd}/theora
        ./autogen.sh
        ./configure --prefix=${bd} 
        make
        make install
    fi
fi

# Compile rxp_player
if [ "${build_rxpplayer}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/rxp_player.a ] ; then
        cd ${sd}/rxp_player/build
        mkdir build.release
        cd build.release
        cmake -DCMAKE_INSTALL_PREFIX=${bd} -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . --target install
    fi
fi

# Compile opencv
if [ "${build_opencv}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libopencv_core.a ] ; then 
        cd ${sd}/opencv
        if [ ! -d build.release ] ; then
            mkdir build.release
        fi
        cd build.release
        cmake -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX=${bd} \
            -DBUILD_SHARED_LIBS=0 \
            -DBUILD_PACKAGE=0 \
            -DBUILD_PERF_TESTS=0 \
            -DBUILD_PNG=0 \
            -DBUILD_TBB=0 \
            -DBUILD_TESTS=0 \
            -DBUILD_TIFF=0 \
            -DBUILD_WITH_DEBUG_INFO=0 \
            -DBUILD_ZLIB=0 \
            -DBUILD_EXAMPLES=1 \
            -DBUILD_opencv_apps=0 \
            -DBUILD_opencv_bioinspired=0 \
            -DBUILD_opencv_calib3d=0 \
            -DBUILD_opencv_contrib=0 \
            -DBUILD_opencv_core=1 \
            -DBUILD_opencv_cuda=0, \
            -DBUILD_opencv_features2d=1 \
            -DBUILD_opencv_flann=1 \
            -DBUILD_opencv_highgui=0 \
            -DBUILD_opencv_imgproc=1 \
            -DBUILD_opencv_legacy=0 \
            -DBUILD_opencv_ml=1 \
            -DBUILD_opencv_nonfree=0 \
            -DBUILD_opencv_objdetect=1 \
            -DBUILD_opencv_ocl=1 \
            -DBUILD_opencv_optim=1 \
            -DBUILD_opencv_photo=1 \
            -DBUILD_opencv_python=0 \
            -DBUILD_opencv_python2=0 \
            -DBUILD_opencv_shape=0 \
            -DBUILD_opencv_softcascade=0 \
            -DBUILD_opencv_stitching=0 \
            -DBUILD_opencv_video=1 \
            -DBUILD_opencv_videostab=0 \
            -DBUILD_opencv_world=0 \
            -DWITH_CUDA=0 \
            -DWITH_CUFFT=0 \
            -DWITH_EIGEN=0 \
            -DWITH_JPEG=0 \
            -DWITH_JASPER=0 \
            -DWITH_LIBV4L=0 \
            -DWITH_OPENCL=1 \
            -DWITH_OPENEXR=0 \
            -DWITH_PNG=0 \
            -DWITH_TIFF=0 \
            -DWITH_V4L=0 \
            -DWITH_WEBP=0 \
            -DWITH_QT=0 \
            -DWITH_FFMPEG=0 \
            -DWITH_VTK=0 \
            -DWITH_IPP=0 \
            ..

        cmake --build . --target install
    fi
fi

# Build ffi
if [ "${build_ffi}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libffi.a ] ; then 
        cd ${sd}/ffi
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Build libxml2
if [ "${build_libxml}" = "y" ] ; then
    if [ ! -f ${bd}/lib/libxml2.a ] ; then 
        cd ${sd}/libxml
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Build glib
if [ "${build_glib}" = "y" ] ; then
    if [ ! -f ${bd}/lib/libglib-2.0.a ] ; then 
        cd ${sd}/glib

        ./autogen.sh
        ./configure --prefix=${bd} \
            --enable-shared=no \
            --enable-static=yes 
        make 
        make install
    fi
fi

# Compile colm
if [ "${build_colm}" = "y" ] ; then 
    if [ ! -f ${bd}/bin/colm ] ; then 
        cd ${sd}/colm
        if [ ! -f ./configure ] ; then 
            ./autogen.sh
        fi
        ./configure --prefix=${bd}
        make 
        make install
    fi
fi


# Compile ragel
if [ "${build_ragel}" = "y" ] ; then 
    if [ ! -f ${bd}/bin/ragel ] ; then 
        # Info:   I had to remove the version check of colm in the configure.ac before
        #         running autogen.sh, see lines 77-80

        # This line will fix the version check on Mac
        sed -i.bak s/COLM_VERSION=\\[.*\\]/COLM_VERSION=\\[\`\$COLM\ -v\ \|\ head\ \-1\ \|\ cut\ -d\ \"\ \"\ -f\ 3\ \|\ cut\ -d\ \".\"\ -f\ 1\ -f\ 2\`\\]/g ${sd}/ragel/configure.ac

        cd ${sd}/ragel
        if [ ! -f ./configure ] ; then
            ./autogen.sh
        fi

        ./configure --prefix=${bd} \
            --with-colm=${bd} \
            --disable-manual
        make 
        make install
    fi
fi

# Compile freetype
if [ "${build_freetype}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libfreetype.a ] ; then

        export PATH=${bd}/bin/:${sd}/gyp/:${PATH}
        export CFLAGS="-I${bd}/include"
        export LDFLAGS="-L${bd}/lib"

        cd ${bd}/bin
        ln -s libtoolize glibtoolize
        cd ${sd}/freetype2

        ./autogen.sh
        ./configure --prefix=${bd}
        make
        make install
        
        export PATH=${pathcopy}
        export CFLAGS=${cfcopy}
        export LDFLAGS=${ldcopy}
    fi
fi

# Compile harfbuzz 
if [ "${build_harfbuzz}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libharfbuzz.a ] ; then 
        cd ${sd}/harfbuzz
        if  [ ! -f ./configure ] ; then
            ./autogen.sh
        fi

        export FREETYPE_CFLAGS="-I${bd}/include/freetype2/"
        export FREETYPE_LIBS="-lfreetype"

        if [ "${is_mac}" = "y" ] ; then 
            ./configure --prefix=${bd} \
                --with-coretext=yes \
                --enable-static=yes \
                --enable-shared=no
        elif [ "${is_linux}" = "y" ] ; then
            ./configure --prefix=${bd} \
                --enable-static=yes \
                --enable-shared=no
        fi

        make
        make install
    fi
fi

# Compile libcurl 
if [ "${build_curl}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libcurl.a ] ; then
        cd ${sd}/curl
        ./configure --prefix=${bd} \
            --enable-static=yes \
            --enable-shared=0 \
            --disable-ldaps \
            --disable-rtsp \
            --disable-dict \
            --disable-telnet \
            --disable-pop3 \
            --disable-imap \
            --disable-smtp \
            --disable-librtmp \
            --disable-libssh2 \
            --disable-gopher \
            --disable-axtls \
            --disable-ares
        make
        make install
    fi
fi

if [ "${build_jansson}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libjansson.a ] ; then 
        cd ${sd}/jansson
        mkdir build.release
        cd build.release 
        cmake -DCMAKE_INSTALL_PREFIX=${bd} -DCMAKE_BUILD_TYPE=Release ../
        cmake --build . --target install
    fi
fi

# Compile pixman
if [ "${build_pixman}" = "y" ] ; then 
    if [ ! -d ${bd}/include/pixman-1 ] ; then

        cd ${sd}/pixman
        if [ ! -f ./configure ] ; then
            ./autogen.sh
        fi

        #export LIBS="-lutils"
        export PKG_CONFIG=${bd}/bin/pkg-config
        export PKG_CONFIG_PATH=${bd}/lib/pkgconfig
        ./configure --prefix=${bd} \
            --disable-dependency-tracking \
            --enable-static=yes \
            --enable-shared=non
        make
        make install

        # We need to copy the pkgconfig file manually 
        cp ${sd}/pixman/pixman-1.pc ${bd}/lib/pkgconfig/
    fi
fi

# Build fontconfig
if [ "${build_fontconfig}" = "y" ] ; then
    if [ ! -f ${bd}/lib/libfontconfig.a ] ; then
        export PKG_CONFIG=${bd}/bin/pkg-config
        export PKG_CONFIG_PATH=${bd}/lib/pkgconfig
        export FREETYPE_CFLAGS="-I${bd}/include/freetype2/"
        export FREETYPE_LIBS="-lfreetype"
        cd ${sd}/fontconfig
        ./configure --prefix=${bd} \
            --enable-static=yes \
            --enable-shared=no
        make
        make install
    fi
fi

# Build cairo 
if [ "${build_cairo}" = "y" ] ; then 
    if [ ! -f ${bd}/lib/libcairo.a ] ; then 
        export PKG_CONFIG=${bd}/bin/pkg-config
        export PKG_CONFIG_PATH=${bd}/lib/pkgconfig
        export pixman_CFLAGS=-I${bd}/include/pixman-1/
        export FONTCONFIG_CFLAGS=-I${bd}/include/
        export FONTCONFIG_LIBS=-lfontconfig
        export pixman_LIBS="-L${bd}/lib -lpixman-1"

        if [ "${is_mac}" = "y" ] ; then
            export LIBS="-framework CoreFoundation -framework Cocoa -lfreetype -lfontconfig"
        elif [ "${is_linux}" = "y" ] ; then
            export LIBS="-lfreetype -lfontconfig"
        fi

        cd ${sd}/cairo
        if [ ! -f ./configure ] ; then
            ./autogen.sh
        fi

        ./configure \
            --prefix=${bd} \
            --disable-dependency-tracking \
            --disable-xlib \
            --enable-static=yes \
            --enable-shared=no
        make
        make install
    fi
fi


# Build pango
if [ "${build_pango}" = "y" ] ; then 
    
    if [ ! -f ${bd}/lib/libpango-1.0.a ] ; then 

        export PKG_CONFIG=${bd}/bin/pkg-config
        export PKG_CONFIG_PATH=${bd}/lib/pkgconfig
        export pixman_CFLAGS=-I${bd}/include/pixman-1/
        export pixman_LIBS="-L${bd}/lib/ -lpixman-1"
        export CAIRO_LIBS="-L${bd}/lib/ -lcairo"
        export CAIRO_CFLAGS="-I${bd}/include/cairo/"
        export FREETYPE_CFLAGS="-I${bd}/include/freetype2/"
        export FREETYPE_LIBS="-lfreetype"
        export FONTCONFIG_CFLAGS=-I${bd}/include/
        export FONTCONFIG_LIBS=-lfontconfig
        export CFLAGS="-I${bd}/include -I${bd}/include/cairo/"

        if [ "${is_mac}" = "y" ] ; then
            export LIBS="-framework CoreFoundation -framework Cocoa"
        fi

        # Info: we need to use ./autogen.sh once gtk docs has been compiled!
        #       also the --with-included-modules is necessary to embed the 
        #       pango modules into the generated pango library.
        cd ${sd}/pango
        if [ ! -f configure ] ; then 
            ./autogen.sh
        fi

        ./configure --prefix=${bd} \
            --disable-dependency-tracking \
            --enable-shared=no \
            --enable-static=yes \
            --with-included-modules 

        make 
        make install
    fi
fi 

# Compile x264
if [ "${build_x264}" = "y" ] ; then
    if [ ! -f ${bd}/lib/libx264.a ] ; then
        cd ${sd}/x264
        ./configure --prefix=${bd} \
            --enable-static
        make
        make install
    fi
fi

# Compile flvmeta
if [ "${build_flvmeta}" = "y" ] ; then
    if [ ! -f ${bd}/bin/flvmeta ] ; then 
        cd ${sd}/flvmeta
        ./configure --prefix=${bd}
        make
        make install
    fi
fi

# Compile the video generator
# if [ "${build_videogenerator}" = "y" ] ; then
#     if [ ! -f ${sd}/lib/libvideogenerator.a ] ; then
#         cd ${sd}/video_generator
#         cd build
#         if [ ! -d compiled ] ; then
#             mkdir compiled
#         fi
#         cd compiled
#         cmake -DCMAKE_INSTALL_PREFIX=${bd} -DCMAKE_BUILD_TYPE=Release  ../
#         cmake --build . --target install --config Release
#     fi
# fi

# Compile liblame
if [ "${build_lame}" = "y" ] ; then
    compile lame lib/libmp3lame.a "--enable-static=yes --enable-nasm"
fi

# Compile portaudio
if [ "${build_portaudio}" = "y" ] ; then
    compile portaudio lib/libportaudio.a "--enable-static=yes"
fi

# Compile video capture
if [ "${build_videocapture}" = "y" ] ; then
    if [ ! -f ${bd}/lib/libvideocapture.a ] ; then
        cd ${sd}/video_capture/build
        if [ -d ${sd}/video_capture/build/build.release ] ; then
            rm -r build.release
        fi
        mkdir build.release
        cd build.release
        cmake -DCMAKE_INSTALL_PREFIX=${bd} -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . --target install --config Release
    fi
fi

# Compile libyuv
if [ "${build_libyuv}" = "y" ] ; then

    # Copy the cmake file. 
    if [ ! -f ${sd}/libyuv/CMakeLists.txt ] ; then
        cp ${sd}/cmake/yuv/CMakeLists.txt ${sd}/libyuv/
    fi

    if [ 0 == 1 ] ; then 
        cd ${sd}/libyuv
        if [ ! -d ${sd}/libyuv/build ] ; then
            mkdir ${sd}/libyuv/build

            cd ${sd}/libyuv/build
            mkdir depot_tools
            cd depot_tools
            git clone --depth 1 --branch master https://chromium.googlesource.com/chromium/tools/depot_tools.git .
        fi

        export PATH=${sd}/libyuv/build/depot_tools:${PATH}
        cd ${sd}/libyuv/
        which gclient
        gclient config http://libyuv.googlecode.com/svn/trunk 
        gclient sync
        exit
        set GYP_DEFINES="clang=1 target_arch=x64" 
        ./gyp_libyuv -fninja --depth=. libyuv.gyp
        mkdir -p out/Release
        ninja -j7 -C out/Release
    else

        if [ ! -f ${bd}/lib/libyuv.a ] ; then
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
fi

if [ "${build_nanovg}" = "y" ] ; then

    echo "..."
#    cd ${sd}/nanovg
#    export CFLAGS="-I${id}"
#    export LDFLAGS="-L${ld}"
#    ./premake4 gmake
#    cd build
#    make config=release
    
fi

if [ "${build_liblo}" = "y" ] ; then
    if [ ! -f ${bd}/lib/liblo.a ] ; then 
        cd ${sd}/liblo
        ./autogen.sh
        ./configure --prefix=${bd} --enable-static=yes --enable-shared=no
        make 
        make install
    fi
fi

if [ "${build_remoxly}" = "y" ] ; then
    if [ ! -f ${bd}/lib/libremoxly.a ] ; then
        cd ${sd}/remoxly/projects/gui/build

        if [ -d build.release ] ; then
            rm -r build.release
            echo "remove!"
        fi
        mkdir build.release
        cd build.release 
        export CFLAGS="${CFLAGS} -I${sd}/tinylib/src/"

        cmake -DCMAKE_INSTALL_PREFIX=${bd} \
            -DEXTERN_LIB_DIR=${bd}/lib \
            -DEXTERN_INCLUDE_DIR=${bd}/include \
            -DCMAKE_BUILD_TYPE=Release ..
        cmake --build . --target install --config Release
    fi
fi
