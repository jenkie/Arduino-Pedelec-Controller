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

#include <SPI.h>
#include <EEPROM.h>

#include <avr/pgmspace.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "RotaryEncoder.h"

#include "MainView.h"
#include "MenuView.h"

#define TFT_DC 10 //used
#define TFT_RST 9 //used
#define TFT_CS 8 //used
#define TFT_MOSI 11
#define TFT_CLK 13
#define TFT_MISO 12

#define KEY A0

// If you change it, also change the interrupt initialisation!
#define KNOB0 A1
#define KNOB1 A2

// Setup a RoraryEncoder for pins A2 and A3:
RotaryEncoder encoder(KNOB0, KNOB1);

// Use Hardware SPI
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);


MainView* mainView;
MenuView* menuView;
BaseView currentView;

//! Key pressed flag
volatile bool g_keyPressed = false;
volatile bool g_ignoreNextKeyPress = false;

void setup() 
{
  Serial.begin(115200);
  Serial.println("Display init");

  tft.begin();

  mainView = new MainView(&tft);
  menuView = new MenuView(&tft);

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

  // This enables the interrupt for pin 0 of Port C.
  // A0 for KEY
  PCMSK1 |= (1 << PCINT8);
}

// Pin Change Interrupt for Button and A1 and A2
ISR(PCINT1_vect) {

  // Handle rotary interrupts
  encoder.tick();

  static unsigned long lastChangeTime = 0;
  if (digitalRead(KEY) == 0) {
    if((millis() - lastChangeTime) >= 30) {
      lastChangeTime = millis();

      if (g_ignoreNextKeyPress) {
        g_ignoreNextKeyPress = false;
      } else {
        g_keyPressed = true;
      }
    }
  }
}

void clearInput()
// clear all characters from the serial input
{
  while (Serial.read() != -1)
    ;
}

void loop() 
{
  int diff = encoder.getPosition();
  if (diff != 0) {
    encoder.setPosition(0);
    currentView->movePosition(diff);
  }

  if (g_keyPressed) {
    g_keyPressed = false;

    if (digitalRead(KEY) == 0) {
      g_ignoreNextKeyPress = true;
    }
    
    menuView->keyPressed();
  }
  
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




