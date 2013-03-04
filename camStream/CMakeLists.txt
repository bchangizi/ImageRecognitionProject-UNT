cmake_minimum_required(VERSION 2.6)

project(camstream CXX)

set( libraries )

include_directories( $ENV{OPENCV}/include )
link_directories(  $ENV{OPENCV}/lib )

IF(WIN32)
	SET(external_libs opencv_core243d.lib opencv_highgui243d.lib opencv_video243d.lib opencv_ml243d.lib opencv_legacy243d.lib opencv_imgproc243d.lib)
ENDIF(WIN32)

IF(CMAKE_COMPILER_IS_GNUCXX)
	SET(external_libs opencv_core opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_features2d opencv_calib3d opencv_objdetect opencv_contrib opencv_legacy opencv_flann)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

#Define sources used to compile our project
add_executable(camstream camStream.cpp)
target_link_libraries(camstream ${external_libs} )