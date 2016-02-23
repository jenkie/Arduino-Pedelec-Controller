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

set(${FIRMWARE_NAME}_BOARD ${FC_PROCESSOR})
set(${FIRMWARE_NAME}_PORT /dev/ttyUSB0)            # Serial upload port

# Hack until we replace the .ino file with a stub
# and move everything to main.cpp
configure_file(Arduino_Pedelec_Controller.ino main.cpp COPYONLY)

set(${FIRMWARE_NAME}_SRCS                          # Source code
    ${CMAKE_CURRENT_BINARY_DIR}/main.cpp
    BMP085.cpp
    BMP085.h
    DSPC01_nano.cpp
    DSPC01_nano.h
    DallasTemp.cpp
    DallasTemp.h
    HX711.cpp
    HX711.h
    LiquidCrystalDogm.cpp
    LiquidCrystalDogm.h
    MenuSystem.cpp
    MenuSystem.h
    PCD8544_charset.cpp
    PCD8544_nano.cpp
    PCD8544_nano.h
    PID_v1_nano.cpp
    PID_v1_nano.h
    config.h
    display.cpp
    display.h
    display_backlight.cpp
    display_backlight.h
    display_kingmeter.cpp
    display_kingmeter.h
    ds1307.cpp
    ds1307.h
    globals.h
    hrmi_funcs.cpp
    hrmi_funcs.h
    menu.cpp
    menu.h
    one_Wire.cpp
    one_Wire.h
    serial_command.cpp
    serial_command.h
    serial_lcd.cpp
    serial_lcd.h
    switches.cpp
    switches.h
    switches_action.h
    )
generate_arduino_firmware(${FIRMWARE_NAME})

# Dirty hack to solve compile error on Fedora 15
#target_link_libraries(${FIRMWARE_NAME} m)
#target_link_libraries(${FIRMWARE_NAME} c)
#target_link_libraries(${FIRMWARE_NAME} m)
