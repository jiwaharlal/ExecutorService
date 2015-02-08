project(gcdCpp)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)
ADD_DEFINITIONS(
    -pthread
    -std=c++0x
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
#list( APPEND CMAKE_CXX_FLAGS "-std=c++0x")
add_executable(${PROJECT_NAME} ${SRC_LIST})

