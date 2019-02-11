project(tinycbor)

file(GLOB tinycbor_src src/*.c)
file(GLOB tinycbor_h src/*.h)
add_library(tinycbor STATIC ${tinycbor_src} ${tinycbor_h})
target_include_directories(tinycbor INTERFACE src/)
