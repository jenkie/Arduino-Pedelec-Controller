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
#include "defines.h"

#include <avr/pgmspace.h>

/**
 * Definition for Menu Entry texts
 */

/*
Root Menu
 ├── Anzeige
 │   ├── Anpassen
 │   ├── Reset Wh
 │   ├── Reset KM
 │   └── Zurück
 ├── Licht an/aus
 ├── BT an/aus
 ├── Profil 1<>2
 ├── Nothilfe
 │    ├── Ign. Bremse
 │    ├── Ign. Treten
 │    ├── Ign. Tacho
 │    ├── Ign. Gasgr.
 │    ├── Ign. Poti             (only with SUPPORT_POTI)
 │    ├── Poti +
 │    ├── Poti -
 │    └── Zurück
 ├── Ausschalten
 └── Zurück

Komponent
 ├── Ersetzen
 ├── Entfernen
 └── Zurück

 //print battery percent left
 lcd.drawVerticalBar((word)(100), (word)(constrain((voltage_display-vcutoff)/(vmax-vcutoff)*100,0,100)), (word)(battery_percent_fromcapacity), 11, 4);


Komponent wählen
 ├── Sensoren
 │    ├── Temperatur 1      // sensors.getTempCByIndex(0) / temperature
 │    └── Temperatur 2      // sensors.getTempCByIndex(1)
 ├── Höhe
 │    ├── Start             // altitude_start
 │    ├── Altitude          // altitude
 │    └── Steigung          // slope
 ├── Leistung
 │    ├── Leistung Motor    // wh
 │    ├── Stromverbrauch    // current_display
 │    ├── Akku Spannung     // voltage_display
 │    ├── Aktuelle Leistung // power
 │    ├── Leistung Mensch   // wh_human
 │    ├── Trittfrequenz     // CAD: cad
 │    └── Reichweite        // range
 ├── Unterstützung          // lcd.drawVerticalBar((word)(max(curr_power_max,curr_power_poti_max)), (word)(max(power_set,0)), (word)(power), 11, 4);
 ├── Uhr                    // now.hh / now.mm
 ├── Fahrzeit               // printTime(millis() / 1000UL);  //millis can be used, because they roll over only after 50 days
 ├── Tages km               // km
 ├── Aktives Profil         // current_profile
 ├── Puls
 │    ├── Puls              // pulse_human
 │    ├── Puls min          // pulse_min
 │    └── Puls Bereich      // pulse_range
 └── Zurück

// TODO Pulse icon

*/

const char TXT_MENU_ROOT[] PROGMEM = "Men" UE;
const char TXT_MENU_VIEW[] PROGMEM = "Anzeige";
const char TXT_MENU_VIEW_EDIT[] PROGMEM = "Customize View";
const char TXT_MENU_RESET_WH[] PROGMEM = "Reset Wh";
const char TXT_MENU_RESET_KM[] PROGMEM = "Reset KM";
const char TXT_MENU_BACK[] PROGMEM = "Zur" UE "ck";
const char TXT_MENU_LIGHT[] PROGMEM = "Licht an/aus";
const char TXT_MENU_BLUETOOTH[] PROGMEM = "Bluetooth an/aus";
const char TXT_MENU_PROFIL[] PROGMEM = "Profil 1/2";
const char TXT_MENU_EMERGENCY[] PROGMEM = "Nothilfe";
const char TXT_MENU_EM_BRAKE[] PROGMEM = "Ign. Bremse";
const char TXT_MENU_EM_PEDAL[] PROGMEM = "Ign. Treten";
const char TXT_MENU_EM_SPEED[] PROGMEM = "Ign. Tacho";
const char TXT_MENU_EM_SPEEDCTRL[] PROGMEM = "Ign. Gasgr.";
const char TXT_MENU_EM_POTI[] PROGMEM = "Ign. Poti";
const char TXT_MENU_ADD_POTI[] PROGMEM = "Poti +";
const char TXT_MENU_DEC_POTI[] PROGMEM = "Poti -";
const char TXT_MENU_TURN_OFF[] PROGMEM = "Ausschalten";


const char TXT_MENU_COMPONENT[] PROGMEM = "Komponent";
const char TXT_MENU_COMPONENT_REPLACE[] PROGMEM = "Ersetzen";
const char TXT_MENU_COMPONENT_REMOVE[] PROGMEM = "Entfernen";

const char TXT_MENU_COMPONENT_SELECTION[] PROGMEM = "Komponent w" ae "hlen";


const MenuItem PROGMEM Menu[] = {
  // Root menu
  {.id = MENU_ID_ROOT,            .parentId = MENU_ID_NONE, .text = TXT_MENU_ROOT, .flags = MENU_WITH_SUBMENU},
  {.id = MENU_ID_VIEW,            .parentId = MENU_ID_ROOT, .text = TXT_MENU_VIEW, .flags = MENU_WITH_SUBMENU},
  {.id = MENU_ID_VIEW_EDIT,       .parentId = MENU_ID_VIEW, .text = TXT_MENU_VIEW_EDIT, .flags = MENU_DEFAULT},
  {.id = MENU_ID_RESET_WH,        .parentId = MENU_ID_VIEW, .text = TXT_MENU_RESET_WH, .flags = MENU_DEFAULT},
  {.id = MENU_ID_RESET_KM,        .parentId = MENU_ID_VIEW, .text = TXT_MENU_RESET_KM, .flags = MENU_DEFAULT},
  {.id = MENU_ID_BACK_VIEW,       .parentId = MENU_ID_VIEW, .text = TXT_MENU_BACK, .flags = MENU_BACK},
  {.id = MENU_ID_LIGHT_CB,        .parentId = MENU_ID_ROOT, .text = TXT_MENU_LIGHT, .flags = MENU_CHECKBOX},
  {.id = MENU_ID_BLUETOOTH_CB,    .parentId = MENU_ID_ROOT, .text = TXT_MENU_BLUETOOTH, .flags = MENU_CHECKBOX},
  {.id = MENU_ID_PROFIL_CB,       .parentId = MENU_ID_ROOT, .text = TXT_MENU_PROFIL, .flags = MENU_CHECKBOX},
  {.id = MENU_ID_EMERGENCY,       .parentId = MENU_ID_ROOT, .text = TXT_MENU_EMERGENCY, .flags = MENU_WITH_SUBMENU},
  {.id = MENU_ID_EM_BRAKE_CB,     .parentId = MENU_ID_EMERGENCY, .text = TXT_MENU_EM_BRAKE, .flags = MENU_CHECKBOX},
  {.id = MENU_ID_EM_PEDAL_CB,     .parentId = MENU_ID_EMERGENCY, .text = TXT_MENU_EM_PEDAL, .flags = MENU_CHECKBOX},
  {.id = MENU_ID_EM_SPEED_CB,     .parentId = MENU_ID_EMERGENCY, .text = TXT_MENU_EM_SPEED, .flags = MENU_CHECKBOX},
  {.id = MENU_ID_EM_SPEEDCTRL_CB, .parentId = MENU_ID_EMERGENCY, .text = TXT_MENU_EM_SPEEDCTRL, .flags = MENU_CHECKBOX},
  {.id = MENU_ID_ADD_POTI_CB,     .parentId = MENU_ID_EMERGENCY, .text = TXT_MENU_ADD_POTI, .flags = MENU_CHECKBOX},
  {.id = MENU_ID_DEC_POTI_CB,     .parentId = MENU_ID_EMERGENCY, .text = TXT_MENU_DEC_POTI, .flags = MENU_CHECKBOX},
  {.id = MENU_ID_BACK_EMERGENCY,  .parentId = MENU_ID_EMERGENCY, .text = TXT_MENU_BACK, .flags = MENU_BACK},
  {.id = MENU_ID_TURN_OFF,        .parentId = MENU_ID_ROOT, .text = TXT_MENU_TURN_OFF, .flags = MENU_DEFAULT},
  {.id = MENU_ID_BACK_MAIN,       .parentId = MENU_ID_ROOT, .text = TXT_MENU_BACK, .flags = MENU_BACK},

  // Component menu
  {.id = MENU_ID_COMPONENT,         .parentId = MENU_ID_NONE, .text = TXT_MENU_COMPONENT, .flags = MENU_WITH_SUBMENU},
  {.id = MENU_ID_COMPONENT_REPLACE, .parentId = MENU_ID_COMPONENT, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = MENU_ID_COMPONENT_REMOVE,  .parentId = MENU_ID_COMPONENT, .text = TXT_MENU_COMPONENT_REMOVE, .flags = MENU_DEFAULT},
  {.id = MENU_ID_COMPONENT_BACK,    .parentId = MENU_ID_COMPONENT, .text = TXT_MENU_BACK, .flags = MENU_BACK},

  // Component selection menu
  /*
  {.id = MENU_ID_COMPONENT_SELECTION, .parentId = MENU_ID_NONE, .text = TXT_MENU_COMPONENT_SELECTION, .flags = MENU_WITH_SUBMENU},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},
  {.id = XXXXXXXXXXXXXXXXXXXXXXXXXxx, .parentId = MENU_ID_COMPONENT_SELECTION, .text = TXT_MENU_COMPONENT_REPLACE, .flags = MENU_DEFAULT},

*/
};

uint8_t Menu_Count = sizeof(Menu) / sizeof(MenuItem);
