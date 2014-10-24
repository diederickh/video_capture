# @todo we should pass the compiler links using the shell scripts!
#  if (USE_IOS_SIMULATOR)
#    set(ios_sysroot "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/")
#    set(CMAKE_OSX_SYSROOT "${ios_sysroot}SDKs/iPhoneSimulator7.1.sdk/")
#    set(CMAKE_OSX_ARCHITECTURES "i386")
#    set(CMAKE_C_COMPILER ${ios_sysroot}/usr/bin/gcc)
#    set(CMAKE_CXX_COMPILER ${ios_sysroot}/usr/bin/g++)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-abi-version=2")
#    include_directories(CMAKE_OSX_SYSROOT "${io_sysroot}/Developer/SDKs/iPhoneSimulator7.1.sdk/usr/include/")
#  endif()

set(ios_root  "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/")
set(CMAKE_OSX_SYSROOT "${ios_root}SDKs/iPhoneOS8.1.sdk/")
set(CMAKE_OSX_ARCHITECTURES "armv7")
set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-objc-arc -fobjc-abi-version=2 -fvisibility=hidden -fvisibility-inlines-hidden -isysroot ${ios_sysroot}")
include_directories(${ios_root}SDKs/iPhoneOS8.1.sdk/usr/include)

set(bd ${CMAKE_CURRENT_LIST_DIR}/..)
set(sd ${bd}/src)
set(id ${bd}/include/)

set(videocapture_sources 
  ${sd}/videocapture/Base.cpp
  ${sd}/videocapture/Capture.cpp
  ${sd}/videocapture/Types.cpp
  ${sd}/videocapture/Utils.cpp
  ${sd}/videocapture/mac/AVFoundation_Capture.cpp
  ${sd}/videocapture/mac/AVFoundation_Implementation.mmo
)

find_library(fr_appkit AppKit)
find_library(fr_core_graphics CoreGraphics)
find_library(fr_foundation Foundation)
find_library(fr_core_foundation CoreFoundation)
find_library(fr_avfoundation AVFoundation)
find_library(fr_core_video CoreVideo)
find_library(fr_core_media CoreMedia)

list(APPEND videocapture_libraries
  ${fr_core_foundation}
  ${fr_avfoundation}
  ${fr_core_video}
  ${fr_core_media}
  ${fr_foundation}
  ${fr_core_graphics}
  ${fr_appkit}
  )

include_directories(${videocapture_include_dirs})
add_library(videocapture ${videocapture_sources})
install(DIRECTORY ${bd}/include/videocapture DESTINATION include)
install(TARGETS videocapture ARCHIVE DESTINATION lib)
