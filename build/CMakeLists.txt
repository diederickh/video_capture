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
    ${sd}/videocapture/mac/AVFoundation.cpp
    ${sd}/videocapture/mac/AVImplementation.mm
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

include_directories(${videocapture_include_dirs})

add_executable(${CMAKE_PROJECT_NAME} ${sd}/main.cpp ${videocapture_sources})

target_link_libraries(${CMAKE_PROJECT_NAME} ${videocapture_libraries})