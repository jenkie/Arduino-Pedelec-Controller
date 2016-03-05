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

#include "MainView.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#include "TextComponent.h"
#include "Components.h"
#include "defines.h"

/**
 * Main view
 */

//! Constructor
MainView::MainView(Components* components)
        : m_wattage(0),
          m_components(components)
{
  model.addListener(this);
}

//! Destructor
MainView::~MainView() {
  // never deleted...
}

//! This view is now enabled and displayed
void MainView::activate() {
  BaseView::activate();
  m_components->deActivateChilren(m_active);
}

//! This view is now disabled and not displayed
void MainView::deactivate() {
  BaseView::deactivate();
  m_components->deActivateChilren(m_active);
}

//! Draw speed
void MainView::drawSpeed(bool clearScreen) {
  if (!m_active) {
    return;
  }

  const uint8_t speedY = 25;

  if (clearScreen) {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  } else {
    tft.setTextColor(ILI9341_WHITE);
  }

  uint16_t speed = model.getValue(VALUE_ID_SPEED);

  tft.setTextSize(4);
  tft.setCursor(45, speedY);
  String str = "";
  uint8_t speed10 = (speed / 100) % 10;

  if (speed10 == 0) {
    str += " ";
  } else {
    str += speed10;
  }

  str += (speed / 10) % 10;
  str += ".";
  str += speed % 10;
  tft.print(str);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);

  if (!clearScreen) {
    tft.setCursor(160, speedY - 5);
    tft.print("km");
    tft.setCursor(167, speedY + 17);
    tft.print("h");

    tft.drawLine(155, speedY + 12, 185, speedY + 12, ILI9341_WHITE);
    tft.drawLine(155, speedY + 13, 185, speedY + 13, ILI9341_WHITE);
  }
}

//! Draw battery
void MainView::drawBattery(bool clearScreen) {
  if (!m_active) {
    return;
  }

  if (!clearScreen) {
    tft.drawRect(0, 9, 29, 7*9, ILI9341_WHITE);
    tft.fillRect(10, 0, 9, 9, ILI9341_WHITE);
    tft.setTextColor(ILI9341_WHITE);
  } else {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  }

  uint16_t batteryVoltage = model.getValue(VALUE_ID_BATTERY_VOLTAGE_CURRENT);
  uint16_t batteryMaxVoltage = model.getValue(VALUE_ID_BATTERY_VOLTAGE_MAX);
  uint16_t batteryMinVoltage = model.getValue(VALUE_ID_BATTERY_VOLTAGE_MIN);

  uint8_t batterPercent = (batteryVoltage - batteryMinVoltage) * 100 / (batteryMaxVoltage - batteryMinVoltage);

  uint16_t batteryColor = RGB_TO_565(0, 255, 0);
  if (batterPercent <= 40) {
    batteryColor = ILI9341_YELLOW;
  }
  if (batterPercent <= 30) {
    batteryColor = RGB_TO_565(255, 0, 0);
  }

  for (uint8_t y = 0; y < 7; y++) {
    uint16_t barColor = ILI9341_BLACK;
    if ((6 - y) * 14 <= batterPercent) {
      barColor = batteryColor;
    }

    tft.fillRect(2, y * 9 + 10, 25, 7, barColor);
  }

  tft.setTextSize(2);

  tft.setCursor(0, 75);

  String strPercent = "";
  if (batterPercent < 10) {
    strPercent += " ";
  }

  strPercent += batterPercent;
  strPercent += "%";

  if (batterPercent < 100) {
    strPercent += " ";
  }

  tft.print(strPercent);
}

//! Battery percent, 0 ... n
void MainView::setWattage(uint16_t wattage) {
  if (m_wattage == wattage) {
    return;
  }

  m_wattage = wattage;
  drawWattage(true);
}

//! Draw wattage bar
void MainView::drawWattage(bool clearScreen) {
  if (!m_active) {
    return;
  }

  const uint8_t wattageBarHeight = 68;
  if (!clearScreen) {
    tft.drawRect(211, 2, 29, wattageBarHeight + 2, ILI9341_WHITE);
    tft.setTextColor(ILI9341_WHITE);
  } else {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  }

  uint16_t wattage = m_wattage;
  if (wattage > 500) {
    wattage = 500;
  }

  uint8_t h;
  if (wattage == 0) {
    h = 0;
  } else {
    h = (wattageBarHeight * (wattage / 10)) / 50;
  }
  uint8_t y = wattageBarHeight - h;

  tft.fillRect(213, 3, 25, y, ILI9341_BLACK);

  uint16_t  barColor = ILI9341_WHITE;
  if (m_wattage > 500) {
    barColor = ILI9341_RED;
  }
  tft.fillRect(213, 3 + y, 25, h, barColor);

  wattage = m_wattage;
  if (wattage > 9999) {
    wattage = 9999;
  }

  String strWattage = "";
  strWattage += m_wattage;
  strWattage += "W";

  while (strWattage.length() < 5) {
    strWattage = " " + strWattage;
  }

  tft.setCursor(180, 75);
  tft.print(strWattage);
}

//! Update full display
void MainView::updateDisplay() {
  if (!m_active) {
    return;
  }

  // Clear full screen
  tft.fillRect(0, 0, 240, 320, ILI9341_BLACK);

  drawSpeed(false);

  // Gesamt KM
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(60, 75);
  tft.print("12345 km");

  drawBattery(false);
  drawWattage(false);

  m_components->draw();
}

//! UP / DOWN Key
void MainView::movePosition(int8_t diff) {

}

//! Key (OK) pressed
ViewResult MainView::keyPressed() {
  ViewResult result;
  result.result = VIEW_RESULT_MENU;

  // Show settings menu
  result.value = 1;

  return result;
}

//! a value was changed
void MainView::onValueChanged(uint8_t valueId) {
  switch (valueId) {
    case VALUE_ID_SPEED:
      drawSpeed(true);
      break;
    case VALUE_ID_BATTERY_VOLTAGE_CURRENT:
      drawBattery(true);
      break;
  }
}
