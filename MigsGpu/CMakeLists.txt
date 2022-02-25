cmake_minimum_required(VERSION 3.12)

include(pico_sdk_import.cmake)

project(MigsGpu)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_compile_options(-Wall)

add_executable(
    MigsGpu

    src/main.cpp

    libdvi/dvi.c
    libdvi/dvi_serialiser.c
    libdvi/dvi_timing.c
    libdvi/tmds_encode.c
    libdvi/tmds_encode.S
)

pico_generate_pio_header(MigsGpu ${CMAKE_CURRENT_LIST_DIR}/libdvi/dvi_serialiser.pio)
pico_generate_pio_header(MigsGpu ${CMAKE_CURRENT_LIST_DIR}/libdvi/tmds_encode_1bpp.pio)

include_directories(
    include libdvi
)

target_link_libraries(
    MigsGpu

    pico_stdlib
    pico_multicore
    pico_util

    hardware_dma
    hardware_irq
    hardware_pio
    hardware_interp
    hardware_pwm
    hardware_gpio
    hardware_sync
)

pico_add_extra_outputs(MigsGpu)
