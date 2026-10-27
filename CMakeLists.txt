cmake_minimum_required(VERSION 3.20)
set(PICO_BOARD pico_w CACHE STRING "RP2040 Pico W da BitDogLab")
if(NOT PICO_BOARD STREQUAL "pico_w" AND NOT PICO_BOARD STREQUAL "pico")
  message(FATAL_ERROR "Este firmware foi preparado para RP2040 (pico_w ou pico).")
endif()
if(NOT DEFINED PICO_SDK_PATH)
  set(PICO_SDK_PATH "$ENV{PICO_SDK_PATH}")
endif()
include("${PICO_SDK_PATH}/external/pico_sdk_import.cmake")
project(bitdoglab_arcade C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()
add_executable(bitdoglab_arcade src/main.cpp)
target_include_directories(bitdoglab_arcade PRIVATE src)
target_link_libraries(bitdoglab_arcade PRIVATE pico_stdlib hardware_i2c hardware_adc hardware_pwm hardware_pio hardware_clocks)
target_compile_options(bitdoglab_arcade PRIVATE -O2 -Wall -Wextra -Wno-misleading-indentation $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions> $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
target_compile_definitions(bitdoglab_arcade PRIVATE PICO_STACK_SIZE=8192 PICO_STDIO_USB_DEFAULT_CRLF=0)
pico_enable_stdio_usb(bitdoglab_arcade 1)
pico_enable_stdio_uart(bitdoglab_arcade 0)
# So gera os formatos de dados; nao executa o picotool bloqueado pelo Windows.
set(PICO_NO_PICOTOOL 1)
pico_add_extra_outputs(bitdoglab_arcade)
add_custom_command(TARGET bitdoglab_arcade POST_BUILD
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_LIST_DIR}/tools/bin_to_uf2.py"
    "$<TARGET_FILE_DIR:bitdoglab_arcade>/bitdoglab_arcade.bin"
    "$<TARGET_FILE_DIR:bitdoglab_arcade>/bitdoglab_arcade.uf2" --replace
  VERBATIM)
target_link_options(bitdoglab_arcade PRIVATE -Wl,--print-memory-usage)
