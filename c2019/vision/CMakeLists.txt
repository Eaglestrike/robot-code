project(c2019-vision)
# add_subdirectory(cam-calibration/interactive-calibration)

find_package( OpenCV 3.3 REQUIRED )



include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(c2019-vision src/main src/mjpeg_stream.cpp)
target_link_libraries(c2019-vision ${OpenCV_LIBS})
target_link_libraries(c2019-vision cscore)
target_link_libraries(c2019-vision copcomp)

set_target_properties(c2019-vision
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

add_executable(del-mar-cams src/del_mar_quick_cams.cpp)
target_link_libraries(del-mar-cams ${OpenCV_LIBS})
target_link_libraries(del-mar-cams cscore)

set_target_properties(del-mar-cams
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
