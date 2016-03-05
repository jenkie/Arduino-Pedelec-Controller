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

#include "IconComponent.h"
#include "defines.h"

#define ICON_TOP_Y (m_y + 2)

//! Constructor
IconComponent::IconComponent() {
  model.addListener(this);
}

//! Destructor
IconComponent::~IconComponent() {
  model.removeListener(this);
}

const uint16_t ICON_HEIGHT = 40;

//! Y Position on display
uint8_t IconComponent::getHeight() {
   return ICON_HEIGHT + 2 + 3;
}

const uint16_t ICON_DISABLED_COLOR = RGB_TO_565(60, 60, 60);

//! Draw Bluetooth Icon
void IconComponent::drawBluetooth(bool clearScreen) {
  if (!m_active) {
    return;
  }

  uint16_t ix = 20;
  uint16_t blX1 = ix;
  uint16_t blX2 = ix + 10;
  uint16_t blX3 = ix + 20;

  uint16_t iconColor;
  if (model.getIcon() & ICON_ID_BLUETOOTH) {
    iconColor = ILI9341_BLUE;
  } else {
    iconColor = ICON_DISABLED_COLOR;
  }

  tft.drawLine(blX2, ICON_TOP_Y, blX2, ICON_TOP_Y + ICON_HEIGHT, iconColor);

  tft.drawLine(blX2, ICON_TOP_Y, blX3, ICON_TOP_Y + ICON_HEIGHT/4, iconColor);
  tft.drawLine(blX3, ICON_TOP_Y + ICON_HEIGHT - ICON_HEIGHT/4, blX1, ICON_TOP_Y + 10, iconColor);

  tft.drawLine(blX2, ICON_TOP_Y + ICON_HEIGHT, blX3, ICON_TOP_Y + ICON_HEIGHT - ICON_HEIGHT/4, iconColor);
  tft.drawLine(blX3, ICON_TOP_Y + ICON_HEIGHT/4, blX1, ICON_TOP_Y + ICON_HEIGHT - 10, iconColor);

}

//! Draw Brakes Icon
void IconComponent::drawBrakes(bool clearScreen) {
  if (!m_active) {
    return;
  }

  uint16_t ix = 60;
  uint8_t breakRadius = 20;

  uint16_t iconColor;
  if (model.getIcon() & ICON_ID_BRAKE) {
    iconColor = ILI9341_RED;
  } else {
    iconColor = ICON_DISABLED_COLOR;
  }

  tft.drawCircle(ix + breakRadius, ICON_TOP_Y + breakRadius, breakRadius, iconColor);
  tft.drawCircle(ix + breakRadius, ICON_TOP_Y + breakRadius, breakRadius-1, iconColor);

  tft.drawCircle(ix + breakRadius+14, ICON_TOP_Y + breakRadius, breakRadius, iconColor);
  tft.drawCircle(ix + breakRadius+14, ICON_TOP_Y + breakRadius, breakRadius-1, iconColor);

  tft.fillRect(ix + 7, ICON_TOP_Y, breakRadius + breakRadius, breakRadius + breakRadius + 2, ILI9341_BLACK);

  tft.drawCircle(ix + breakRadius + 7, ICON_TOP_Y + breakRadius, breakRadius, iconColor);
  tft.drawCircle(ix + breakRadius + 7, ICON_TOP_Y + breakRadius, breakRadius-1, iconColor);


  for (int8_t i = -1; i < 2; i++) {
    uint8_t bx = ix + breakRadius + i + 7;
    uint8_t y1 = ICON_TOP_Y + 6;
    uint8_t y2 = ICON_TOP_Y + ICON_HEIGHT - 7 - 6;

    tft.drawLine(bx, y1, bx, y2, iconColor);
    tft.drawLine(bx, y2 + 4, bx, y2 + 7, iconColor);
  }
}

//! Draw Light Icon
void IconComponent::drawLight(bool clearScreen) {
  if (!m_active) {
    return;
  }

  uint16_t ix = 130;
  uint8_t lightRadius = 12;

  uint16_t iconColor;
  if (model.getIcon() & ICON_ID_LIGHT) {
    iconColor = ILI9341_YELLOW;
  } else {
    iconColor = ICON_DISABLED_COLOR;
  }

  tft.fillCircle(ix + lightRadius, ICON_TOP_Y + lightRadius, lightRadius, iconColor);
  tft.fillRect(ix + lightRadius - 4, ICON_TOP_Y + lightRadius * 2, 9, 8, iconColor);

  tft.fillRect(ix + lightRadius - 2, ICON_TOP_Y + lightRadius * 2 + 10, 5, 2, iconColor);
}

//! Draw the component to the display
void IconComponent::draw() {
  drawBluetooth(true);
  drawBrakes(true);
  drawLight(true);
}

//! Icon changed
void IconComponent::onIconUpdate(uint8_t iconId) {
  if (iconId & ICON_ID_BLUETOOTH) {
    drawBluetooth(true);
  }
  if (iconId & ICON_ID_BRAKE) {
    drawBrakes(true);
  }
  if (iconId & ICON_ID_LIGHT) {
    drawLight(true);
  }
}
