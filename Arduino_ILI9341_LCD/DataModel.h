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
 * Model with all data which can be displayed by the different views
 */

 #include "Arduino.h"

enum IconId {
  ICON_ID_BLUETOOTH = (1 << 0),
  ICON_ID_BRAKE     = (1 << 1),
  ICON_ID_LIGHT     = (1 << 2),
  ICON_ID_HEART     = (1 << 3)
};

enum ValueId {
  //! the speed in 0.1 km/h and update display if needed
  VALUE_ID_SPEED = 0,

  //! Voltage when fully charged
  VALUE_ID_BATTERY_VOLTAGE_MAX,

  //! Voltage when dischaerged
  VALUE_ID_BATTERY_VOLTAGE_MIN,

  //! Current voltage
  VALUE_ID_BATTERY_VOLTAGE_CURRENT,

  VALUE_COUNT
};

class DataListener {
public:
  //! Icon changed
  virtual void onIconUpdate(uint8_t iconId) = 0;

  //! a value was changed
  virtual void onValueChanged(uint8_t valueId) = 0;

};

class DataModel {
  // Constructor / Destructor
public:
  //! Constructor
  DataModel();

  // public API
public:
  //! Clear an icon
  void clearIcon(uint8_t icon);

  //! Show an icon
  void showIcon(uint8_t icon);

  //! Bitmask with icon state
  uint8_t getIcon();

  //! Set the value
  void setValue(uint8_t valueId, uint16_t value);

  //! Get a value
  uint16_t getValue(uint8_t valueId);

public:
  //! Add a listener to get informed about changes
  void addListener(DataListener* listener);

private:
  //! fire an icon state change
  void fireIconUpdate(uint8_t iconId);

  //! fire a value changed
  void fireValueUpdate(uint8_t valueId);

  // Member
private:
  //! Bitmask with icon state
  uint8_t m_iconState;

  //! Listener list
  DataListener* m_listener[4];

  //! Values
  uint16_t m_values[VALUE_COUNT];
};
