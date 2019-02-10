project(copcomp CXX)


file(GLOB_RECURSE copcomp_src src/*.cpp)

add_library(copcomp ${cop_comp_src})
target_include_directories(copcomp INTERFACE include/)
set_target_properties(copcomp PROPERTIES LINKER_LANGUAGE CXX)
# target_link_libraries(copcomp socket_int)
target_link_libraries(copcomp socket++)


# set_target_properties(c2019-vision
#     PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
# )
