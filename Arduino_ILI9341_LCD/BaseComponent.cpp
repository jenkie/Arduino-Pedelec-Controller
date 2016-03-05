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
 * Base class for customizeable components
 */

#include "BaseComponent.h"

//! Constructor
BaseComponent::BaseComponent()
             : m_y(0)
{
}

//! Destructor
BaseComponent::~BaseComponent() {
}

//! Y Position on display
void BaseComponent::setY(uint16_t y) {
  m_y = y;
}

//! Y Position on display
uint16_t BaseComponent::getY() {
  return m_y;
}

//! If this component is active drawed on update
void BaseComponent::setActive(bool active) {
  m_active = active;
}
