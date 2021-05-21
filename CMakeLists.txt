project (ztun)
cmake_minimum_required (VERSION 3.0)
set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -O2")

set (target "ztun")
set (utils "utils")
set (event "event")
set (buffer "buffer")
set (endpoint "endpoint")

aux_source_directory ("src" source_main)
aux_source_directory ("src/utils" source_utils)
aux_source_directory ("src/event" source_event)
aux_source_directory ("src/buffer" source_buffer)
aux_source_directory ("src/endpoint" source_endpoint)

include_directories ("src")

add_library (${utils} STATIC ${source_utils})
add_library (${event} STATIC ${source_event})
add_library (${buffer} STATIC ${source_buffer})
add_library (${endpoint} STATIC ${source_endpoint})
add_executable (${target} ${source_main})

target_link_libraries (${event} ${utils})
target_link_libraries (${buffer} ${utils})
target_link_libraries (${endpoint} ${event} ${buffer})
target_link_libraries (${target} ${endpoint})