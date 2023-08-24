cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH "/home/sonny/pico/pico-sdk")
set(PICO_BOARD pico_w)

include(../pico_sdk_import.cmake)

add_subdirectory(.. gamepad_host)

project(example C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(example
  example.cpp
)

pico_enable_stdio_usb(example 1)

target_link_libraries(example
  pico_stdlib
  pico_btstack_cyw43
  pico_cyw43_arch_none
  hardware_gpio
  gamepad_host
  hardware_i2c
)

target_compile_definitions(example PRIVATE
  # WANT_HCI_DUMP=1 # This enables btstack debug
)

pico_add_extra_outputs(example)
