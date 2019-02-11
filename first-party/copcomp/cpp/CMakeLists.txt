project(copcomp CXX)


file(GLOB_RECURSE copcomp_src src/*.cpp)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(copcomp ${copcomp_src})
target_include_directories(copcomp PUBLIC include/)
set_target_properties(copcomp PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(copcomp socket_int)
target_link_libraries(copcomp socket++_int)
target_link_libraries(copcomp tinycbor)



# set_target_properties(c2019-vision
#     PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
# )
