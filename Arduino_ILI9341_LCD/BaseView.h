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
#include "Print.h"

/**
 * Base class for a display view
 */

class Adafruit_ILI9341;


#define RGB_TO_565(r, g, b) ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)
extern const uint16_t LINE_GRAY;

//! The view is still alive, nothing to do
#define VIEW_RESULT_NOTHING 0;

//! Menu exited with back
#define VIEW_RESULT_BACK 1;

//! Show a menu, the menu root ID is in ViewResult.value
#define VIEW_RESULT_MENU 2;

//! An entry was selected, selected ID is return in ViewResult.value
#define VIEW_RESULT_SELECTED 3;

//! Checkbox toggled
#define VIEW_RESULT_CHECKBOX_CHECKED 4;

//! Checkbox toggled
#define VIEW_RESULT_CHECKBOX_UNCHECKED 5;

typedef struct {
  uint8_t result;
  uint8_t value;
} ViewResult;

class BaseView {
  // Constructor / Destructor
public:
  //! Constructor
  BaseView(Adafruit_ILI9341* tft);
  
  //! Destructor
  virtual ~BaseView();

  // public API
public:
  //! This view is now enabled and displayed
  void activate();

  //! This view is now disabled and not displayed
  void deactivate();

  //! Update full display
  virtual void updateDisplay() = 0;

  //! UP / DOWN Key
  virtual void movePosition(int8_t diff) = 0;

  //! Key (OK) pressed
  virtual ViewResult keyPressed() = 0;

  // Member
protected:
  //! Display to write
  Adafruit_ILI9341* m_tft;

  //! Flag if active / not active
  bool m_active;
};

