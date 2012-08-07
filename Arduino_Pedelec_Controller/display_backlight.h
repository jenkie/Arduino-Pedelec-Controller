/*
Generic LCD display backlight functions
(c) 2012 Thomas Jarosch

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
#ifndef DISPLAY_BACKLIGHT_H
#define DISPLAY_BACKLIGHT_H

#include "config.h"

#ifdef SUPPORT_DISPLAY_BACKLIGHT
void enable_backlight();
void enable_custom_backlight(unsigned int duration_ms);
void blink_backlight();
void blink_backlight_stay_on();
void handle_backlight();
#endif

#endif
