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
#ifndef SERIAL_LCD_H
#define SERIAL_LCD_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include "config.h"

#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_SERIAL)

#include <SoftwareSerial.h>

class SerialLCD : public SoftwareSerial
{
public:
    SerialLCD(const byte &display_pin=7);

    void init();
    void clear();

    void home();
    void setCursor(const byte &x, const byte &y);

    void setContrast(const byte &contrast);
    void setBacklight(const byte &brightness);

    void display(const bool &enable_backlight=true);
    void noDisplay(const bool &disable_backlight=true);

    void createChar(byte address, const byte data[]);

private:
    byte DisplayPin;
};

#endif //eo DISPLAY_TYPE_16X2_SERIAL
#endif
