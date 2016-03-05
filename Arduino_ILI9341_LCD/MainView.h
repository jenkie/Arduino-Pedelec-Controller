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

class MainView : public BaseView, public DataListener {
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

  //! This view is now enabled and displayed
  virtual void activate();

  //! This view is now disabled and not displayed
  virtual void deactivate();

  //! UP / DOWN Key
  virtual void movePosition(int8_t diff);

  //! Key (OK) pressed
  virtual ViewResult keyPressed();

  //! Battery percent, 0 ... n
  void setWattage(uint16_t wattage);

    // DataListener
public:
  //! Icon changed
  virtual void onIconUpdate(uint8_t iconId);

  //! a value was changed
  virtual void onValueChanged(uint8_t valueId);

private:
  //! Draw speed
  void drawSpeed(bool clearScreen);

  //! Draw battery
  void drawBattery(bool clearScreen);

  //! Draw wattage bar
  void drawWattage(bool clearScreen);

  // Member
protected:
  //! Current motor wattage
  uint16_t m_wattage;

  //! Customizeable components on the screen
  Components* m_components;
};
