/*
Switch actions for use in config.h
(c) Thomas Jarosch / pedelecforum.de

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
#ifndef SWITCHES_ACTION_H
#define SWITCHES_ACTION_H

// Switch names
enum switch_name { SWITCH_THROTTLE=0, SWITCH_DISPLAY1=1, SWITCH_DISPLAY2=2, _SWITCHES_COUNT=3 };

// Switch actions: Customizable actions for short and long press. See config.h
enum sw_action { ACTION_NONE=0,                // do nothing
                 ACTION_SET_SOFT_POTI,         // set soft poti
                 ACTION_SHUTDOWN_SYSTEM,       // power down system
                 ACTION_ENABLE_BACKLIGHT_LONG, // enable backlight for 60s
                 ACTION_TOGGLE_BLUETOOTH,      // switch bluetooth on/off
                 ACTION_ENTER_MENU,            // enter on-the-go settings menu
                 ACTION_PROFILE_1,             // switch to profile 1
                 ACTION_PROFILE_2,             // switch to profile 2
                 ACTION_PROFILE,               // switch profiles
               };

#endif
