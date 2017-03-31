cmake_minimum_required(VERSION 3.5)
project(Http)
FIND_PACKAGE (Threads)

#ADD_DEFINITIONS(-D_Linux)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -pthread")
#set(SOURCE_FILES event.c http.c epoll.c server.c select.c)
set(SOURCE_FILES event.c http.c kqueue.c server.c)

add_subdirectory(contianer)

add_executable(Http ${SOURCE_FILES})
TARGET_LINK_LIBRARIES(Http pthread)