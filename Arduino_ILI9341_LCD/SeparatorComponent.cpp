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

/**
 * Display a separator line
 */

 #include "SeparatorComponent.h"
 #include "defines.h"

 const uint16_t SEPARATOR_COLOR = RGB_TO_565(120, 120, 120);

//! Constructor
SeparatorComponent::SeparatorComponent() {
}

//! Destructor
SeparatorComponent::~SeparatorComponent() {
}

//! Y Position on display
uint8_t SeparatorComponent::getHeight() {
   return 4;
}

//! Draw the component to the display
void SeparatorComponent::draw() {
  for (uint8_t i = 1; i <= 2; i++) {
    tft.drawLine(0, m_y + i, 240, m_y + i, SEPARATOR_COLOR);
  }
}
