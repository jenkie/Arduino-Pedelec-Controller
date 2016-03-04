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
 * Display a text with value on the display
 */

#include "TextComponent.h"


//! Constructor
TextComponent::TextComponent() {

}

//! Destructor
TextComponent::~TextComponent() {

}

//! Y Position on display
uint8_t TextComponent::getHeight() {
   return 18;
}

//! Draw the component to the display
void TextComponent::draw() {
   tft.drawRect(0, m_y, 240, getHeight(), ILI9341_GREEN);

   tft.setTextColor(ILI9341_YELLOW);
   tft.setCursor(0, m_y + 2);
   tft.print("ASDF");
}
