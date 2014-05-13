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
void display_show_important_info(const __FlashStringHelper *str, int duration_secs);

void display_show_welcome_msg();
void display_show_welcome_msg_temp();

//definitions for different screen mode
typedef enum {DISPLAY_MODE_TEXT,
              DISPLAY_MODE_GRAPHIC,               // Note: Same as _TEXT on 16x2 displays
              DISPLAY_MODE_MENU,
              DISPLAY_MODE_IMPORTANT_INFO
} display_mode_type;

extern display_mode_type display_mode; //currently display mode
extern boolean display_force_text;         //only valid for Nokia displays

#endif
