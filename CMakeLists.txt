cmake_minimum_required( VERSION 3.5 )

add_compile_options(-Wall -pedantic -Wextra -Werror)

project(team114-robot-code)
add_subdirectory(third-party)
add_subdirectory(first-party)
add_subdirectory(c2018)
