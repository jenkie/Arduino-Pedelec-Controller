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

#include "MainViewEdit.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#include "TextComponent.h"
#include "Components.h"
#include "MenuEntries.h"

/**
 * Custmize main view
 */

//! Constructor
MainViewEdit::MainViewEdit(Components* components)
        : MainView(components),
          m_selectedId(0),
          m_selectionPosition(0),
          m_lastSelectedId(-1)
{
}

//! Destructor
MainViewEdit::~MainViewEdit() {
}

//! UP / DOWN Key
void MainViewEdit::movePosition(int8_t diff) {
  m_lastSelectedId = m_selectedId;

  while (diff > 0) {
    m_selectedId++;
    if (m_selectedId -1 >= COMPONENT_COUNT) {
      m_selectedId = COMPONENT_COUNT - 1;
      break;
    }

    if (m_components->get(m_selectedId) == NULL) {
      m_selectedId--;
      break;
    }

    diff--;
  }

  while (diff < 0) {
    m_selectedId--;
    if (m_selectedId <= 0) {
      m_selectedId = 0;
      break;
    }

    diff++;
  }

  drawSelection();
}

//! draw the selection mark
void MainViewEdit::drawSelection() {
  if (!m_active) {
    return;
  }

  if (m_lastSelectedId != -1) {
    BaseComponent* component = m_components->get(m_lastSelectedId);
    if (component != NULL) {
      tft.fillRect(0, component->getY() - 1, 240, component->getHeight() + 2, ILI9341_BLACK);
      component->draw();
    }
  }
  m_lastSelectedId = -1;

  BaseComponent* component = m_components->get(m_selectedId);
  if (component == NULL) {
    return;
  }

  tft.drawRect(0, component->getY(), 240, component->getHeight() + 1, ILI9341_MAGENTA);
  tft.drawRect(1, component->getY() - 1, 238, component->getHeight() + 1, ILI9341_MAGENTA);
}

//! Update full display
void MainViewEdit::updateDisplay() {
  if (!m_active) {
    return;
  }

  MainView::updateDisplay();

  drawSelection();
}

//! remove the selected component
void MainViewEdit::removeSelected() {
  m_components->remove(m_selectedId);
  updateDisplay();
}

//! Key (OK) pressed
ViewResult MainViewEdit::keyPressed() {
  ViewResult result;
  result.result = VIEW_RESULT_NOTHING;
  result.value = 0;

  if (m_selectedId != -1) {
    BaseComponent* component = m_components->get(m_selectedId);
    if (component != NULL) {
      result.result = VIEW_RESULT_MENU;
      result.value = MENU_ID_COMPONENT;
    }
  }

  return result;
}
