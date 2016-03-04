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
 * Display a diagram with a history of the value
 */

#include "DiagramComponent.h"

// for RGB Macro
#include "BaseView.h"


//! Constructor
DiagramComponent::DiagramComponent()
: m_data({1,
2,
3,
4,
5,
6,
7,
8,
9,
10,
11,
12,
13,
14,
15,
16,
17,
18,
19,
20,
21,
22,
23,
24,
25,
26,
26,
26,
26,
26,
26,
26,
27,
27,
27,
28,
28,
28,
27,
27,
27,
28,
28,
28,
29,
29,
29,
28,
28,
28,
29,
29,
29,
30,
30,
30,
29,
29,
29,
30,
30,
30,
31,
31,
31,
30,
30,
30,
31,
31,
31,
32,
32,
32,
31,
31,
31,
32,
32,
32,
33,
33,
33,
32,
32,
32,
33,
33,
33,
34,
34,
60,
60,
60,
33,
34,
34,
34,
35,
35,
35,
34,
34,
50,
51,
52,
53,
54,
55,
56,
40,
40,
40,
30,
30,
30,
20,
20,
20,
20})
{
}

//! Destructor
DiagramComponent::~DiagramComponent() {
}

//! Y Position on display
uint8_t DiagramComponent::getHeight() {
   return 60;
}

const uint16_t DIAGRAM_LINE_COLOR = RGB_TO_565(0xBE, 0x82, 0x34);
const uint16_t DIAGRAM_DATA_COLOR = RGB_TO_565(0x28, 0x2B, 0xDA);

//! Draw the component to the display
void DiagramComponent::draw() {
  tft.drawLine(0, m_y, 240, m_y, DIAGRAM_LINE_COLOR);
  tft.drawLine(0, m_y + 30, 240, m_y + 30, DIAGRAM_LINE_COLOR);
  tft.drawLine(0, m_y + 60, 240, m_y + 60, DIAGRAM_LINE_COLOR);

  for (uint16_t x = 20; x < 240; x += 40) {
    tft.drawLine(x, m_y + 1, x, m_y + 58, DIAGRAM_LINE_COLOR);
  }

  for (uint8_t i = 0; i < sizeof(m_data) - 1; i++) {
    uint8_t x = i * 2;
    uint16_t y1 = m_y + (60 - m_data[i]) - 1;
    uint16_t y2 = m_y + (60 - m_data[i + 1]) - 1;
    tft.drawLine(x, y1, x + 2, y2, DIAGRAM_DATA_COLOR);
    tft.drawLine(x, y1 + 1, x + 2, y2 + 1, DIAGRAM_DATA_COLOR);
  }

/*
   tft.setTextColor(ILI9341_WHITE);
   tft.setCursor(0, m_y + 2);
   tft.print(m_text.c_str());

   tft.setTextColor(ILI9341_YELLOW);
   tft.setCursor(240 - 5*12, m_y + 2);
   tft.print("75.50");*/
}
