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
typedef enum {NOKIA_SCREEN_TEXT,
              NOKIA_SCREEN_GRAPHIC,
              NOKIA_SCREEN_MENU,
              NOKIA_SCREEN_IMPORTANT_INFO
} nokia_screen_type;

extern nokia_screen_type nokia_screen; //currently displayed screen on the Nokia display
extern boolean display_force_text;

#endif
