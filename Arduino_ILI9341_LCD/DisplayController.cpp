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

#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "RotaryEncoder.h"

#include "MainView.h"
#include "MenuView.h"
#include "MainViewEdit.h"

#include "protocol.h"

#include "Components.h"

/**
 * Control the whole display Navigation and output
 */


#define TFT_DC 10 //used
#define TFT_RST 9 //used
#define TFT_CS 8 //used
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_MISO 12

// Use Hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);


#define KEY A0

// If you change it, also change the interrupt initialisation!
#define KNOB0 A1
#define KNOB1 A2

// Setup a RoraryEncoder for pins A2 and A3:
RotaryEncoder encoder(KNOB0, KNOB1);

//! Main view with speed etc.
MainView* mainView;

//! Edit custmizeable part of the main view
MainViewEdit* mainViewEdit;

//! Menu view to show a menu
MenuView* menuView;

//! Current active view
BaseView* currentView;

//! Customizeable components on the main screen
Components components;

//! Key pressed flag
volatile bool g_keyPressed = false;

//! Call once on startup
void displayControllerSetup() {
  tft.begin();

  mainView = new MainView(&components);
  menuView = new MenuView();
  mainViewEdit = new MainViewEdit(&components);

  currentView = mainView;
  currentView->activate();

  // Button
  pinMode(KEY, INPUT);
  pinMode(KNOB0, INPUT);
  pinMode(KNOB1, INPUT);

  // enable pullup
  digitalWrite(KEY, HIGH);
  digitalWrite(KNOB0, HIGH);
  digitalWrite(KNOB1, HIGH);

  // You may have to modify the next 2 lines if using other pins than A1 and A2
  // This enables Pin Change Interrupt 1 that covers the Analog input pins or Port C.
  PCICR |= (1 << PCIE1);

  // This enables the interrupt for pin 1 and 2 of Port C.
  PCMSK1 |= (1 << PCINT9) | (1 << PCINT10);

  // Run timer2 interrupt every 15 ms
  TCCR2A = 0;
  TCCR2B = 1 << CS22 | 1 << CS21 | 1 << CS20;

  // Timer2 Overflow Interrupt Enable
  TIMSK2 |= 1 << TOIE2;
}

//! Timer 2 interrupt, for button debouncing
SIGNAL(TIMER2_OVF_vect) {
  static bool lastState = 1;
  bool currentState = digitalRead(KEY);

  if (currentState == lastState) {
    return;
  }

  if (currentState == 0) {
    g_keyPressed = true;
  }

  lastState = currentState;
}

// Pin Change Interrupt for Rotation encoder, A1 and A2
ISR(PCINT1_vect) {

  // Handle rotary interrupts
  encoder.tick();
}

//! Call in the main loop
void displayControllerLoop() {
  int diff = encoder.getPosition();

  if (diff != 0) {
    encoder.setPosition(0);
    currentView->movePosition(diff);
  }

  if (g_keyPressed) {
    g_keyPressed = false;
    ViewResult result = currentView->keyPressed();

    if (result.result == VIEW_RESULT_MENU) {
      currentView->deactivate();
      menuView->setRootMenuId(result.value);
      currentView = menuView;
      currentView->activate();
    } else if (result.result == VIEW_RESULT_BACK) {
      currentView->deactivate();
      currentView = mainView;
      currentView->activate();
    } else if (result.result == VIEW_RESULT_SELECTED) {
      currentView->deactivate();

      if (MENU_ID_VIEW_EDIT == result.value) {
        currentView = mainViewEdit;
      } else {
        currentView = mainView;
      }

      currentView->activate();
    } else if (result.result == VIEW_RESULT_CHECKBOX_CHECKED) {
      //! Checkbox toggled
    } else if (result.result == VIEW_RESULT_CHECKBOX_UNCHECKED) {
      //! Checkbox toggled
    }
  }

}

//! Execute 1 byte command
void displayControlerCommand1(uint8_t cmd, uint8_t value) {
  switch (cmd) {
    case DISP_CMD_STATES:
      mainView->setBluetooth(value & DISP_BIT_STATE_BLUETOOTH ? true : false);
      mainView->setBrakes(value & DISP_BIT_STATE_BRAKE ? true : false);
      mainView->setLight(value & DISP_BIT_STATE_LIGHT ? true : false);
      break;
  }
}

//! Execute 2 byte command
void displayControlerCommand2(uint8_t cmd, uint16_t value) {
  switch (cmd) {
    case DISP_CMD_BATTERY:
      mainView->setBatteryVoltage(value);
      break;
    case DISP_CMD_BATTERY_MAX:
      mainView->setBatteryMaxVoltage(value);
      break;
    case DISP_CMD_BATTERY_MIN:
      mainView->setBatteryMinVoltage(value);
      break;
    case DISP_CMD_SPEED:
      mainView->setSpeed(value);
      break;
    case DISP_CMD_WATTAGE:
      mainView->setWattage(value);
      break;
  }
}
