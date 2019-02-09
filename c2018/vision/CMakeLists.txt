project(c2019-vision)
add_subdirectory(cam-calibration/interactive-calibration)

find_package( OpenCV 3.3 REQUIRED )



file(GLOB_RECURSE c2019_vision_src src/*.cpp)

add_executable(c2019-vision ${c2019_vision_src})
target_link_libraries(c2019-vision ${OpenCV_LIBS})
target_link_libraries(c2019-vision cscore)
target_link_libraries(c2019-vision GoldFish)
target_link_libraries(c2019-vision socket++)

set_target_properties(c2019-vision
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)
