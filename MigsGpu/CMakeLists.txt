cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)

project(MigsGpu)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(
    MigsGpu
    src/main.cpp
)
include_directories(
    include
)

target_link_libraries(
    MigsGpu pico_stdlib pico_multicore pico_scanvideo_dpi
)

pico_enable_stdio_usb(MigsGpu 1)
pico_enable_stdio_uart(MigsGpu 0)

pico_add_extra_outputs(MigsGpu)
