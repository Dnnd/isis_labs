cmake_minimum_required(VERSION 3.13)
project(test C)

set(CMAKE_C_STANDARD 11)
add_compile_definitions(USERSPACE)
file(COPY "pci.ids" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_executable(test main.c pci_device_info.h pci_list_def.h) 
