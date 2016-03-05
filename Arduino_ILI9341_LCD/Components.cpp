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
#include "DiagramComponent.h"
#include "IconComponent.h"
#include "SeparatorComponent.h"

//! Constructor
Components::Components()
          : m_components({0})
{
  for (uint8_t i = 0; i < COMPONENT_COUNT; i++) {
    if (i == 1) {
      m_components[i] = new IconComponent();
    } else if (i == 0 || i == 2) {
      m_components[i] = new SeparatorComponent();
    } else if (i == 5) {
      m_components[i] = new DiagramComponent();
    } else {
      String txt = "Eintrag ";
      txt += i;
      m_components[i] = new TextComponent(txt);
    }

  }

  updatePositionAndRemoveInvisible();
}

//! Destructor
Components::~Components() {
}

//! Activate / Deactivate children
void Components::deActivateChilren(bool enabled) {
  for (uint8_t i = 0; i < COMPONENT_COUNT; i++) {
    if (m_components[i] != NULL) {
      m_components[i]->setActive(enabled);
    }
  }
}

//! Update the Y position of all elements, and remove invisible elements from the list
void Components::updatePositionAndRemoveInvisible() {
  uint16_t y = 95;
  uint8_t i = 0;
  for (; i < COMPONENT_COUNT; i++) {
    if (m_components[i] == NULL) {
      break;
    }

    m_components[i]->setY(y);
    y += m_components[i]->getHeight();

    if (y > 320) {
      // Not fully visible, the next will be invisible
      break;
    }

    y += 2;
  }

  for (; i < COMPONENT_COUNT; i++) {
    m_components[i] = NULL;
  }
}

//! Return the component at position index
BaseComponent* Components::get(uint8_t index) {
  return m_components[index];
}

//! remove the element at index, but does not delete it
void Components::remove(uint8_t index) {
  for (uint8_t i = index; i < COMPONENT_COUNT - 1; i++) {
    m_components[i] = m_components[i + 1];
  }

  m_components[COMPONENT_COUNT - 1] = NULL;

  updatePositionAndRemoveInvisible();
}

//! Draw all components
void Components::draw() {
  for (uint8_t i = 0; i < COMPONENT_COUNT; i++) {
    if (m_components[i] == NULL) {
      return;
    }
    m_components[i]->draw();
  }
}
