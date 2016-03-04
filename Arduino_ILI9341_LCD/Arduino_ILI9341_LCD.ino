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
#include "protocol.h"

void setup() {
  Serial.begin(115200);
  Serial.println("Display init");

  displayControllerSetup();

  displayControlerCommand2(DISP_CMD_BATTERY_MAX, 430);
  displayControlerCommand2(DISP_CMD_BATTERY_MIN, 330);
}

void loop() {
  displayControllerLoop();


  // Simulation
  uint16_t s = millis() / 1000;
  static uint16_t lastSecond = 0;

  if (lastSecond == s) {
    return;    
  }

  lastSecond = s;

  uint8_t v5 = s % 5;

  if (v5 == 0) {
    displayControlerCommand1(DISP_CMD_STATES, 0);
  } else if (v5 == 1) {
    displayControlerCommand1(DISP_CMD_STATES, DISP_BIT_STATE_BLUETOOTH);
  } else if (v5 == 2) {
    displayControlerCommand1(DISP_CMD_STATES, DISP_BIT_STATE_BRAKE);
  } else if (v5 == 3) {
    displayControlerCommand1(DISP_CMD_STATES, DISP_BIT_STATE_LIGHT);
  } else if (v5 == 4) {
    displayControlerCommand1(DISP_CMD_STATES, DISP_BIT_STATE_BLUETOOTH | DISP_BIT_STATE_BRAKE | DISP_BIT_STATE_LIGHT);
  }
  
  displayControlerCommand2(DISP_CMD_BATTERY, s % 100 + 330);
  Serial.print("v1 ");
  Serial.println(s % 100 + 330);

  displayControlerCommand2(DISP_CMD_SPEED, s % 200 + 5);

  displayControlerCommand2(DISP_CMD_WATTAGE, s % 500 + 3);
}




