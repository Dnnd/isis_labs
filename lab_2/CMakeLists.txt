cmake_minimum_required(VERSION 3.13)
project(lab_2 C)

set(CMAKE_C_STANDARD 11)
include_directories(${PROJECT_SOURCE_DIR})
add_executable(lab_2 country.c list.c list.h map.h map.c)
