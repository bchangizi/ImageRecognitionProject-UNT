#Required minimum cmake version
cmake_minimum_required( VERSION 2.6)
#Declare our project name, use C++
project(ImageSelect CXX)

#Include OPENCV src files
include_directories( $ENV{OPENCV}/include )
link_directories(  $ENV{OPENCV}/lib )

IF(WIN32)
	SET(external_libs opencv_core243d.lib opencv_highgui243d.lib opencv_video243d.lib opencv_ml243d.lib opencv_legacy243d.lib opencv_imgproc243d.lib opencv_features2d243d.lib opencv_nonfree243d.lib opencv_calib3d243d.lib opencv_objdetect243d.lib opencv_flann243d.lib)
ENDIF(WIN32)

IF(CMAKE_COMPILER_IS_GNUCXX)
	include_directories( /usr/include/qt4 /usr/include/qt4/QtCore)
	SET(external_libs opencv_core opencv_imgproc opencv_highgui opencv_ml opencv_video opencv_features2d opencv_nonfree opencv_calib3d opencv_objdetect opencv_contrib opencv_legacy opencv_flann)
	set(CMAKE_BUILD_TYPE Debug)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)

FILE(GLOB source_files ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
source_group( "Headers" FILES )
#Define sources used to compile our project
add_executable(ImageSelect selector.cpp )
target_link_libraries(ImageSelect ${external_libs} )
