/*
Code by JeeLabs http://news.jeelabs.org/code/
Released to the public domain! Enjoy!
modified by Jens Kießling / jenkie

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

#include <Wire.h>
#include "ds1307.h"
#include <avr/pgmspace.h>
#define WIRE Wire
#define DS1307_ADDRESS 0x68

#if (ARDUINO >= 100)
#include <Arduino.h> // capital A so it is error prone on case-sensitive filesystems
#else
#include <WProgram.h>
#endif

static uint8_t bcd2bin (uint8_t val) { return val - 6 * (val >> 4); }
static uint8_t bin2bcd (uint8_t val) { return val + 6 * (val / 10); }

void RTC_DS1307::adjust_time(uint8_t hh, uint8_t mm, uint8_t ss)
{
    WIRE.beginTransmission(DS1307_ADDRESS);
    WIRE.write(0);
    WIRE.write(bin2bcd(ss));
    WIRE.write(bin2bcd(mm));
    WIRE.write(bin2bcd(hh));
    WIRE.endTransmission();
}

Time RTC_DS1307::get_time()
{
    WIRE.beginTransmission(DS1307_ADDRESS);
    WIRE.write(0);
    WIRE.endTransmission();
    WIRE.requestFrom(DS1307_ADDRESS, 7);
    uint8_t ss = bcd2bin(WIRE.read() & 0x7F);
    uint8_t mm = bcd2bin(WIRE.read());
    uint8_t hh = bcd2bin(WIRE.read());
    return (Time) {hh,mm,ss};
}

