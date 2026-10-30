cmake_minimum_required(VERSION 3.22)

add_library(embylog STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/embylog.c
)

target_include_directories(embylog 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# add custom command
# add custom target
