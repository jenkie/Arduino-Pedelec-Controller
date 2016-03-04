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

#include "BaseView.h"
#include "MenuEntries.h"

/**
 * Display a single menu
 */

class MenuView : public BaseView {
  // Constructor / Destructor
public:
  //! Constructor
  MenuView(Adafruit_ILI9341* tft);
  
  //! Destructor
  virtual ~MenuView();

  // public API
public:
  //! Update full display
  virtual void updateDisplay();

  //! UP / DOWN Key
  virtual void movePosition(int8_t diff);

  //! Key (OK) pressed
  virtual ViewResult keyPressed();

  //! Set the root menu ID
  void setRootMenuId(uint8_t menu);

private:
  //! Read the menu item from const memory (for AVR only)
  void readMenuItem(const void* p, MenuItem* current);

  //! Draw the menu to the display
  void drawMenu(bool clearScreen);

  //! Draw a single menuitem
  void drawMenuItem(MenuItem* menuItem, uint8_t menuIndex, uint16_t y, bool clearScreen);

  //! if a checkbox is selected
  bool isSelected(uint8_t index);

  //! Mark a checkbox as selected
  void selectCheckbox(uint8_t index);
  
  //! Mark a checkbox as not selected
  void unselectCheckbox(uint8_t index);
  
  // Member
private:
  //! Root menu index
  uint8_t m_menu;

  //! Last selected index, to also repaint
  int8_t m_lastSelectedMenuIndex;

  //! Count of currently visible items, -1 for not initialized
  int8_t m_itemCount;

  //! Selected menu index, 0 for first menu
  uint8_t m_selectedMenuIndex;

  //! Selected menu
  MenuItem m_selectedMenu;

  // Menu index to go back
  uint8_t m_backIndex;

  //! Selected checkboxes indexes, 0 is an empty field
  uint8_t m_selectedCheckboxes[30];
};

