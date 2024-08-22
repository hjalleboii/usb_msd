cmake_minimum_required(VERSION 3.12)
#include(littlefs-lib/CMakeLists.txt)


include(pico_sdk_import.cmake)

project(pico_msc C CXX ASM)
pico_sdk_init()

#add_subdirectory(layouts)
add_executable(pico_msc main.cpp usb_descriptors.c msc_disk_dual.c fat12/FAT12.cpp )
target_include_directories(pico_msc PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/fat12
)

target_link_libraries(pico_msc 
PUBLIC

pico_stdlib
tinyusb_device
tinyusb_board
)


pico_add_extra_outputs(pico_msc)

