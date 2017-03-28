cmake_minimum_required(VERSION 3.6)
project(lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES
    lib.h
    main.c
    map.c
    vector.c)

add_executable(lib ${SOURCE_FILES})