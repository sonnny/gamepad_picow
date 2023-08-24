cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(gamepad_host C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_library(gamepad_host
  src/dualsense_hci.cpp
  src/dualsense_l2cap.cpp
  src/dualsense_parser.cpp
  src/pico_dualsense.cpp
  src/ht16k33.c
  src/ws2812.c
)

target_include_directories(gamepad_host PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include )
target_include_directories(gamepad_host PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src )

pico_generate_pio_header(gamepad_host ${CMAKE_CURRENT_LIST_DIR}/src/ws2812.pio)

target_link_libraries(gamepad_host
  pico_stdlib
  pico_btstack_ble
  pico_btstack_classic
  hardware_i2c
  hardware_pio
  hardware_clocks
)
