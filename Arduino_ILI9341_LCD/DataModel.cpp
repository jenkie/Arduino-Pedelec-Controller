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

#include "DataModel.h"

/**
 * Model with all data which can be displayed by the different views
 */



//! Constructor
DataModel::DataModel()
         : m_iconState(0),
           m_listener({0}),
           m_values({0})
{
}

//! Clear an icon
void DataModel::clearIcon(uint8_t icon) {
  uint8_t oldValue = m_iconState;
  m_iconState &= ~icon;
  uint8_t diff = m_iconState ^ oldValue;

  fireIconUpdate(diff);
}

//! Show an icon
void DataModel::showIcon(uint8_t icon) {
  uint8_t oldValue = m_iconState;
  m_iconState |= icon;
  uint8_t diff = m_iconState ^ oldValue;

  fireIconUpdate(diff);
}

//! Set the value
void DataModel::setValue(uint8_t valueId, uint16_t value) {
  m_values[valueId] = value;

  fireValueUpdate(valueId);
}

//! Get a value
uint16_t DataModel::getValue(uint8_t valueId) {
  return m_values[valueId];
}

//! Bitmask with icon state
uint8_t DataModel::getIcon() {
  return m_iconState;
}

//! Add a listener to get informed about changes
void DataModel::addListener(DataListener* listener) {
  for (uint8_t i = 0; i < sizeof(m_listener) / sizeof(DataListener*); i++) {
    if (m_listener[i] == NULL) {
      m_listener[i] = listener;
      break;
    }
  }
}

//! fire an icon state change
void DataModel::fireIconUpdate(uint8_t iconId) {
  for (uint8_t i = 0; i < sizeof(m_listener) / sizeof(DataListener*); i++) {
    if (m_listener[i] != NULL) {
      m_listener[i]->onIconUpdate(iconId);
    }
  }
}

//! fire a value changed
void DataModel::fireValueUpdate(uint8_t valueId) {
  for (uint8_t i = 0; i < sizeof(m_listener) / sizeof(DataListener*); i++) {
    if (m_listener[i] != NULL) {
      m_listener[i]->onValueChanged(valueId);
    }
  }
}
