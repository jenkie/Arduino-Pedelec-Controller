/*
Serial LCD function. Currently for New Haven display only.
Written by Thomas Jarosch in 2014

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

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include "serial_lcd.h"

#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_SERIAL)

/**
 * @brief Constructor. No data is written to the display yet
 *
 * @param display_pin The pin the display is connected to
 */
SerialLCD::SerialLCD(const byte &display_pin)
    : SoftwareSerial(0, display_pin)
    , DisplayPin(display_pin)
{
}

/**
 * @brief Initialize display. Clear screen, enable backlight etc.
 *
 */
void SerialLCD::init()
{
    pinMode(DisplayPin, OUTPUT);
    begin(9600);

    // Controller on the display needs 100ms init time
    delay(100);

    // Clear screen
    clear();

    // Set contrast
    setContrast(50);

    // Enable display
    display(true);
}

/**
 * @brief Clear screen and move cursor home
 *
 */
void SerialLCD::clear()
{
    write(0xFE);
    write(0x51);

    // As clear() also clears the receive buffer
    // and takes 1.5ms to execute, we wait for 2 millis
    delay(2);
}

/**
 * @brief Move cursor home to (0,0)
 *
 */
void SerialLCD::home()
{
    write(0xFE);
    write(0x46);
}

/**
 * @brief Set display contrast
 * 
 * @param contrast Value between 1 and 50
 */
void SerialLCD::setContrast(const byte &contrast)
{
    write(0xFE);
    write(0x52);
    write(contrast);
}

/**
 * @brief Set backlight brightness
 * 
 * @param brightness Backlight brightness. Value between 1 and 8. '1' is off.
 */
void SerialLCD::setBacklight(const byte& brightness)
{
    write(0xFE);
    write(0x53);
    write(brightness);
}

/**
 * @brief Set cursor x/y position. Use an out-of-screen position to hide the cursor
 * 
 * @param x Horizontal position, starting at 0
 * @param y Line, starting at 0
 */
void SerialLCD::setCursor(const byte& x, const byte& y)
{
    write(0xFE);
    write(0x45);
    write(y * 0x40 + x);
}

void SerialLCD::noDisplay(const bool &disable_backlight)
{
    write(0xFE);
    write(0x42);

    if (disable_backlight)
        setBacklight(1);
}

void SerialLCD::display(const bool &enable_backlight)
{
    write(0xFE);
    write(0x41);

    if (enable_backlight)
        setBacklight(8);
}

/**
 * @brief Load custom character
 * Custom characters can be printed via ASCII code 0 to 8
 *
 * @param address Character value to store this. Valid values are 0 to 8
 * @param data Character bitmap data. A character is 5x8 pixels.
 */
void SerialLCD::createChar(byte address, const byte data[])
{
    address = address & 0x7;

    write(0xFE);
    write(0x54),
    write(address);

    for (byte i = 0; i < 8; ++i)
        write(data[i]);
}

#endif
