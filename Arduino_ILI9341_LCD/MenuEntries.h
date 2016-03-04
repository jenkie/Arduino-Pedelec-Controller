/*
ILI9341 LCD Display controller for Arduino_Pedelec_Controller

Copyright (C) 2016
Andreas Butti, andreas.b242 at gmail dot com

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

#pragma once

#include "Arduino.h"

/**
 * Definition for Menu Entry texts
 */

typedef struct _MenuItem {
  uint8_t id;
  uint8_t parentId;
  const char* text;
  uint8_t flags;
} MenuItem;

#define MENU_DEFAULT 0
#define MENU_CHECKBOX 1
#define MENU_BACK 2
#define MENU_WITH_SUBMENU 4

enum {
   MENU_ID_NONE = 0,

   // -------------

   MENU_ID_ROOT,
   MENU_ID_TURN_OFF,
   MENU_ID_VIEW,
   MENU_ID_RESET_WH,
   MENU_ID_RESET_KM,
   MENU_ID_VIEW_EDIT,
   MENU_ID_BACK_VIEW,
   MENU_ID_LIGHT_CB,
   MENU_ID_BLUETOOTH_CB,
   MENU_ID_PROFIL_CB,
   MENU_ID_EMERGENCY,
   MENU_ID_EM_BRAKE_CB,
   MENU_ID_EM_PEDAL_CB,
   MENU_ID_EM_SPEED_CB,
   MENU_ID_EM_SPEEDCTRL_CB,
   MENU_ID_ADD_POTI_CB,
   MENU_ID_DEC_POTI_CB,
   MENU_ID_BACK_EMERGENCY,
   MENU_ID_BACK_MAIN,

   // -------------

   MENU_ID_COMPONENT = 40,
   MENU_ID_COMPONENT_REPLACE,
   MENU_ID_COMPONENT_REMOVE,
   MENU_ID_COMPONENT_BACK,
};

extern const MenuItem PROGMEM Menu[];

extern uint8_t Menu_Count;
