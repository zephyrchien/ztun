cmake_minimum_required(VERSION 3.10)
project(ztun)

file(GLOB_RECURSE CXX_SRC "src/*.cc")

add_executable(ztun ${CXX_SRC})
target_include_directories(ztun PRIVATE src)

# getaddrinfo_a requires libanl
find_library(LIBANL anl)

if(LIBANL)
    target_link_libraries(ztun PRIVATE anl)
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
