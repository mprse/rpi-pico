cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(app_freertos C CXX ASM)

include(pico_sdk_import.cmake)

pico_sdk_init()

add_subdirectory(freertos)
add_subdirectory(src)
