cmake_minimum_required( VERSION 3.5 )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wextra -Werror")

project(team114-robot-code)
add_subdirectory(c2018)
add_subdirectory(third_party)
