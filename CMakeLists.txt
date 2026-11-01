cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_FLAGS  "-Wall")
set(CMAKE_SYSTEM_NAME Generic)

set( CMAKE_SYSTEM_NAME      "Generic"   )
set( CMAKE_SYSTEM_PROCESSOR "Arm Cortex-M/Cortex-R" )

set(CMAKE_SYSTEM_VERSION "0.0.0")
set(CMAKE_CXX_FLAGS  "-Wall")
project(embylog)

add_library(embylog STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/embylog.c
)

target_include_directories(embylog 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# add custom command
# add custom target


add_executable(embylog-example
  ${CMAKE_SOURCE_DIR}/example/main.c
)
target_link_libraries(embylog-example
  PUBLIC embylog
)
