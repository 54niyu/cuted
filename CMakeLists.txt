cmake_minimum_required(VERSION 3.5)
project(Http)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

set(SOURCE_FILES http.c server.c)
add_executable(Http ${SOURCE_FILES})