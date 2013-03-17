/*
On the go menu system
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

#ifndef MENU_H
#define MENU_H

#include "Arduino.h"
#include "MenuSystem.h"

void init_menu();

extern unsigned long menu_activity_expire; // Timestamp when we kick the user ouf of the menu
extern boolean menu_active;             //Flag to indicate menu is shown or not
extern boolean menu_changed;            //Flag to indicate menu display needs update

extern MenuSystem menu_system;

#endif
