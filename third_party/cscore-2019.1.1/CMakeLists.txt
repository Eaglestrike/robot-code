project(cscore)

find_package( OpenCV 3.4 REQUIRED )

file(GLOB
    cscore_native_src src/main/native/cpp/*.cpp)
file(GLOB cscore_linux_src src/main/native/linux/*.cpp)
file(GLOB cscore_osx_src src/main/native/osx/*.cpp)
file(GLOB cscore_windows_src src/main/native/windows/*.cpp)

add_library(cscore ${cscore_native_src})
set_target_properties(cscore PROPERTIES DEBUG_POSTFIX "d")

if(NOT MSVC)
    if (APPLE)
        target_sources(cscore PRIVATE ${cscore_osx_src})
    else()
        target_sources(cscore PRIVATE ${cscore_linux_src})
    endif()
else()
    target_sources(cscore PRIVATE ${cscore_windows_src})
    target_compile_options(cscore PUBLIC -DNOMINMAX)
    target_compile_options(cscore PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

target_include_directories(cscore PUBLIC
                $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/main/native/include>
                            $<INSTALL_INTERFACE:${include_dest}/cscore>)
target_include_directories(cscore PRIVATE src/main/native/cpp)
target_link_libraries(cscore PUBLIC wpiutil ${OpenCV_LIBS})

set_property(TARGET cscore PROPERTY FOLDER "libraries")

if (MSVC)
    set (cscore_config_dir ${wpilib_dest})
else()
    set (cscore_config_dir share/cscore)
endif()
