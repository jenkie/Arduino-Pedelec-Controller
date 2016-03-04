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
 * List with the customized components
 */

#include "Components.h"
#include "TextComponent.h"


//! Constructor
Components::Components()
          : m_components({0})
{
  uint16_t y = 150;
  for (uint8_t i = 0; i < COMPONENT_COUNT; i++) {
     m_components[i] = new TextComponent();
     m_components[i]->setY(y);
     y += m_components[i]->getHeight();
  }
}

//! Destructor
Components::~Components() {

}

//! Draw all components
void Components::draw() {
  for (uint8_t i = 0; i < COMPONENT_COUNT; i++) {
    m_components[i]->draw();
  }
}
