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
 * Base class for customizeable components
 */

// Don't use a member, use this extern declared reference
// Not a nice coding practice, but saves about 300 Bytes of Flash
extern Adafruit_ILI9341 tft;
extern DataModel model;

class BaseComponent {
  // Constructor / Destructor
public:
  //! Constructor
  BaseComponent();

  //! Destructor
  virtual ~BaseComponent();

  // public API
public:
  //! Return the height in pixel
  virtual uint8_t getHeight() = 0;

  //! Draw the component to the display
  virtual void draw() = 0;

  //! Y Position on display
  void setY(uint16_t y);

  //! Y Position on display
  uint16_t getY();

  //! If this component is active drawed on update
  void setActive(bool active);

  // Member
protected:
  //! Y Position on display
  uint16_t m_y;

  //! Flag if active / not active
  bool m_active;
};
