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

#include "BaseView.h"

/**
 * Base class for a display view
 */

const uint16_t LINE_GRAY = RGB_TO_565(150, 150, 150);

//! Constructor
BaseView::BaseView(Adafruit_ILI9341* tft)
        : m_tft(tft),
          m_active(false)
{
}
  
//! Destructor
BaseView::~BaseView() {
}

//! This view is now enabled and displayed
void BaseView::activate() {
  m_active = true;

  updateDisplay();
}

//! This view is now disabled and not displayed
void BaseView::deactivate() {
  m_active = false;  
}

 
