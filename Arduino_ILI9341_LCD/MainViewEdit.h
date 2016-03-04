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

#include "MainView.h"

/**
 * Custmize main view
 */

class MainViewEdit : public MainView {
  // Constructor / Destructor
public:
  //! Constructor
  MainViewEdit(Components* components);

  //! Destructor
  virtual ~MainViewEdit();

  // public API
public:
  //! Update full display
  virtual void updateDisplay();

  //! UP / DOWN Key
  virtual void movePosition(int8_t diff);

  //! Key (OK) pressed
  virtual ViewResult keyPressed();

private:
  //! draw the selection mark
  void drawSelection();

  // Member
private:
  //! Selected element
  int8_t m_selectedId;

  //! Last selected element to repaint, -1 for none
  int8_t m_lastSelectedId;

  //! Positino, -1 top, 0 Element, 1 bottom
  int8_t m_selectionPosition;
};
