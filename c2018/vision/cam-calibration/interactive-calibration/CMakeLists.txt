

cmake_minimum_required(VERSION 3.5.1)
project(cv_playground)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package( OpenCV 4.0.1 REQUIRED )

file(GLOB SRCS *.cpp)
add_executable(opencv_interactive-calibration ${SRCS} )
target_link_libraries( opencv_interactive-calibration ${OpenCV_LIBS} )
