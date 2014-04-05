/*
Switch handling functions: short and long press detection
(c) 2012-2013 jenkie and Thomas Jarosch / pedelecforum.de

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
#ifndef SWITCHES_H
#define SWITCHES_H

#include "Arduino.h"
#include "config.h"

enum switch_result { PRESSED_NONE=0, PRESSED_SHORT=1, PRESSED_LONG=2 };

void init_switches();
void handle_switch(const switch_name sw_name, boolean sw_state, const switch_result &force_press=PRESSED_NONE);

void action_increase_poti();
void action_decrease_poti();

#endif
