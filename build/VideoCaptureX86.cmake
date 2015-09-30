set(bd ${CMAKE_CURRENT_LIST_DIR}/..)
set(sd ${bd}/src)
set(id ${bd}/include/)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(debug_flag "_debug")
endif()

set(videocapture_sources 
  ${sd}/videocapture/Base.cpp
  ${sd}/videocapture/Capture.cpp
  ${sd}/videocapture/Types.cpp
  ${sd}/videocapture/Utils.cpp
  ${sd}/videocapture/CapabilityFinder.cpp
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

  if (NOT USE_IOS)
    find_library(fr_cocoa Cocoa)
    list(APPEND video_capture_libraries
      ${fr_cocoa}
      )
  endif()

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

  # Use the Udev backend to query for capture devices; otherwise use V4L2 defaults.
  find_library(libudev udev)
  if (libudev)
    list(APPEND videocapture_sources ${sd}/videocapture/linux/V4L2_Devices_Udev.cpp)
    list(APPEND videocapture_libraries
      ${libudev}
      )
  else()
    list(APPEND videocapture_sources  ${sd}/videocapture/linux/V4L2_Devices_Default.cpp) 
  endif()

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

   add_definitions(
     -DPA_WDMKS_NO_KSGUID_LIB 
     )

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
add_library(videocapture${debug_flag} ${videocapture_sources})
install(DIRECTORY ${bd}/include/videocapture DESTINATION include)
install(TARGETS videocapture${debug_flag} ARCHIVE DESTINATION lib)

if (USE_DECKLINK)
  add_dependencies(videocapture${debug_flag} DeckLinkAPI)
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
  target_link_libraries(easy_opengl_example ${videocapture_libraries} videocapture${debug_flag})
  add_dependencies(easy_opengl_example videocapture${debug_flag})
  install(TARGETS easy_opengl_example RUNTIME DESTINATION bin)

endif()

if (NOT USE_IOS)
  add_executable(api_example ${sd}/api_example.cpp)
  target_link_libraries(api_example ${videocapture_libraries} videocapture${debug_flag})
  install(TARGETS api_example RUNTIME DESTINATION bin)

  add_executable(test_conversion ${sd}/test_conversion.cpp)
  target_link_libraries(test_conversion ${videocapture_libraries} videocapture${debug_flag})
  install(TARGETS test_conversion RUNTIME DESTINATION bin)

  add_executable(test_capability_filter ${sd}/test_capability_filter.cpp)
  target_link_libraries(test_capability_filter ${videocapture_libraries} videocapture${debug_flag})
  install(TARGETS test_capability_filter RUNTIME DESTINATION bin)
      
endif()

if (UNIX AND NOT APPLE)
  
  # Experiment to retrieve device information using udev.
  add_executable(test_v4l2_devices ${sd}/test_v4l2_devices.cpp)
  target_link_libraries(test_v4l2_devices ${videocapture_libraries} videocapture${debug_flag})
  install(TARGETS test_v4l2_devices RUNTIME DESTINATION bin)

  # Test device list 
  add_executable(test_linux_device_list ${sd}/test_linux_device_list.cpp)
  target_link_libraries(test_linux_device_list ${videocapture_libraries} videocapture${debug_flag})
  install(TARGETS test_linux_device_list RUNTIME DESTINATION bin)
  
endif()

if (USE_DECKLINK)
  add_executable(decklink_example ${sd}/decklink_example.cpp)
  target_link_libraries(decklink_example ${videocapture_libraries} videocapture${debug_flag})
  install(TARGETS decklink_example RUNTIME DESTINATION bin)
endif()
