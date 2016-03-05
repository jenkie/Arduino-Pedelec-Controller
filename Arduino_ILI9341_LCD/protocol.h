/*
ILI9341 LCD Display controller for Arduino_Pedelec_Controller

Copyright (C) 2016
Andreas Butti, andreas.b242 at gmail dot com

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#pragma once

/**
 * Protocol definition or Display UART
 * Because Software UART only supports 9600 Baud (1200 Byte/s) the Protocoll is
 * binary and size optimized
 *
 * The first byte is the command, followed by data bytes
 * Length is defined by command
 */

// Command ranges
// 0 to DISP_CMD_1_BYTE_RANGE are 1 byte commands
#define DISP_CMD_1_BYTE_RANGE 50
// DISP_CMD_1_BYTE_RANGE +1 to DISP_CMD_2_BYTE_RANGE are 2 byte commands
#define DISP_CMD_2_BYTE_RANGE 100


// *** One byte commands ***
// Turn icon on / off, 8 bit bitmask
#define DISP_CMD_STATES 1
#define DISP_BIT_STATE_BLUETOOTH (1 << 0)
#define DISP_BIT_STATE_BRAKE     (1 << 1)
#define DISP_BIT_STATE_LIGHT     (1 << 2)

// *** Two byte commands ***

// Battery Voltage in 0.1V
#define DISP_CMD_BATTERY 51

// Battery Voltage in 0.1V when fully charged
#define DISP_CMD_BATTERY_MAX 52

// Battery Voltage in 0.1V when fully empty
#define DISP_CMD_BATTERY_MIN 53

// Speed in 0.1km/h
#define DISP_CMD_SPEED 54

// current Wattage in 1W
#define DISP_CMD_WATTAGE 55




