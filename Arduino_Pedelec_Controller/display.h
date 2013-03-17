/*
Generic display init and update functions
Written by Thomas Jarosch

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
#ifndef DISPLAY_H
#define DISPLAY_H

#include "Arduino.h"



void display_init();
void display_update();
void display_show_important_info(const char *str, int duration_secs);

//definitions for Nokia graphic display
typedef enum {NOKIA_SCREEN_TEXT, NOKIA_SCREEN_GRAPHIC, NOKIA_SCREEN_MENU} nokia_screen_type;
extern boolean display_force_text;

// Global defines from main program.
// This list will be replaced by a proper defined interface between
// the main code and the display code sooner or later.
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
extern float temperature;
extern float altitude;
extern float slope;
extern const int bluetooth_pin;
extern float poti_stat;
extern int throttle_stat;
extern boolean brake_stat;
extern volatile unsigned long wheel_time;
extern const int fet_out;
extern byte pulse_human;
#ifdef SUPPORT_XCELL_RT
extern double power_human;
extern nokia_screen_type nokia_screen; //currently displayed screen on the Nokia display
#endif
#endif
