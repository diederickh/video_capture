***************
Getting Started
***************

.. highlight:: c++

To compile Video Capture you need to do:

- Make sure that you installed all dependencies
- Clone the Video Capture repository from github
- Compile using the build script 

Building the library
====================

Video Capture primary location is Github. To get the code clone the project::

     git clone git@github.com:roxlu/video_capture.git     


Dependencies
------------

Video Capture main development systems are  Mac OS 10.9, Windows 8.1 and 
Arch Linux. On Linux we use the Video4Linux API, on Mac we use AVFoundation 
which are both part of the OS. On Windows you need to download the latest 
Windows SDK which provides the MediaFoundation libraries.  We use CMake_ to 
compile the library and examples. The Video Capture library contains an OpenGL 
example. For this OpenGL example we depend on libglfw_ 3.

.. _libglfw: http://www.glfw.org


Compiling Video Capture on Mac and Linux
----------------------------------------

For Mac and Linux systems we use the same compile script. To compile
follow these steps.

::

  cd build
  ./release.sh

Compiling Video Capture on Windows
----------------------------------

On windows we use CMake too with a build script. Development uses 
Microsoft Visual Studio 2012 Express. To compile on Windows follow these
steps.

::
   
   cd build
   build.bat 64 release


.. _CMake: http://www.cmake.org


Compiling programs that use Video Capture
=========================================

To compile a program that uses Video Capture make sure to link with the 
created `libvideocapture.a` on Mac and Linux and the `libvideocapture.lib` 
file on Windows.  The library is installed in the `install` directory that 
we create when you use the above describe build steps.

Also make sure to add a header search path to the headers that we also
install into the `install` directory.

Libraries to link with on Linux
-------------------------------

 - udev

Libraries to link with on Mac
-----------------------------

 - CoreFoundation Framework
 - AVFoundatoin framework
 - Cocoa
 - CoreVideo
 - CoreMedia


Libraries to link with on Windows
---------------------------------

 - Mfplat.lib
 - Mf.lib
 - Mfuuid.lib
 - Mfreadwrite.lib 
 - Shlwapi.lib

