#=============================================================================#
# Author: Thomas Jarosch                                                      #
# Date:   24.04.2012                                                          #
#                                                                             #
# Description: Pedelec controller cmake project                               #
#                                                                             #
#=============================================================================#
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}/Arduino_Pedelec_Controller)

set(FIRMWARE_NAME pcontroller)

set(${FIRMWARE_NAME}_BOARD atmega328)              # Target board: Arduino nano
set(${FIRMWARE_NAME}_PORT /dev/ttyUSB0)            # Serial upload port

# Hack until we replace the .ino file with a stub
# and move everything to main.cpp
configure_file(Arduino_Pedelec_Controller.ino main.cpp COPYONLY)

set(${FIRMWARE_NAME}_SRCS                          # Source code
    BMP085.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    PCD8544_charset.cpp
    PCD8544_nano.cpp
    PID_v1_nano.cpp
    display.cpp
    display.h
    display_backlight.cpp
    display_backlight.h
    )
generate_arduino_firmware(${FIRMWARE_NAME})

# Dirty hack to solve compile error on Fedora 15
target_link_libraries(${FIRMWARE_NAME} m)
target_link_libraries(${FIRMWARE_NAME} c)
target_link_libraries(${FIRMWARE_NAME} m)
