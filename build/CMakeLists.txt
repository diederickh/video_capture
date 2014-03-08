cmake_minimum_required(VERSION 2.8)

project(videocapture)

set(bd ${CMAKE_CURRENT_LIST_DIR}/..)
set(sd ${bd}/src)
set(id ${bd}/include/)

set(videocapture_sources 
  ${sd}/main.cpp
  ${sd}/videocapture/Base.cpp
  ${sd}/videocapture/Types.cpp
  ${sd}/videocapture/Utils.cpp
)

set(videocapture_include_dirs ${id})

if(APPLE)

  list(APPEND videocapture_sources
    ${sd}/videocapture/mac/AVFoundation_Capture.cpp
    ${sd}/videocapture/mac/AVFoundation_Implementation.mm
    )

  find_library(fr_core_foundation CoreFoundation)
  find_library(fr_avfoundation AVFoundation)
  find_library(fr_cocoa Cocoa)
  find_library(fr_core_video CoreVideo)
  find_library(fr_core_media CoreMedia)
  find_library(fr_core_media_io CoreMediaIO)
  
  list(APPEND videocapture_libraries
    ${fr_core_foundation}
    ${fr_avfoundation}
    ${fr_cocoa}
    ${fr_core_video}
    ${fr_core_media}
    ${fr_core_media_io}
    )
endif()

if(UNIX AND NOT APPLE)

  list(APPEND videocapture_sources
    ${sd}/videocapture/linux/V4L2_Capture.cpp
    ${sd}/videocapture/linux/V4L2_Types.cpp
    ${sd}/videocapture/linux/V4L2_Utils.cpp
    )

  list(APPEND videocapture_libraries
    udev
    )

  add_definitions(-D__STDC_CONSTANT_MACROS)

endif()

if(WIN32)

  include_directories(${windows_sdk_dir}/Include)

  list(APPEND videocapture_sources
    ${sd}/videocapture/win/MediaFoundation_Capture.cpp
    ${sd}/videocapture/win/MediaFoundation_Utils.cpp
    ${sd}/videocapture/win/MediaFoundation_Callback.cpp
    )

  list(APPEND videocapture_libraries
    Mfplat.lib
    Mf.lib
    Mfuuid.lib
    Mfreadwrite.lib # MFCreateSourceFreaderFromMediaSource
    Shlwapi.lib  # QISearch (Callback)
    )
 
endif()

include_directories(${videocapture_include_dirs})

add_executable(${CMAKE_PROJECT_NAME} ${sd}/main.cpp ${videocapture_sources})

target_link_libraries(${CMAKE_PROJECT_NAME} ${videocapture_libraries})