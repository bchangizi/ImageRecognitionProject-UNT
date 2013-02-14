#Required minimum cmake version
cmake_minimum_required( VERSION 2.6)
#Declare our project name, use C++
project(ImageSelect CXX)

#Include OPENCV src files
include_directories( $ENV{OPENCV}/include )
link_directories(  $ENV{OPENCV}/lib )

set(external_libs opencv_core243d.lib opencv_highgui243d.dll opencv_video243d.dll opencv_ml243d.dll opencv_legacy243d.dll opencv_imgproc243d.dll)

#Define sources used to compile our project
add_executable(ImageSelect selector.cpp)
target_link_libraries(ImageSelect ${external_libs} )