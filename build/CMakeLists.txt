cmake_minimum_required(VERSION 2.8)

# You should define the following variables when calling cmake:
#
#  - USE_GENERATE_X86                 Set to true/on if you want to compile for a x86_64 architecture.
#  - USE_GENERATE_IPHONE              Set to true/on if you want to compile for a iphone. 
#  - USE_GENERATE_IPHONE_SIMULATOR    Set to true/on if you want to compile for the iphone simulator.
#  - USE_GENERATE_RPI                 Set to true/on if you want to compile for a rpi.
#  - EXTERN_LIB_DIR                   Path to the directory where we can find e.g. libpng, libz, etc..
#  - EXTERN_INC_DIR                   Path to the directory where we can find general headers e.g. png.h 
#  - EXTERN_SRC_DIR                   Path to the directory where we can find sources for dependencies (e.g. glad.c) 
#  - TINYLIB_DIR                      Path to tinylib base dir. 
#
# DeckLink
# ========
#    - You need to download the DeckLink SDK and copy the include dir from
#      the SDK to extern/*/include/decklink/
#    - On windows we generate a .c and .h file using a custom target.  


project(videocapture)

include(ExternalProject)

option(USE_GENERATE_X86               "Compile for x86_64" Off)
option(USE_GENERATE_IPHONE            "Compile for iOS" Off)
option(USE_GENERATE_IPHONE_SIMULATOR  "Compile for iOS simulator" Off)
option(USE_GENERATE_RPI               "Compile for RaspberryPI" Off)
option(USE_OPENGL                     "Create OpenGL examples" Off)
option(USE_DECKLINK                   "Use Decklink capture card." Off)

message(STATUS "USE_GENERATE_X86: ${USE_GENERATE_X86}")
message(STATUS "USE_GENERATE_IPHONE: ${USE_GENERATE_IPHONE}")
message(STATUS "USE_GENERATE_IPHONE_SIMULATOR: ${USE_GENERATE_IPHONE_SIMULATOR}")
message(STATUS "USE_GENERATE_RPI: ${USE_GENERATE_RPI}")
message(STATUS "USE_OPENGL: ${USE_OPENGL}")
message(STATUS "USE_DECKLINK: ${USE_DECKLINK}")

if (USE_GENERATE_X86)
  include("VideoCaptureX86.cmake")  
elseif (USE_GENERATE_IPHONE OR USE_GENRATE_IPHONE_SIMULATOR)
  include("VideoCaptureIPhone.cmake")  
elseif (USE_GENERATE_RPI)
  include("VideoCaptureRPI.cmake")  
endif()
