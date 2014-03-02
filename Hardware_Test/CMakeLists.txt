#=============================================================================#
# Author: Thomas Jarosch                                                      #
# Date:   24.04.2012                                                          #
#                                                                             #
# Description: Pedelec controller cmake project                               #
#                                                                             #
#=============================================================================#
include_directories(BEFORE ${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}/Hardware_Test)

configure_file(Hardware_Test.ino testprogram.cpp COPYONLY)

# Hardware test program
set(TESTPROGRAM_NAME testprogram)
set(${TESTPROGRAM_NAME}_BOARD atmega328)              # Target board: Arduino nano
set(${TESTPROGRAM_NAME}_PORT /dev/ttyUSB0)            # Serial upload port
set(${TESTPROGRAM_NAME}_SRCS                          # Source code
    ${CMAKE_CURRENT_BINARY_DIR}/testprogram.cpp
    PCD8544_charset.cpp
    PCD8544_nano.cpp
    )
generate_arduino_firmware(${TESTPROGRAM_NAME})

# Dirty hack to solve compile error on Fedora 15
#target_link_libraries(${TESTPROGRAM_NAME} m)
#target_link_libraries(${TESTPROGRAM_NAME} c)
#target_link_libraries(${TESTPROGRAM_NAME} m)
