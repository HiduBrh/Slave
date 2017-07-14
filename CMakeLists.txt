cmake_minimum_required(VERSION 3.6)
project(Slave)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(Slave ${SOURCE_FILES})