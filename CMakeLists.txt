cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(NAME picosynth)

include(pimoroni_pico_import.cmake)
include(pico_sdk_import.cmake)

enable_language(C CXX)

pico_sdk_init()

add_subdirectory(./src)

pico_enable_stdio_usb(picosynth 1)
pico_enable_stdio_uart(picosynth 0)

project(${NAME} C CXX ASM)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)
