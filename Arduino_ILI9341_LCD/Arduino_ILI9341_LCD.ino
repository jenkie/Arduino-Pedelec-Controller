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

#include "DisplayController.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Display init");

  displayControllerSetup();
}

void loop() {
  displayControllerLoop();

  
/*
    if (g_state == 0) {
      menuView->deactivate();
      mainView->activate();
    } else if (g_state == 1) {
      mainView->deactivate();
      menuView->activate();
    }
*/


/*
  if (g_state == 0) {
    mainView->setBluetooth(true);
    mainView->setBrakes(true);
    mainView->setLight(true);
  
    mainView->setSpeed(125);
    mainView->setBatteryPercent(100);
    mainView->setWattage(500);
    delay(2000);
  
    mainView->setBatteryPercent(39);
    mainView->setWattage(900);
    delay(2000);
    
    mainView->setBluetooth(false);
    mainView->setBrakes(false);
    mainView->setLight(false);
  
    mainView->setBatteryPercent(20);
    mainView->setWattage(100);
    delay(2000);
  
    mainView->setSpeed(253);
    mainView->setBatteryPercent(5);
    mainView->setWattage(5);
    delay(2000);
  } else if (g_state == 1) {
  }
  */
}




