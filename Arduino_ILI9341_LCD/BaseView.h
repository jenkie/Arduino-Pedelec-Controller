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
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#include "DataModel.h"

/**
 * Base class for a display view
 */

// Don't use a member, use this extern declared reference
// Not a nice coding practice, but saves about 300 Bytes of Flash (only for TFT!)
extern Adafruit_ILI9341 tft;
extern DataModel model;

extern const uint16_t LINE_GRAY;

//! The view is still alive, nothing to do
#define VIEW_RESULT_NOTHING 0

//! Menu exited with back
#define VIEW_RESULT_BACK 1

//! Show a menu, the menu root ID is in ViewResult.value
#define VIEW_RESULT_MENU 2

//! An entry was selected, selected ID is return in ViewResult.value
#define VIEW_RESULT_SELECTED 3

//! Checkbox toggled
#define VIEW_RESULT_CHECKBOX_CHECKED 4

//! Checkbox toggled
#define VIEW_RESULT_CHECKBOX_UNCHECKED 5

typedef struct {
  uint8_t result;
  uint8_t value;
} ViewResult;

class BaseView {
  // Constructor / Destructor
public:
  //! Constructor
  BaseView();

  //! Destructor
  virtual ~BaseView();

  // public API
public:
  //! This view is now enabled and displayed
  virtual void activate();

  //! This view is now disabled and not displayed
  virtual void deactivate();

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
