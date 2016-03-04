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

/**
 * Main view
 */

//! Constructor
MainView::MainView(Adafruit_ILI9341* tft)
        : BaseView(tft),
          m_speed(0),
          m_batteryMinVoltage(0),
          m_batteryMaxVoltage(50),
          m_batteryVoltage(25),
          m_wattage(0)
{
}

//! Destructor
MainView::~MainView() {
}

//! Draw speed
void MainView::drawSpeed(bool clearScreen) {
  if (!m_active) {
    return;
  }

  const uint8_t speedY = 25;

  if (clearScreen) {
    m_tft->setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  } else {
    m_tft->setTextColor(ILI9341_WHITE);
  }

  m_tft->setTextSize(4);
  m_tft->setCursor(45, speedY);
  String str = "";
  uint8_t speed10 = (m_speed / 100) % 10;

  if (speed10 == 0) {
    str += " ";
  } else {
    str += speed10;
  }

  str += (m_speed / 10) % 10;
  str += ".";
  str += m_speed % 10;
  m_tft->print(str);

  m_tft->setTextColor(ILI9341_WHITE);
  m_tft->setTextSize(2);

  if (!clearScreen) {
    m_tft->setCursor(160, speedY - 5);
    m_tft->print("km");
    m_tft->setCursor(167, speedY + 17);
    m_tft->print("h");
    m_tft->drawLine(155, speedY + 12, 185, speedY + 12, ILI9341_WHITE);
    m_tft->drawLine(155, speedY + 13, 185, speedY + 13, ILI9341_WHITE);
  }
}

//! Set the speed in 0.1 km/h and update display if needed
void MainView::setSpeed(uint16_t kmh) {
  if (m_speed == kmh) {
    return;
  }

  m_speed = kmh;
  drawSpeed(true);
}

//! Voltage when fully charged
void MainView::setBatteryMaxVoltage(uint16_t voltage) {
  if (m_batteryMaxVoltage == voltage) {
    return;
  }

  m_batteryMaxVoltage = voltage;
}

//! Voltage when dischaerged
void MainView::setBatteryMinVoltage(uint16_t voltage) {
  if (m_batteryMinVoltage == voltage) {
    return;
  }

  m_batteryMinVoltage = voltage;
}

//! Current voltage
void MainView::setBatteryVoltage(uint16_t voltage) {
  if (m_batteryVoltage == voltage) {
    return;
  }

  m_batteryVoltage = voltage;
  drawBattery(true);
}

//! Draw battery
void MainView::drawBattery(bool clearScreen) {
  if (!m_active) {
    return;
  }

  if (!clearScreen) {
    m_tft->drawRect(0, 9, 29, 7*9, ILI9341_WHITE);
    m_tft->fillRect(10, 0, 9, 9, ILI9341_WHITE);
    m_tft->setTextColor(ILI9341_WHITE);
  } else {
    m_tft->setTextColor(ILI9341_WHITE, ILI9341_BLACK);
  }

  uint16_t batteryColor = RGB_TO_565(0, 255, 0);

  uint8_t batterPercent = (m_batteryVoltage - m_batteryMinVoltage) * 100 / (m_batteryMaxVoltage - m_batteryMinVoltage);

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

    m_tft->fillRect(2, y * 9 + 10, 25, 7, barColor);
  }

  m_tft->setTextSize(2);

  m_tft->setCursor(0, 75);

  String strPercent = "";
  if (batterPercent < 10) {
    strPercent += " ";
  }

  strPercent += batterPercent;
  strPercent += "%";

  if (batterPercent < 100) {
    strPercent += " ";
  }

  m_tft->print(strPercent);
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
    m_tft->drawRect(211, 2, 29, wattageBarHeight + 2, ILI9341_WHITE);
    m_tft->setTextColor(ILI9341_WHITE);
  } else {
    m_tft->setTextColor(ILI9341_WHITE, ILI9341_BLACK);
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

  m_tft->fillRect(213, 3, 25, y, ILI9341_BLACK);

  uint16_t  barColor = ILI9341_WHITE;
  if (m_wattage > 500) {
    barColor = ILI9341_RED;
  }
  m_tft->fillRect(213, 3 + y, 25, h, barColor);

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

  m_tft->setCursor(180, 75);
  m_tft->print(strWattage);
}

//! Bluetooth enabled
void MainView::setBluetooth(bool bluetooth) {
  if (m_bluetooth == bluetooth) {
    return;
  }

  m_bluetooth = bluetooth;
  drawBluetooth(true);
}

//! Brakes enabled
void MainView::setBrakes(bool brakes) {
  if (m_brakes == brakes) {
    return;
  }

  m_brakes = brakes;
  drawBrakes(true);
}

//! Light on
void MainView::setLight(bool light) {
  if (m_light == light) {
    return;
  }

  m_light = light;
  drawLight(true);
}

// Icon position
const uint16_t ICON_Y = 100;
const uint16_t ICON_HEIGHT = 40;
const uint16_t ICON_DISABLED_COLOR = RGB_TO_565(25, 25, 25);

//! Draw Bluetooth Icon
void MainView::drawBluetooth(bool clearScreen) {
  if (!m_active) {
    return;
  }

  uint16_t ix = 20;
  uint16_t blX1 = ix;
  uint16_t blX2 = ix + 10;
  uint16_t blX3 = ix + 20;

  uint16_t iconColor;
  if (m_bluetooth) {
    iconColor = ILI9341_BLUE;
  } else {
    iconColor = ICON_DISABLED_COLOR;
  }

  m_tft->drawLine(blX2, ICON_Y, blX2, ICON_Y + ICON_HEIGHT, iconColor);

  m_tft->drawLine(blX2, ICON_Y, blX3, ICON_Y + ICON_HEIGHT/4, iconColor);
  m_tft->drawLine(blX3, ICON_Y + ICON_HEIGHT - ICON_HEIGHT/4, blX1, ICON_Y + 10, iconColor);

  m_tft->drawLine(blX2, ICON_Y + ICON_HEIGHT, blX3, ICON_Y + ICON_HEIGHT - ICON_HEIGHT/4, iconColor);
  m_tft->drawLine(blX3, ICON_Y + ICON_HEIGHT/4, blX1, ICON_Y + ICON_HEIGHT - 10, iconColor);

}

//! Draw Brakes Icon
void MainView::drawBrakes(bool clearScreen) {
  if (!m_active) {
    return;
  }

  uint16_t ix = 60;
  uint8_t breakRadius = 20;

  uint16_t iconColor;
  if (m_brakes) {
    iconColor = ILI9341_RED;
  } else {
    iconColor = ICON_DISABLED_COLOR;
  }

  m_tft->drawCircle(ix + breakRadius, ICON_Y + breakRadius, breakRadius, iconColor);
  m_tft->drawCircle(ix + breakRadius, ICON_Y + breakRadius, breakRadius-1, iconColor);

  m_tft->drawCircle(ix + breakRadius+14, ICON_Y + breakRadius, breakRadius, iconColor);
  m_tft->drawCircle(ix + breakRadius+14, ICON_Y + breakRadius, breakRadius-1, iconColor);

  m_tft->fillRect(ix + 7, ICON_Y, breakRadius + breakRadius, breakRadius + breakRadius + 2, ILI9341_BLACK);

  m_tft->drawCircle(ix + breakRadius + 7, ICON_Y + breakRadius, breakRadius, iconColor);
  m_tft->drawCircle(ix + breakRadius + 7, ICON_Y + breakRadius, breakRadius-1, iconColor);


  for (int8_t i = -1; i < 2; i++) {
    uint8_t bx = ix + breakRadius + i + 7;
    uint8_t y1 = ICON_Y + 6;
    uint8_t y2 = ICON_Y + ICON_HEIGHT - 7 - 6;

    m_tft->drawLine(bx, y1, bx, y2, iconColor);
    m_tft->drawLine(bx, y2 + 4, bx, y2 + 7, iconColor);
  }
}

//! Draw Light Icon
void MainView::drawLight(bool clearScreen) {
  if (!m_active) {
    return;
  }

  uint16_t ix = 130;
  uint8_t lightRadius = 12;

  uint16_t iconColor;
  if (m_light) {
    iconColor = ILI9341_YELLOW;
  } else {
    iconColor = ICON_DISABLED_COLOR;
  }

  m_tft->fillCircle(ix + lightRadius, ICON_Y + lightRadius, lightRadius, iconColor);
  m_tft->fillRect(ix + lightRadius - 4, ICON_Y + lightRadius*2, 9, 8, iconColor);

  m_tft->fillRect(ix + lightRadius - 2, ICON_Y + lightRadius*2+10, 5, 2, iconColor);
}


//! Update full display
void MainView::updateDisplay() {
  if (!m_active) {
    return;
  }

  // Clear full screen
  m_tft->fillRect(0, 0, 240, 320, ILI9341_BLACK);

  drawSpeed(false);

  // Gesamt KM
  m_tft->setTextColor(ILI9341_WHITE);
  m_tft->setTextSize(2);
  m_tft->setCursor(60, 75);
  m_tft->print("12345 km");

  drawBattery(false);
  drawWattage(false);

  // Icons
  m_tft->drawLine(0, 95, 240, 95, LINE_GRAY);
  drawBluetooth(false);
  drawBrakes(false);
  drawLight(false);
  m_tft->drawLine(0, 95 + 50, 240, 95 + 50, LINE_GRAY);






  m_tft->setTextColor(ILI9341_WHITE);
  m_tft->setTextSize(2);

  uint16_t textY = 240;
  m_tft->drawLine(0, textY-5, 240, textY-5, LINE_GRAY);

  m_tft->setCursor(0, textY);
  m_tft->print("Reichweite");

  // Zeichenbreite: 12px
  m_tft->setCursor(168, textY);
  m_tft->print("41.5km");

  m_tft->setCursor(0, textY + 17+7);
  m_tft->print("Gefahren");

  m_tft->setCursor(168, textY + 17+7);
  m_tft->print(" 1.2km");

  m_tft->setCursor(168, textY + 34+7);
  m_tft->print("0:12h");

  m_tft->setCursor(0, textY + 51+14);
  m_tft->print("Verbraucht");

  m_tft->setCursor(168, textY + 51+14);
  m_tft->print(" 240Wh");

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
