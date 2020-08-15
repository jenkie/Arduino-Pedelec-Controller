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

add_executable(${TESTPROGRAM_NAME}
    ${CMAKE_CURRENT_BINARY_DIR}/testprogram.cpp
    PCD8544_charset.cpp
    PCD8544_nano.cpp
    )
target_link_arduino_libraries(${TESTPROGRAM_NAME} PRIVATE EEPROM core)

# Upload support: Usage:
#     make upload-testprogram SERIAL_PORT=/dev/ttyUSB0
target_enable_arduino_upload(${TESTPROGRAM_NAME})
