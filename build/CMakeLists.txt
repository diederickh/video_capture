cmake_minimum_required(VERSION 2.8)

project(videocapture)

include(${CMAKE_CURRENT_LIST_DIR}/Triplet.cmake)

option(USE_OPENGL "Create OpenGL examples" ON)

set(bd ${CMAKE_CURRENT_LIST_DIR}/..)
set(sd ${bd}/src)
set(id ${bd}/include/)

if(NOT EXISTS ${bd}/extern)
  message(STATUS "Downloading opengl dependencies")
  file(DOWNLOAD https://github.com/roxlu/binaries/blob/master/video_capture/extern_video_capture_0.0.0.1.zip?raw=true ${bd}/extern/extern.zip SHOW_PROGRESS TIMEOUT 5)
  execute_process(COMMAND
     ${CMAKE_COMMAND} -E tar xzf ${bd}/extern/extern.zip
     WORKING_DIRECTORY ${bd}/
     )
endif()

set(CMAKE_INSTALL_PREFIX ${bd}/install/${tri_triplet}/)

set(videocapture_sources 
  ${sd}/videocapture/Base.cpp
  ${sd}/videocapture/Capture.cpp
  ${sd}/videocapture/Types.cpp
  ${sd}/videocapture/Utils.cpp
)

set(videocapture_include_dirs 
  ${id}
  ${extern_include_dir}
  ${bd}/shared/tinylib/src/
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
      ${extern_lib_dir}/libglfw3.a
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

  list(APPEND videocapture_libraries
    udev
    )

  add_definitions(-D__STDC_CONSTANT_MACROS)

  if(USE_OPENGL)
    list(APPEND videocapture_libraries
      ${extern_lib_dir}/libglfw3.a
      GL
      X11
      Xxf86vm
      Xrandr
      pthread
      dl
      Xi
      )

    list(APPEND videocapture_sources
      ${extern_source_dir}/GLXW/glxw.c
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
    list(APPEND videocapture_sources
      ${extern_source_dir}/GLXW/glxw.c
      )

    list(APPEND videocapture_libraries
      ${extern_lib_dir}/glfw3.lib
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
# add_executable(api_example ${sd}/api_example.cpp)
# target_link_libraries(api_example ${videocapture_libraries} videocapture)
# add_dependencies(api_example video_capture)
# install(TARGETS api_example RUNTIME DESTINATION bin)

if(USE_OPENGL)
  # Plain OpenGL example
  # add_executable(opengl_example ${sd}/opengl_example.cpp) 
  # target_link_libraries(opengl_example ${videocapture_libraries} videocapture)
  # add_dependencies(opengl_example videocapture)
  # install(TARGETS opengl_example RUNTIME DESTINATION bin)
endif()

# Using the simple wrapper
add_executable(easy_opengl_example ${sd}/easy_opengl_example.cpp) 
target_link_libraries(easy_opengl_example ${videocapture_libraries} videocapture)
add_dependencies(easy_opengl_example videocapture)
install(TARGETS easy_opengl_example RUNTIME DESTINATION bin)

install(DIRECTORY ${bd}/include/videocapture DESTINATION include)
install(TARGETS videocapture ARCHIVE DESTINATION lib)

