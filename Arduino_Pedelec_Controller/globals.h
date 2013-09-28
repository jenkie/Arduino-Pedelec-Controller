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
extern char inchar;

extern const int bluetooth_pin;
extern const int fet_out;


extern const int *ptr_startingaid_speed;
extern const int *ptr_spd_max1;                 
extern const int *ptr_spd_max2;               
extern const int *ptr_power_max;                 
extern const int *ptr_power_poti_max;            
extern const double *ptr_capacity;   
extern boolean current_profile;
extern void save_eeprom();

#endif
