cmake_minimum_required(VERSION 3.22)
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
