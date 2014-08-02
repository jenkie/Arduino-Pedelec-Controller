/*
Global variables from the main program code end up here.
(c) 2013 Thomas Jarosch / pedelecforum.de

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

#ifndef GLOBALS_H
#define GLOBALS_H

#include "ds1307.h"

// Global defines from the main program code
extern float mah;
extern boolean display_force_text;
extern float voltage_display;
extern float current_display;
extern byte battery_percent_fromvoltage;
extern byte battery_percent_fromcapacity;
extern double power;
extern double power_set;
extern float wh;
extern volatile float spd;
extern volatile int cad;
extern volatile float km;
extern float range;
extern unsigned long odo;
extern float temperature;
extern float altitude;
extern float altitude_start;
extern float slope;
extern int poti_stat;
extern int throttle_stat;
extern boolean brake_stat;
extern volatile unsigned long wheel_time;
extern byte pulse_human;
extern double power_human;
extern boolean variables_saved;

extern const int bluetooth_pin;
extern const int fet_out;
extern const int lights_pin;

extern int curr_startingaid_speed;
extern int curr_spd_max1;
extern int curr_spd_max2;
extern int curr_power_max;
extern int curr_power_poti_max;
extern double curr_capacity;
extern boolean current_profile;

extern boolean first_aid_ignore_break;
extern boolean first_aid_ignore_pas;
extern boolean first_aid_ignore_speed;
extern boolean first_aid_ignore_poti;
extern boolean first_aid_ignore_throttle;

extern void save_eeprom();
extern void save_shutdown();
extern void activate_new_profile();

extern RTC_DS1307 rtc;
extern Time now;

// Define own version of F() macro since compile will
// fail on newer gcc versions with a "const" warning.
// This is the version as in Arduino 1.5 and newer.
#define MY_F(string_literal) (reinterpret_cast<const __FlashStringHelper *>(PSTR(string_literal)))

#define FROM_FLASH(str) (reinterpret_cast<const __FlashStringHelper *>(str))

#if HARDWARE_REV <= 5
#define FET_ON LOW
#define FET_OFF HIGH
#else
#define FET_ON HIGH
#define FET_OFF LOW
#endif

#endif
