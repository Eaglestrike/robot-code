cmake_minimum_required( VERSION 3.5 )

add_library(GoldFish INTERFACE)
file(GLOB_RECURSE headers inc/*.hpp)
target_sources(GoldFish INTERFACE ${headers})
target_include_directories(GoldFish INTERFACE inc/)
