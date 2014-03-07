#!/bin/sh

if [ -d build ] ; then 
    rm -r build 
fi

if [ -d build.release ] ; then 
    rm -r build.release
fi

if [ -d build.debug ] ; then 
    rm -r build.debug
fi
