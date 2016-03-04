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

#include "MenuView.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

/**
 * Display a single menu
 */

//! Constructor
MenuView::MenuView(Adafruit_ILI9341* tft)
        : BaseView(tft),
          m_menu(1),
          m_selectedMenuIndex(0),
          m_lastSelectedMenuIndex(-1),
          m_itemCount(-1),
          m_selectedMenu({0}),
          m_backIndex(0),
          m_selectedCheckboxes({0})
{
}
  
//! Destructor
MenuView::~MenuView() {
}

//! Read the menu item from const memory (for AVR only)
void MenuView::readMenuItem(const void* p, MenuItem* current) {
  static char menuString[32];
  
  uint8_t* pTarget = (uint8_t*)current;
  for (uint8_t i = 0; i < sizeof(MenuItem); i++) {
    *pTarget = pgm_read_byte_near(p + i);
    pTarget++;
  }

  for (uint8_t i = 0; i < sizeof(menuString); i++) {
    char c = pgm_read_byte_near(current->text + i);
    menuString[i] = c;
    if (c == 0) {
      break;
    }
  }

  current->text = menuString;
}

//! Draw a single menuitem
void MenuView::drawMenuItem(MenuItem* menuItem, uint8_t menuIndex, uint16_t y, bool clearScreen) {
  bool selected = m_selectedMenuIndex == menuIndex;
  uint16_t textColor;

  if (selected) {
    textColor = ILI9341_BLACK;
    clearScreen = true;
    m_selectedMenu = *menuItem;
  } else {
    textColor = ILI9341_WHITE;
  }

  if (clearScreen) {
    if (selected) {
      m_tft->fillRect(0, y - 5, 240, 25, ILI9341_YELLOW);
    } else {
      m_tft->fillRect(0, y - 5, 240, 25, ILI9341_BLACK);
    }
  }

  m_tft->setCursor(25, y);
  m_tft->setTextColor(textColor);
  m_tft->print(menuItem->text);

  if (menuItem->flags & MENU_CHECKBOX) {
    m_tft->drawRect(2, y, 15, 15, textColor);
  
    if (isSelected(menuItem->id)) {
      m_tft->drawLine(0, y - 2, 17, y - 2 + 17, textColor);
      m_tft->drawLine(1, y - 2, 18, y - 2 + 17, textColor);
      m_tft->drawLine(17, y - 2, 0, y - 2 + 17, textColor);
      m_tft->drawLine(18, y - 2, 1, y - 2 + 17, textColor);
    }
  }

  
  if (menuItem->flags & MENU_WITH_SUBMENU) {
    uint8_t offset = 3;
    for (uint8_t i = 1; i <= 6; i++) {
      m_tft->drawLine(2, y + offset, 15, y + offset, ILI9341_BLUE);
      offset++;
      if (i % 2 == 0) {
        offset += 2;
      }
    }
  } 

  if (menuItem->flags & MENU_BACK) {
    m_tft->drawLine(2, y + 6, 7, y + 2, ILI9341_BLUE);
    m_tft->drawLine(2, y + 7, 7, y + 3, ILI9341_BLUE);
    
    m_tft->drawLine(2, y + 6, 15, y + 6, ILI9341_BLUE);
    m_tft->drawLine(2, y + 7, 15, y + 7, ILI9341_BLUE);

    m_tft->drawLine(2, y + 6, 7, y + 10, ILI9341_BLUE);
    m_tft->drawLine(2, y + 7, 7, y + 11, ILI9341_BLUE);
  } 
}

//! Draw the menu to the display
void MenuView::drawMenu(bool clearScreen) {
  if (clearScreen) {
    // Clear full screen
    m_tft->fillRect(0, 0, 240, 30, RGB_TO_565(150, 150, 255));
    m_tft->fillRect(0, 30, 240, 290, ILI9341_BLACK);
  }

  m_tft->setTextSize(2);

  MenuItem current;

  void* pMenu = (void*)&Menu;
  uint16_t y = 40;

  uint8_t menuIndex = 0;
  for (uint8_t i = 0; i < Menu_Count; i++) {
    readMenuItem(pMenu, &current);
    
    if (current.id == m_menu && clearScreen) {
      m_tft->setTextColor(ILI9341_BLACK);
      m_tft->setCursor(25, 7);
      m_tft->print(current.text);
      m_backIndex = current.parentId;
    }

    if (current.parentId == m_menu) {
      if (!clearScreen) {
        if (m_lastSelectedMenuIndex == menuIndex) {
          drawMenuItem(&current, menuIndex, y, true);       
          m_lastSelectedMenuIndex = -1;
        }
        if (m_selectedMenuIndex == menuIndex) {
          drawMenuItem(&current, menuIndex, y, true);       
        }
      } else {
        drawMenuItem(&current, menuIndex, y, false);
      }
      y += 25;

      menuIndex++;
    }

    pMenu += sizeof(MenuItem);
  }

  // save count
  m_itemCount = menuIndex;
}

//! Update full display
void MenuView::updateDisplay() {
  if (!m_active) {
    return;
  }

  drawMenu(true);
}

//! UP / DOWN Key
void MenuView::movePosition(int8_t diff) {
  if (m_selectedMenuIndex == 0 && diff < 0) {
    return;
  }
  if (m_selectedMenuIndex == m_itemCount - 1 && diff > 0) {
    return;
  }
  
  m_lastSelectedMenuIndex = m_selectedMenuIndex;

  int16_t tmp = m_selectedMenuIndex + diff;

  if (tmp < 0) {
    tmp = 0;
  }

  if (tmp >= m_itemCount) {
    tmp = m_itemCount - 1;
  }

  m_selectedMenuIndex = tmp;
  
  drawMenu(false);
}

//! Key (OK) pressed
ViewResult MenuView::keyPressed() {
  ViewResult result;
  result.result = VIEW_RESULT_NOTHING;
  result.value = 0;
  
  if (m_selectedMenu.flags & MENU_WITH_SUBMENU) {
    setRootMenuId(m_selectedMenu.id);
    return result;
  }

  if (m_selectedMenu.flags & MENU_BACK) {
    if (m_backIndex == 0) {
      result.result = VIEW_RESULT_BACK;
      return result;
    } else {
      setRootMenuId(m_backIndex);
    }
    return result;
  }

  if (m_selectedMenu.flags & MENU_CHECKBOX) {
    // Force repaint
    m_lastSelectedMenuIndex = m_selectedMenu.id;

    if (isSelected(m_selectedMenu.id)) {
      unselectCheckbox(m_selectedMenu.id);
    } else {
      selectCheckbox(m_selectedMenu.id);
    }

    drawMenu(false);
    return result;
  }

  if (m_selectedMenu.flags & MENU_CHECKBOX) {
    if (isSelected(m_selectedMenu.id)) {
      result.result = VIEW_RESULT_CHECKBOX_CHECKED;
    } else {
      result.result = VIEW_RESULT_CHECKBOX_UNCHECKED;
    }
    
    result.value = m_selectedMenu.id;
    return result;
  }

  result.result = VIEW_RESULT_SELECTED;
  result.value = m_selectedMenu.id;
  return result;
}

//! if a checkbox is selected
bool MenuView::isSelected(uint8_t index) {
  for (uint8_t i = 0; i < sizeof(m_selectedCheckboxes); i++) {
    if (index == m_selectedCheckboxes[i]) {
      return true;
    }
  }

  return false;
}

//! Mark a checkbox as selected
void MenuView::selectCheckbox(uint8_t index) {
  unselectCheckbox(index);

  for (uint8_t i = 0; i < sizeof(m_selectedCheckboxes); i++) {
    if (0 == m_selectedCheckboxes[i]) {
      m_selectedCheckboxes[i] = index;
    }
  }
}

//! Mark a checkbox as not selected
void MenuView::unselectCheckbox(uint8_t index) {
  for (uint8_t i = 0; i < sizeof(m_selectedCheckboxes); i++) {
    if (index == m_selectedCheckboxes[i]) {
      m_selectedCheckboxes[i] = 0;
    }
  }
}

//! Set the root menu ID
void MenuView::setRootMenuId(uint8_t menu) {
  m_menu = menu;
  m_lastSelectedMenuIndex = -1;
  m_itemCount = -1;
  m_selectedMenuIndex = 0;
  m_selectedMenu.id = 0;
  m_backIndex = 0;
  for (uint8_t i = 0; i < sizeof(m_selectedCheckboxes); i++) {
    m_selectedCheckboxes[i] = 0;
  }

  drawMenu(true);
}




