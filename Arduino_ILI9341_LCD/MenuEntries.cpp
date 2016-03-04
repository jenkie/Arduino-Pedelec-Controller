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

#include "MenuEntries.h"

#include <avr/pgmspace.h>

/**
 * Definition for Menu Entry texts
 */

/*
Layout:
    ├── [Ausschalten]              (only shown when no button is set to ACTION_SHUTDOWN_SYSTEM)
    ├── Anzeige
    │   ├── Reset Wh
    │   ├── Reset KM
    │   ├── Graf. an/              (only for Nokia displays)
    |   ├── Beleuchtung an         (only with SUPPORT_DISPLAY_BACKLIGHT)
    |   ├── [Beleucht. aus]        (only without SUPPORT_DISPLAY_BACKLIGHT and DISPLAY_TYPE_16X2_SERIAL)
    │   └── Zurück
    ├── Licht an/aus
    ├── BT an/aus
    ├── Profil 1<>2
    ├── Sonstiges
    │   ├── Nothilfe
    │   │    ├── Ign. Bremse
    │   │    ├── Ign. Treten
    │   │    ├── Ign. Tacho
    │   │    ├── Ign. Gasgr.
    │   │    ├── Ign. Poti             (only with SUPPORT_POTI)
    │   │    ├── Poti +
    │   │    ├── Poti -
    │   │    └── Zurück
    │   └── Zurück
    └── Zurück
*/

#define UE "\x81"

const char TXT_MENU_ROOT[] PROGMEM = "Men" UE;
const char TXT_MENU_TURN_OFF[] PROGMEM = "Ausschalten";
const char TXT_MENU_VIEW[] PROGMEM = "Anzeige";
const char TXT_MENU_RESET_WH[] PROGMEM = "Reset Wh";
const char TXT_MENU_RESET_KM[] PROGMEM = "Reset KM";
const char TXT_MENU__BACK[] PROGMEM = "Zur" UE "ck";
const char TXT_MENU__LIGHT[] PROGMEM = "Licht an/aus";
const char TXT_MENU__BLUETOOTH[] PROGMEM = "BT an/aus";
const char TXT_MENU_PROFIL[] PROGMEM = "Profil 1<>2";
const char TXT_MENU_EMERGENCY[] PROGMEM = "Nothilfe";
const char TXT_MENU_EM_BRAKE[] PROGMEM = "Ign. Bremse";
const char TXT_MENU_EM_PEDAL[] PROGMEM = "Ign. Treten";
const char TXT_MENU_EM_SPEED[] PROGMEM = "Ign. Tacho";
const char TXT_MENU_EM_SPEEDCTRL[] PROGMEM = "Ign. Gasgr.";
const char TXT_MENU_EM_POTI[] PROGMEM = "Ign. Poti";
const char TXT_MENU_ADD_POTI[] PROGMEM = "Poti +";
const char TXT_MENU_DEC_POTI[] PROGMEM = "Poti -";

/*
typedef struct {
  uint8_t id;
  uint8_t parentId;
  const char* text;
} MenuItem;

#define MENU_DEFAULT 0
#define MENU_CHECKBOX 1
#define MENU_BACK 2

*/
const MenuItem PROGMEM Menu[] = {
  // Root menu
  {.id =  1, .parentId =  0, .text = TXT_MENU_ROOT, .flags = MENU_WITH_SUBMENU},
  {.id =  2, .parentId =  1, .text = TXT_MENU_TURN_OFF, .flags = MENU_DEFAULT},
  {.id =  3, .parentId =  1, .text = TXT_MENU_VIEW, .flags = MENU_WITH_SUBMENU},
  {.id =  4, .parentId =  3, .text = TXT_MENU_RESET_WH, .flags = MENU_DEFAULT},
  {.id =  5, .parentId =  3, .text = TXT_MENU_RESET_KM, .flags = MENU_DEFAULT},
  {.id =  6, .parentId =  3, .text = TXT_MENU__BACK, .flags = MENU_BACK},
  {.id =  7, .parentId =  1, .text = TXT_MENU__LIGHT, .flags = MENU_CHECKBOX},
  {.id =  8, .parentId =  1, .text = TXT_MENU__BLUETOOTH, .flags = MENU_CHECKBOX},
  {.id =  9, .parentId =  1, .text = TXT_MENU_PROFIL, .flags = MENU_CHECKBOX},
  {.id = 10, .parentId =  1, .text = TXT_MENU_EMERGENCY, .flags = MENU_WITH_SUBMENU},
  {.id = 11, .parentId = 10, .text = TXT_MENU_EM_BRAKE, .flags = MENU_CHECKBOX},
  {.id = 12, .parentId = 10, .text = TXT_MENU_EM_PEDAL, .flags = MENU_CHECKBOX},
  {.id = 13, .parentId = 10, .text = TXT_MENU_EM_SPEED, .flags = MENU_CHECKBOX},
  {.id = 14, .parentId = 10, .text = TXT_MENU_EM_SPEEDCTRL, .flags = MENU_CHECKBOX},
  {.id = 15, .parentId = 10, .text = TXT_MENU_ADD_POTI, .flags = MENU_CHECKBOX},
  {.id = 16, .parentId = 10, .text = TXT_MENU_DEC_POTI, .flags = MENU_CHECKBOX},
  {.id = 17, .parentId = 10, .text = TXT_MENU__BACK, .flags = MENU_BACK},
  {.id = 18, .parentId =  1, .text = TXT_MENU__BACK, .flags = MENU_BACK},

  // Menu 
};

const uint8_t Menu_Count = 18;
//const uint8_t Menu_Count = sizeof(Menu) / sizeof(MenuItem);



