cmake_minimum_required(VERSION 3.3)
project(ori-ufscar-2015)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS "-std=c99")

set(SOURCE_FILES
        btree.c
        btree.h
        main.c
)
add_executable(ori-ufscar-2015 ${SOURCE_FILES})