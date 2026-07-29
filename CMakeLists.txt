cmake_minimum_required(VERSION 3.28)
project(cpu)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SOURCES "src/*.cpp")
add_executable(cpu main.cpp ${SOURCES})
target_include_directories(cpu PRIVATE include)