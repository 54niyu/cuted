cmake_minimum_required(VERSION 2.5)
project(Http)
FIND_PACKAGE (Threads)

#ADD_DEFINITIONS(-D_Linux)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -pthread")

if(APPLE)
    MESSAGE(STATUS "Now is apple")
    set(SOURCE_FILES event.c http.c kqueue.c server.c)
else(LINUX)
    MESSAGE(STATUS "Now is linux")
    set(SOURCE_FILES event.c http.c epoll.c server.c timer.c signal.c)
endif(APPLE)

#add_subdirectory(contianer)
add_executable(Http ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(Http pthread)
