project (ztun)
cmake_minimum_required (VERSION 3.10)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -O2")

set (target "ztun")
set (utils "utils")
set (endpoint "endpoint")

aux_source_directory("src" source_main)
aux_source_directory("src/utils" source_utils)
aux_source_directory("src/endpoint" source_endpoint)

add_library (${utils} STATIC ${source_utils})
add_library (${endpoint} STATIC ${source_endpoint})
add_executable (${target} ${source_main})

include_directories (PUBLIC "src/include")

target_link_libraries (${endpoint} ${utils})
target_link_libraries (${target} ${utils} ${endpoint})