cmake_minimum_required(VERSION 2.8)

# You should define the following variables when calling cmake:
#
#  - EXTERN_LIB_DIR      Path to the directory where we can find e.g. libpng, libz, etc..
#  - EXTERN_INC_DIR      Path to the directory where we can find general headers e.g. png.h 
#  - EXTERN_SRC_DIR      Path to the directory where we can find sources for dependencies (e.g. glad.c) 
#  - TINYLIB_DIR         Path to tinylib base dir. 

project(videocapture)

include(ExternalProject)

option(USE_OPENGL "Create OpenGL examples" ON)
option(USE_DECKLINK "Use Decklink capture card." ON)

set(bd ${CMAKE_CURRENT_LIST_DIR}/..)
set(sd ${bd}/src)
set(id ${bd}/include/)

set(videocapture_sources 
  ${sd}/videocapture/Base.cpp
  ${sd}/videocapture/Capture.cpp
  ${sd}/videocapture/Types.cpp
  ${sd}/videocapture/Utils.cpp
)

set(videocapture_include_dirs 
  ${id}
  ${EXTERN_INC_DIR}
  ${TINYLIB_DIR}/src/
  )

set(gl_sources 
  ${EXTERN_SRC_DIR}/glad.c
  )

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
  
  list(APPEND videocapture_libraries
    ${fr_core_foundation}
    ${fr_avfoundation}
    ${fr_cocoa}
    ${fr_core_video}
    ${fr_core_media}
    )

  if(USE_OPENGL)

    find_library(fr_opengl OpenGL)
    find_library(fr_iokit IOKit)

    list(APPEND videocapture_libraries
      ${EXTERN_LIB_DIR}/libglfw3.a
      ${fr_opengl}
      ${fr_iokit}
      )
  endif()

endif()

if(UNIX AND NOT APPLE)

  list(APPEND videocapture_sources
    ${sd}/videocapture/linux/V4L2_Capture.cpp
    ${sd}/videocapture/linux/V4L2_Types.cpp
    ${sd}/videocapture/linux/V4L2_Utils.cpp
    )
 
  if (USE_DECKLINK)
    list(APPEND videocapture_sources
      ${EXTERN_INC_DIR}/decklink/DeckLinkAPIDispatch.cpp
      ${sd}/videocapture/decklink/Decklink.cpp
      ${sd}/videocapture/decklink/DecklinkDevice.cpp
      )
  endif()

  list(APPEND videocapture_libraries
    udev
    )

  add_definitions(-D__STDC_CONSTANT_MACROS)

  if(USE_OPENGL)
    list(APPEND videocapture_libraries
      ${EXTERN_LIB_DIR}/libglfw3.a
      dl
      rt
      GL
      X11
      Xxf86vm
      Xrandr
      dl
      Xi
      Xcursor
      Xinerama
      udev  
      pthread
      )
    
  endif()

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

  if(USE_OPENGL)

    list(APPEND videocapture_libraries
      ${EXTERN_LIB_DIR}/glfw3.lib
      Opengl32.lib
      ws2_32.lib
      psapi.lib
      iphlpapi.lib
      )
  endif()
 
endif()

include_directories(${videocapture_include_dirs})
add_library(videocapture ${videocapture_sources})

# Basic API example
if(USE_OPENGL)

  # Plain OpenGL example
  # add_executable(opengl_example ${sd}/opengl_example.cpp) 
  # target_link_libraries(opengl_example ${videocapture_libraries} videocapture)
  # add_dependencies(opengl_example videocapture)
  # install(TARGETS opengl_example RUNTIME DESTINATION bin)

  # Using the simple wrapper
  add_executable(easy_opengl_example ${sd}/easy_opengl_example.cpp ${gl_sources}) 
  target_link_libraries(easy_opengl_example ${videocapture_libraries} videocapture)
  add_dependencies(easy_opengl_example videocapture)
  install(TARGETS easy_opengl_example RUNTIME DESTINATION bin)

endif()

add_executable(api_example ${sd}/api_example.cpp)
target_link_libraries(api_example ${videocapture_libraries} videocapture)
install(TARGETS api_example RUNTIME DESTINATION bin)

install(DIRECTORY ${bd}/include/videocapture DESTINATION include)
install(TARGETS videocapture ARCHIVE DESTINATION lib)

add_executable(decklink_example ${sd}/decklink_example.cpp)
target_link_libraries(decklink_example ${videocapture_libraries} videocapture)
install(TARGETS decklink_example RUNTIME DESTINATION bin)

