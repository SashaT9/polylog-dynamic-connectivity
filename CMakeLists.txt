cmake_minimum_required(VERSION 3.16)
project(polylog_dynamic_connectivity LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra)

add_subdirectory(src)

add_executable(stress_test stress_test.cpp)
target_link_libraries(stress_test PRIVATE connectivity)
