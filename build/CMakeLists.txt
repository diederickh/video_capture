cmake_minimum_required(VERSION 2.8)

# You should define the following variables when calling cmake:
#
#  - EXTERN_LIB_DIR      Path to the directory where we can find e.g. libpng, libz, etc..
#  - EXTERN_INC_DIR      Path to the directory where we can find general headers e.g. png.h 
#  - EXTERN_SRC_DIR      Path to the directory where we can find sources for dependencies (e.g. glad.c) 
#  - TINYLIB_DIR         Path to tinylib base dir. 
#
#
# DeckLink
# ========
#    - You need to download the DeckLink SDK and copy the include dir from
#      the SDK to extern/*/include/decklink/
#    - On windows we generate a .c and .h file using a custom target.  


project(videocapture)

include(ExternalProject)

option(USE_IOS "Build an iOS library" ON)
option(USE_IOS_SIMULATOR "Build for the iOS simulator." OFF)
option(USE_OPENGL "Create OpenGL examples" OFF)
option(USE_DECKLINK "Use Decklink capture card." OFF)

message(STATUS "USE_IOS: ${USE_IOS}")
message(STATUS "USE_IOS_SIMULATOR: ${USE_IOS_SIMULATOR}")
message(STATUS "USE_OPENGL: ${USE_OPENGL}")
message(STATUS "USE_DECKLINK: ${USE_DECKLINK}")


if (USE_IOS) 

  option(USE_IOS_DEVICE "Build an iOS library for a iPhone" ON)

  if (USE_IOS_SIMULATOR)
    set(ios_sysroot "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/")
    set(CMAKE_OSX_SYSROOT "${ios_sysroot}SDKs/iPhoneSimulator7.1.sdk/")
    set(CMAKE_OSX_ARCHITECTURES "i386")
    set(CMAKE_C_COMPILER ${ios_sysroot}/usr/bin/gcc)
    set(CMAKE_CXX_COMPILER ${ios_sysroot}/usr/bin/g++)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-abi-version=2")
    include_directories(CMAKE_OSX_SYSROOT "${io_sysroot}/Developer/SDKs/iPhoneSimulator7.1.sdk/usr/include/")
  endif()
  
  if (USE_IOS_DEVICE)
    set(ios_root  "/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/")
    set(CMAKE_OSX_SYSROOT "${ios_root}SDKs/iPhoneOS8.1.sdk/")
    set(CMAKE_OSX_ARCHITECTURES "armv7")
    set(CMAKE_C_COMPILER /usr/bin/clang)
    set(CMAKE_CXX_COMPILER /usr/bin/clang++)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-abi-version=2 -fvisibility=hidden -fvisibility-inlines-hidden -isysroot ${ios_sysroot}")
    include_directories(${ios_root}SDKs/iPhoneOS8.1.sdk/usr/include)
    message(STATUS "INC: ${CMAKE_OSX_SYSROOT}/usr/include")
  endif()

endif()

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

if (USE_DECKLINK)

  list(APPEND videocapture_sources
    ${sd}/videocapture/decklink/Decklink.cpp
    ${sd}/videocapture/decklink/DecklinkDevice.cpp
    ${sd}/videocapture/decklink/DecklinkCallback.cpp
    )

  add_definitions(
    -DUSE_DECKLINK=1
    )

endif()

if(APPLE)

  list(APPEND videocapture_sources
    ${sd}/videocapture/mac/AVFoundation_Capture.cpp
    ${sd}/videocapture/mac/AVFoundation_Implementation.mm
    )

  find_library(fr_core_foundation CoreFoundation)
  find_library(fr_avfoundation AVFoundation)
  find_library(fr_core_video CoreVideo)
  find_library(fr_core_media CoreMedia)
  
  list(APPEND videocapture_libraries
    ${fr_core_foundation}
    ${fr_avfoundation}
    ${fr_core_video}
    ${fr_core_media}
    )

  if (NOT USE_IOS)
    find_library(fr_cocoa Cocoa)
    list(APPEND video_capture_libraries
      ${fr_cocoa}
      )
  endif()

  if(USE_OPENGL AND NOT USE_IOS)

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
 
  list(APPEND videocapture_libraries
    udev
    )

  if (USE_DECKLINK)
    list(APPEND videocapture_sources
      ${EXTERN_INC_DIR}/decklink/DeckLinkAPIDispatch.cpp
      )
  endif()

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

  if (USE_DECKLINK)

    if (NOT EXISTS ${EXTERN_INC_DIR}/decklink/DeckLinkAPI.idl)
      message(FATAL_ERROR "Cannot find the decklink api. Copy the SDK include files to ${EXTERN_INC_DIR}/decklink/")
    endif()

    # midl.exe only works when the output directory (-out) actually exists
    #set(vd_sdk_dir ${CMAKE_CURRENT_BINARY_DIR}/)
    add_custom_target(DeckLinkAPI
      COMMAND midl.exe -nologo -W1 -char signed -env win32 -out "${CMAKE_CURRENT_BINARY_DIR}" -h ${EXTERN_INC_DIR}/decklink/DeckLinkAPI.h /iid DeckLinkAPI_i.c ${EXTERN_INC_DIR}/decklink/DeckLinkAPI.idl
      )

    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c PROPERTIES GENERATED TRUE)

    list(APPEND videocapture_sources ${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c)

    list(APPEND videocapture_libraries comsuppw.lib)

    include_directories(${CMAKE_CURRENT_BINARY_DIR})
      
  endif() 

  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/DeckLinkAPI_i.c PROPERTIES GENERATED TRUE)

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
install(DIRECTORY ${bd}/include/videocapture DESTINATION include)
install(TARGETS videocapture ARCHIVE DESTINATION lib)

if (USE_DECKLINK)
  add_dependencies(videocapture DeckLinkAPI)
endif()

# Basic API example
if(USE_OPENGL AND NOT USE_IOS)

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

if (NOT USE_IOS)
  add_executable(api_example ${sd}/api_example.cpp)
  target_link_libraries(api_example ${videocapture_libraries} videocapture)
  install(TARGETS api_example RUNTIME DESTINATION bin)
endif()


if (USE_DECKLINK)
  add_executable(decklink_example ${sd}/decklink_example.cpp)
  target_link_libraries(decklink_example ${videocapture_libraries} videocapture)
  install(TARGETS decklink_example RUNTIME DESTINATION bin)
endif()

