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


#include "PCD8544_nano.h"
#include "config.h"

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include <avr/pgmspace.h>


#define PCD8544_CMD  LOW
#define PCD8544_DATA HIGH


/*
 * If this was a ".h", it would get added to sketches when using
 * the "Sketch -> Import Library..." menu on the Arduino IDE...
 */
#include "PCD8544_charset.cpp"


PCD8544::PCD8544(unsigned char sclk, unsigned char sdin,
                 unsigned char dc, unsigned char reset,
                 unsigned char sce):
    pin_sclk(sclk),
    pin_sdin(sdin),
    pin_dc(dc),
    pin_reset(reset),
    pin_sce(sce)
{}


void PCD8544::begin(unsigned char width, unsigned char height, unsigned char model)
{
    this->width = width;
    this->height = height;

    this->column = 0;
    this->line = 0;

    // Sanitize the custom glyphs...
    memset(this->custom, 0, sizeof(this->custom));

    // All pins are outputs (these displays cannot be read)...
    pinMode(this->pin_sclk, OUTPUT);
    pinMode(this->pin_sdin, OUTPUT);
    pinMode(this->pin_dc, OUTPUT);
    pinMode(this->pin_reset, OUTPUT);
    //pinMode(this->pin_sce, OUTPUT);

    // Reset the controller state...
    digitalWrite(this->pin_reset, HIGH);
    //digitalWrite(this->pin_sce, HIGH);
    digitalWrite(this->pin_reset, LOW);
    delay(100);
    digitalWrite(this->pin_reset, HIGH);

    // Set the LCD parameters...
    this->send(PCD8544_CMD, 0x21);  // extended instruction set control (H=1)
    this->send(PCD8544_CMD, 0x13);  // bias system (1:48)

    if (model == CHIP_ST7576)
    {
        this->send(PCD8544_CMD, 0xe0);  // higher Vop, too faint at default
        this->send(PCD8544_CMD, 0x05);  // partial display mode
    }
    else
    {
        this->send(PCD8544_CMD, NOKIA_LCD_CONTRAST);  // default Vop (3.06 + 66 * 0.06 = 7V)
        this->send(PCD8544_CMD, 0x06);  // Temperature Control Mode 2
    }

    this->send(PCD8544_CMD, 0x20);  // extended instruction set control (H=0)
    this->send(PCD8544_CMD, 0x09);  // all display segments on

    // Clear RAM contents...
    this->clear();

    // Activate LCD...
    this->send(PCD8544_CMD, 0x08);  // display blank
    this->send(PCD8544_CMD, 0x0c);  // normal mode (0x0d = inverse mode)
    delay(100);

    // Place the cursor at the origin...
    this->send(PCD8544_CMD, 0x80);
    this->send(PCD8544_CMD, 0x40);
}


void PCD8544::stop()
{
    this->clear();
    this->setPower(false);
}


void PCD8544::clear()
{
    this->setCursor(0, 0);

    for (unsigned short i = 0; i < this->width * (this->height/8); i++)
    {
        this->send(PCD8544_DATA, 0x00);
    }

    this->setCursor(0, 0);
}


void PCD8544::clearLine()
{
    this->setCursor(0, this->line);

    for (unsigned char i = 0; i < this->width; i++)
    {
        this->send(PCD8544_DATA, 0x00);
    }

    this->setCursor(0, this->line);
}


void PCD8544::setPower(bool on)
{
    this->send(PCD8544_CMD, on ? 0x20 : 0x24);
}


inline void PCD8544::display()
{
    this->setPower(true);
}


inline void PCD8544::noDisplay()
{
    this->setPower(false);
}


void PCD8544::setInverse(bool inverse)
{
    this->send(PCD8544_CMD, inverse ? 0x0d : 0x0c);
}


void PCD8544::home()
{
    this->setCursor(0, this->line);
}


void PCD8544::setCursor(unsigned char column, unsigned char line)
{
    this->column = (column*6) % this->width;                                //in original Library there is no *6
    this->line = (line % (this->height/9 + 1));

    this->send(PCD8544_CMD, 0x80 | this->column);
    this->send(PCD8544_CMD, 0x40 | this->line);
}

void PCD8544::setCursorInPixels(unsigned char column, unsigned char line)  //same as SetCursor, but the column is specified in pixels, not in 6-pixel wide characters
{
    this->column = (column % this->width);
    this->line = (line % (this->height/9 + 1));

    this->send(PCD8544_CMD, 0x80 | this->column);
    this->send(PCD8544_CMD, 0x40 | this->line);
}

void PCD8544::createChar(unsigned char chr, const unsigned char *glyph)
{
    // ASCII 0-31 only...
    if (chr >= ' ')
    {
        return;
    }

    this->custom[chr] = glyph;
}


#if ARDUINO < 100
void PCD8544::write(uint8_t chr)
#else
size_t PCD8544::write(uint8_t chr)
#endif
{
    // ASCII 7-bit only...
    if (chr >= 0x80)
    {
#if ARDUINO < 100
        return;
#else
        return 0;
#endif
    }

    const unsigned char *glyph;
    unsigned char pgm_buffer[5];

    if (chr >= ' ')
    {
        // Regular ASCII characters are kept in flash to save RAM...
        memcpy_P(pgm_buffer, &charset[chr - ' '], sizeof(pgm_buffer));
        glyph = pgm_buffer;
    }
    else
    {
        // Custom glyphs, on the other hand, are stored in RAM...
        if (this->custom[chr])
        {
            glyph = this->custom[chr];
        }
        else
        {
            // Default to a space character if unset...
            memcpy_P(pgm_buffer, &charset[0], sizeof(pgm_buffer));
            glyph = pgm_buffer;
        }
    }

    // Output one column at a time...
    for (unsigned char i = 0; i < 5; i++)
    {
        this->send(PCD8544_DATA, glyph[i]);
    }

    // One column between characters...
    this->send(PCD8544_DATA, 0x00);

    // Update the cursor position...
    this->column = (this->column + 6) % this->width;

    if (this->column == 0)
    {
        this->line = (this->line + 1) % (this->height/9 + 1);
    }

#if ARDUINO >= 100
    return 1;
#endif
}


void PCD8544::drawBitmap(const unsigned char *data, unsigned char columns, unsigned char lines)
{
    unsigned char scolumn = this->column;
    unsigned char sline = this->line;

    // The bitmap will be clipped at the right/bottom edge of the display...
    unsigned char mx = (scolumn + columns > this->width) ? (this->width - scolumn) : columns;
    unsigned char my = (sline + lines > this->height/8) ? (this->height/8 - sline) : lines;

    for (unsigned char y = 0; y < my; y++)
    {
        this->setCursorInPixels(scolumn, sline + y);

        for (unsigned char x = 0; x < mx; x++)
        {
            this->send(PCD8544_DATA, data[y * columns + x]);
        }
    }

    // Leave the cursor in a consistent position...
    this->setCursorInPixels(scolumn + columns, sline);
}


void PCD8544::drawColumn(unsigned char lines, unsigned char value)
{
    unsigned char scolumn = this->column;
    unsigned char sline = this->line;

    // Keep "value" within range...
    if (value > lines*8)
    {
        value = lines*8;
    }

    // Find the line where "value" resides...
    unsigned char mark = (lines*8 - 1 - value)/8;

    // Clear the lines above the mark...
    for (unsigned char line = 0; line < mark; line++)
    {
        this->setCursor(scolumn, sline + line);
        this->send(PCD8544_DATA, 0x00);
    }

    // Compute the byte to draw at the "mark" line...
    unsigned char b = 0xff;
    for (unsigned char i = 0; i < lines*8 - mark*8 - value; i++)
    {
        b <<= 1;
    }

    this->setCursor(scolumn, sline + mark);
    this->send(PCD8544_DATA, b);

    // Fill the lines below the mark...
    for (unsigned char line = mark + 1; line < lines; line++)
    {
        this->setCursor(scolumn, sline + line);
        this->send(PCD8544_DATA, 0xff);
    }

    // Leave the cursor in a consistent position...
    this->setCursor(scolumn + 1, sline);
}


void PCD8544::send(unsigned char type, unsigned char data)
{

    //
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA_4PIN)
    if (type)
        bitSet(PORTB,this->pin_dc - 8);
    else
        bitClear(PORTB,this->pin_dc -8);
    this->shiftOutFast(this->pin_sdin, this->pin_sclk, data);  //modified, see https://github.com/jenkie/Arduino-Pedelec-Controller/issues/22 Many thanks to pillepalle
#else
    digitalWrite(this->pin_dc, type);
    shiftOut(this->pin_sdin, this->pin_sclk, MSBFIRST, data);
#endif

}

//--- shiftOutFast - Shiftout method done in a faster way .. needed for tighter timer process
void PCD8544::shiftOutFast(unsigned char myDataPin, unsigned char myClockPin, unsigned char myDataOut)
{
//=== This function shifts 8 bits out MSB first much faster than the normal shiftOut function by writing directly to the memory address for port
//--- clear data pin
    bitClear(PORTB,myDataPin);

//Send each bit of the myDataOut byte MSBFIRST
    for (int i=7; i>=0; i--)
    {
        bitClear(PORTB,myClockPin-8);
//--- Turn data on or off based on value of bit
        if ( bitRead(myDataOut,i) == 1)
        {
            bitSet(PORTB,myDataPin-8);
        }
        else
        {
            bitClear(PORTB,myDataPin-8);
        }
//register shifts bits on upstroke of clock pin
        bitSet(PORTB,myClockPin-8);
//zero the data pin after shift to prevent bleed through
        bitClear(PORTB,myDataPin-8);
    }
//stop shifting
    bitClear(PORTB,myClockPin-8);
}

//Draws a vertical bargraph with an outer and inner frame. One pixel line at the bottom is left free. It is not used because of 1 pixel necessary distance to the next line of text below the bar.
//An (optional) horizontal limit line is drawn into the bar. If you do not want to use the limit line, then set limit to the same value as maxValue. E.G. drawVerticalBar(100,100,50);
//outerFrameInPixels: valid range:0..7
//innerFrameInPixels: valid range: outerFrameInPixels+innerFrameInPixels <=7 !!
void PCD8544::drawVerticalBar(word maxValue, word limit, word value, byte widthInPixels, byte heightInBytes, byte outerFrameInPixels, byte innerFrameInPixels)
{
    limit=constrain(limit,0,maxValue); //contrain limit to maximum value
    const byte xpos=this->column;
    const byte yposInBytes=this->line;
    const byte heightInPixels=8*heightInBytes;
    enum {OUTERFRAME, INNERFRAME, BAR} category;
    byte distanceFromTop;
    byte limitLinePattern[6]= {0,0,0,0,0,0}; //  Array size of 6 is sufficient, because display height is max 6 bytes = 48 Pixels
    byte pixelLimit=(byte)map(limit,0, maxValue,2, heightInPixels-outerFrameInPixels-3); //calc distance of the limit line from the bottom of the bar graph
    byte pixelValue;
    //calculate the number of pixels of the data bar depending on "value"
     pixelValue=constrain((byte)map(value, 0,maxValue,0, heightInPixels-2*outerFrameInPixels-2*innerFrameInPixels-1), 0, heightInPixels-2*outerFrameInPixels-2*innerFrameInPixels-1);
    //the limit line has the same number of pixels as the outer Frame (e.g. 1 or 2 pixels usually), but at least 1 pixel
    for(byte i=0; i<max(outerFrameInPixels,1); i++) //for all pixels of the limit line: create the bit pattern
    {
        distanceFromTop=heightInPixels-pixelLimit-outerFrameInPixels-1+i;
        limitLinePattern[distanceFromTop/8] |= 1<<distanceFromTop%8;  //draw limit line
    }
    //calculate top line of the data bar
    distanceFromTop=heightInPixels-pixelValue-outerFrameInPixels-innerFrameInPixels-1;

    for(byte y=0; y<heightInBytes; y++)
    {
        byte barPattern; //calculate the bit pattern for the data bar
        if(y <distanceFromTop/8)
            barPattern=0x00; //clear lines above the end of the bar
        else if(y >distanceFromTop/8)
            barPattern=0xff; //fill lines below the end of the bar
        else
            barPattern= 0xff<<(distanceFromTop%8); //calculate bit pattern

        setCursorInPixels(xpos,y+yposInBytes);
        for(byte x=0; x<widthInPixels; x++)
        {
            byte b=0; //1 byte is 8 vertical pixels on the display, LSB is on top.
            if(x<outerFrameInPixels) //x position is in OUTERFRAME
            {
                category=OUTERFRAME;
            }
            else if(x<outerFrameInPixels+innerFrameInPixels) //x position is in INNERFRAME
            {
                category=INNERFRAME;
            }
            else if(x<widthInPixels-outerFrameInPixels-innerFrameInPixels) //x position is in the BAR area
            {
                category=BAR;
            }
            else if(x<widthInPixels-outerFrameInPixels) //x position is in INNERFRAME
            {
                category=INNERFRAME;
            }
            else //x position is in OUTERFRAME
            {
                category=OUTERFRAME;
            }
            switch(category)
            {
                case OUTERFRAME: b=0xff; //all pixels filled
                    break;
                case INNERFRAME: b=0x00; //all pixels cleared
                    break;
                case BAR: b=barPattern;
                    break;
            }
            b^=limitLinePattern[y]; //draw the limit line
            //draw the top outer frame
            if(y==0) b|= (1<<outerFrameInPixels)-1; //calculate the bit pattern. e.g. for 2 pixels outer frame:  1<<2-1=4-1=3= 00000011
            //draw the bottom frame
            if(y==heightInBytes-1)
            {
                b|= 0xff<<(7-outerFrameInPixels);
                if(category==BAR)
                {  b&= ~(0xff<<(7-outerFrameInPixels-innerFrameInPixels) & 0xff>>(1+outerFrameInPixels));} //clear the inner frame
                b&= 0x7F; //Clear last pixel line. It is not used because of 1 pixel necessary distance to the next line of text below the bar
            }
            
            send(PCD8544_DATA, b);
        }
    }
    // Leave the cursor in a consistent position...
    setCursorInPixels(xpos+widthInPixels+1,yposInBytes); //set cursor on the right side of the bar with 1 pixel distance
}

/* vim: set expandtab ts=4 sw=4: */
#endif
