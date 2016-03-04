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

#include "BaseView.h"

/**
 * Main view
 */

class Components;

class MainView : public BaseView {
  // Constructor / Destructor
public:
  //! Constructor
  MainView(Components* components);

  //! Destructor
  virtual ~MainView();

  // public API
public:
  //! Update full display
  virtual void updateDisplay();

  //! UP / DOWN Key
  virtual void movePosition(int8_t diff);

  //! Key (OK) pressed
  virtual ViewResult keyPressed();

  //! Set the speed in 0.1 km/h and update display if needed
  void setSpeed(uint16_t kmh);

  //! Voltage when fully charged
  void setBatteryMaxVoltage(uint16_t voltage);

  //! Voltage when dischaerged
  void setBatteryMinVoltage(uint16_t voltage);

  //! Current voltage
  void setBatteryVoltage(uint16_t voltage);

  //! Battery percent, 0 ... n
  void setWattage(uint16_t wattage);

  //! Bluetooth enabled
  void setBluetooth(bool bluetooth);

  //! Brakes enabled
  void setBrakes(bool brakes);

  //! Light on
  void setLight(bool light);

private:
  //! Draw speed
  void drawSpeed(bool clearScreen);

  //! Draw battery
  void drawBattery(bool clearScreen);

  //! Draw wattage bar
  void drawWattage(bool clearScreen);

  //! Draw Bluetooth Icon
  void drawBluetooth(bool clearScreen);

  //! Draw Brakes Icon
  void drawBrakes(bool clearScreen);

  //! Draw Light Icon
  void drawLight(bool clearScreen);

  // Member
private:
  //! the speed in 0.1 km/h and update display if needed
  uint16_t m_speed;

  //! Voltage when fully charged
  uint16_t m_batteryMaxVoltage;

  //! Voltage when dischaerged
  uint16_t m_batteryMinVoltage;

  //! Current voltage
  uint16_t m_batteryVoltage;

  //! Current motor wattage
  uint16_t m_wattage;

  //! Bluetooth enabled
  bool m_bluetooth;

  //! Brakes enabled
  bool m_brakes;

  //! Light on
  bool m_light;

  //! Customizeable components on the screen
  Components* m_components;
};
