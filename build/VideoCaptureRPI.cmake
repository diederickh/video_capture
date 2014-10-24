set(bd ${CMAKE_CURRENT_LIST_DIR}/..)
set(sd ${bd}/src)
set(id ${bd}/include/)

set(videocapture_include_dirs 
  ${id}
  ${EXTERN_INC_DIR}
  ${TINYLIB_DIR}/src/
  )

set(videocapture_sources 
  ${sd}/videocapture/Base.cpp
  ${sd}/videocapture/Capture.cpp
  ${sd}/videocapture/Types.cpp
  ${sd}/videocapture/Utils.cpp
  ${sd}/videocapture/linux/V4L2_Capture.cpp
  ${sd}/videocapture/linux/V4L2_Types.cpp
  ${sd}/videocapture/linux/V4L2_Utils.cpp
)

list(APPEND videocapture_libraries
  udev
  dl
  rt
  dl
  udev  
  pthread
  )

# Video Capture library
include_directories(${videocapture_include_dirs})
add_library(videocapture ${videocapture_sources})
install(DIRECTORY ${bd}/include/videocapture DESTINATION include)
install(TARGETS videocapture ARCHIVE DESTINATION lib)

# API example
add_executable(api_example ${sd}/api_example.cpp)
target_link_libraries(api_example ${videocapture_libraries} videocapture)
install(TARGETS api_example RUNTIME DESTINATION bin)