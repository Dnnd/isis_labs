cmake_minimum_required(VERSION 3.13)
project(lab_3 C)

set(CMAKE_C_STANDARD 11)

add_executable(lab_3 main.c vector.h http.h http.c vector.c api.h api.c)
