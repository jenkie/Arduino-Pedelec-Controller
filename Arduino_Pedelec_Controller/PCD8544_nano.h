/*
 * PCD8544 - Interface with Philips PCD8544 (or compatible) LCDs.
 *
 * Copyright (c) 2010 Carlos Rodrigues <cefrodrigues@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


#include "config.h"

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
#ifndef PCD8544_H
#define PCD8544_H



#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif


// Chip variants supported...
#define CHIP_PCD8544 0
#define CHIP_ST7576  1


class PCD8544: public Print
{

public:
// All the pins can be changed from the default values...
#if HARDWARE_REV <= 5
#define DISPLAYPORT PORTB
#define DISPLAYPORT_DIR DDRB
    PCD8544(unsigned char sclk  = 1,   /* clock       (display pin 2) */
            unsigned char sdin  = 2,   /* data-in     (display pin 3) */
            unsigned char dc    = 3,   /* data select (display pin 4) */
#if (DISPLAY_TYPE==DISPLAY_TYPE_NOKIA_4PIN)
            unsigned char reset = 5,   /* reset       (display pin 8) */
            unsigned char sce   = 4);  /* enable      (display pin 5) */
#endif
#if (DISPLAY_TYPE==DISPLAY_TYPE_NOKIA_5PIN)
            unsigned char reset = 4,   /* reset       (display pin 8) */
            unsigned char sce   = 5);  /* enable      (display pin 5) */
#endif 
#endif

#if HARDWARE_REV == 20
#define DISPLAYPORT PORTH
#define DISPLAYPORT_DIR DDRH
    PCD8544(unsigned char sclk  = 4,   /* clock       (display pin 2) */
            unsigned char sdin  = 0,   /* data-in     (display pin 3) */
            unsigned char dc    = 1,   /* data select (display pin 4) */
#if (DISPLAY_TYPE==DISPLAY_TYPE_NOKIA_4PIN)
            unsigned char reset = 3,   /* reset       (display pin 8) */
            unsigned char sce   = 2);  /* enable      (display pin 5) */
#endif
#if (DISPLAY_TYPE==DISPLAY_TYPE_NOKIA_5PIN)
            unsigned char reset = 2,   /* reset       (display pin 8) */
            unsigned char sce   = 3);  /* enable      (display pin 5) */
#endif 
#endif


    // Display initialization (dimensions in pixels)...
    void begin(unsigned char width=84, unsigned char height=48, unsigned char model=CHIP_PCD8544);
    void stop();

    // Erase everything on the display...
    void clear();
    void clearLine();  // ...or just the current line

    // Control the display's power state...
    void setPower(bool on);

    // For compatibility with the LiquidCrystal library...
    void display();
    void noDisplay();

    // Activate white-on-black mode (whole display)...
    void setInverse(bool inverse);

    // Place the cursor at the start of the current line...
    void home();

    // Place the cursor at position (column, line)...
    void setCursor(unsigned char column, unsigned char line);

    //same as SetCursor, but the column is specified in pixels, not in 6-pixel wide characters
    void setCursorInPixels(unsigned char column, unsigned char line);

    // Assign a user-defined glyph (5x8) to an ASCII character (0-31)...
    void createChar(unsigned char chr, const unsigned char *glyph);

    void shiftOutFast(unsigned char myDataPin, unsigned char myClockPin, unsigned char myDataOut);

    // Write an ASCII character at the current cursor position (7-bit)...
#if ARDUINO < 100
    virtual void write(uint8_t chr);
#else
    virtual size_t write(uint8_t chr);
#endif

    // Draw a bitmap at the current cursor position...
    void drawBitmap(const unsigned char *data, unsigned char columns, unsigned char lines);

    // Draw a chart element at the current cursor position...
    void drawColumn(unsigned char lines, unsigned char value);

    //Draws a vertical bargraph with an outer and inner frame. One pixel line at the bottom is left free. It is not used because of 1 pixel necessary distance to the next line of text below the bar.
    //An (optional) horizontal limit line is drawn into the bar. If you do not want to use the limit line, then set limit to the same value as maxValue. E.G. drawVerticalBar(100,100,50);
    void drawVerticalBar(word maxValue, word limit, word value, byte widthInPixels, byte heightInBytes, byte outerFrameInPixels=2, byte innerFrameInPixels=1);

private:
    unsigned char pin_sclk;
    unsigned char pin_sdin;
    unsigned char pin_dc;
    unsigned char pin_reset;
    unsigned char pin_sce;

    // The size of the display, in pixels...
    unsigned char width;
    unsigned char height;

    // Current cursor position...
    unsigned char column;
    unsigned char line;

    // User-defined glyphs (below the ASCII space character)...
    const unsigned char *custom[' '];

    // Send a command or data to the display...
    void send(unsigned char type, unsigned char data);
};


#endif  /* PCD8544_H */


/* vim: set expandtab ts=4 sw=4: */
#endif
