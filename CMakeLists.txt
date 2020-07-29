#=============================================================================#
# Author: Thomas Jarosch                                                      #
# Date:   24.04.2012                                                          #
#                                                                             #
# Description: Pedelec controller cmake project                               #
#                                                                             #
#=============================================================================#
set(CMAKE_TOOLCHAIN_FILE cmake/Arduino-toolchain.cmake) # Arduino Toolchain

# "grep" for FC 2.0 hardware in config.h
# Do this before project() since it invokes toolchain detection
file(STRINGS Arduino_Pedelec_Controller/config.h config_h_lines REGEX "^#define HARDWARE_REV 2[0-9]")
if (config_h_lines)
    message("Building firmware for FC 2.0 or higher")
    set(ARDUINO_BOARD "Arduino Mega or Mega 2560 [avr.mega]")
    set(ARDUINO_AVR_MEGA_MENU_CPU_ATMEGA2560 TRUE)
else(config_h_lines)
    set(ARDUINO_BOARD "Arduino Nano [avr.nano]")
    set(ARDUINO_AVR_NANO_MENU_CPU_ATMEGA328 TRUE)
endif(config_h_lines)

cmake_minimum_required(VERSION 3.7.0)
project(Pedelec_Controller C CXX)

# Documentation
option(DOCUMENTATION "Generate API documentation with Doxygen" OFF)

find_package(Doxygen)
if(DOCUMENTATION AND DOXYGEN_FOUND)

   # Set variables
   set(top_srcdir ${CMAKE_SOURCE_DIR})

   # Find doxy config
   message(STATUS "Doxygen found.")
   set(DOXY_DIR "${CMAKE_SOURCE_DIR}/docs")
   set(DOXY_CONFIG "${DOXY_DIR}/Doxyfile.in")

   # Copy doxy.config.in
   configure_file("${DOXY_CONFIG}" "${CMAKE_BINARY_DIR}/doxy.config")

   # Create doc directory
   add_custom_command(
   OUTPUT ${CMAKE_BINARY_DIR}/doc
   COMMAND rm -rf ${CMAKE_BINARY_DIR}/doc/{html,man}
   COMMAND mkdir -p ${CMAKE_BINARY_DIR}/doc
   DEPENDS pcontroller
   )

   # Run doxygen
   add_custom_command(
   OUTPUT ${CMAKE_BINARY_DIR}/doc/html/index.html
   COMMAND ${DOXYGEN_EXECUTABLE} "${CMAKE_BINARY_DIR}/doxy.config"
   DEPENDS "${CMAKE_BINARY_DIR}/doxy.config" "${CMAKE_BINARY_DIR}/doc"
   WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/doc
   )

   add_custom_target(docs ALL DEPENDS ${CMAKE_BINARY_DIR}/doc/html/index.html)

   message(STATUS "Generating API documentation with Doxygen")
else(DOCUMENTATION AND DOXYGEN_FOUND)
   message(STATUS "Not generating API documentation")
endif(DOCUMENTATION AND DOXYGEN_FOUND)

add_subdirectory(Arduino_Pedelec_Controller)
add_subdirectory(Hardware_Test)
