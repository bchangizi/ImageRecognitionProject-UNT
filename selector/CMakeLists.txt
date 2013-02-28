#Required minimum cmake version
cmake_minimum_required( VERSION 2.6)
#Declare our project name, use C++
project(ImageSelect CXX)

#Include OPENCV src files
include_directories( $ENV{OPENCV}/include )
link_directories(  $ENV{OPENCV}/lib )

IF(WIN32)
	SET(external_libs opencv_core243d.lib opencv_highgui243d.dll opencv_video243d.dll opencv_ml243d.dll opencv_legacy243d.dll opencv_imgproc243d.dll)
ENDIF(WIN32)

IF(CMAKE_COMPILER_IS_GNUCXX)
	SET(external_libs opencv_core opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_features2d opencv_calib3d opencv_objdetect opencv_contrib opencv_legacy opencv_flann)
	set(CMAKE_BUILD_TYPE Debug)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#Define sources used to compile our project
add_executable(ImageSelect selector.cpp)
target_link_libraries(ImageSelect ${external_libs} )
