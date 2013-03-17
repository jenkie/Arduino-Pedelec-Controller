/*
Generic display init and update functions
Written by jenkie and Thomas Jarosch
Functions for the Nokia graphical screen mainly by m--k

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
#include "display.h"
#include "display_backlight.h"
#include "config.h"


#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
#include "PCD8544_nano.h"                    //for Nokia Display
static PCD8544 lcd;                          //for Nokia Display
nokia_screen_type nokia_screen = NOKIA_SCREEN_GRAPHIC; //startup screen on the Nokia display
nokia_screen_type nokia_screen_last = NOKIA_SCREEN_TEXT; //last screen type on the Nokia display
#endif



#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
#include "LiquidCrystalDogm.h"             //for 4bit (e.g. EA-DOGM) Display
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);   //for 4bit (e.g. EA-DOGM) Display
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
#include <SoftwareSerial.h>                //for Kingmeter J-LCD
SoftwareSerial mySerial(10, 11);           // RX (YELLOW cable of J-LCD), TX (GREEN-Cable)
byte jlcd_received[]= {0,0,0,0,0,0};
byte jlcd_receivecounter=0;
byte jlcd_maxspeed=0;                      //max speed set on display
byte jlcd_wheelsize=0;                     //wheel size set on display
boolean jlcd_lighton=false;                //backlight switched on?
byte jlcd_zerocounter=0;
unsigned long jlcd_last_transmission=millis(); //last time jlcd sent data--> still on?
#endif

static byte glyph1[] = {0x0b, 0xfc, 0x4e, 0xac, 0x0b}; //symbol for wh/km part 1
static byte glyph2[] = {0xc8, 0x2f, 0x6a, 0x2e, 0xc8}; //symbol for wh/km part 2
static byte glyph3[] = {0x44, 0x28, 0xfe, 0x6c, 0x28}; //bluetooth-symbol       check this out: http://www.carlos-rodrigues.com/projects/pcd8544/

unsigned long show_important_info_until = 0;
void display_show_important_info(const char *str, int duration_secs)
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    unsigned long seconds = 2;
    if (duration_secs)
        seconds = duration_secs;

    show_important_info_until = millis() + (seconds*1000);

#ifdef SUPPORT_DISPLAY_BACKLIGHT
    enable_backlight();
#endif

    // TODO: Implement and test 4bit display mode
    lcd.clear();
    lcd.setCursor(0, 2);
    lcd.print(str);
#endif
}

static void display_nokia_setup()    //first time setup of nokia display
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    lcd.begin(84, 48);
    lcd.createChar(0, glyph1);
    lcd.createChar(1, glyph2);
    lcd.createChar(2, glyph3);
    lcd.setCursor(4,0);
    lcd.print("V");
    lcd.setCursor(13,0);
    lcd.print("%");
    lcd.setCursor(3,1);
    lcd.print("W");
    lcd.setCursor(12,1);
    lcd.print("Wh");
    lcd.setCursor(0,2);
    lcd.print(" SPD   KM  CAD");
    lcd.setCursor(12,4);
    lcd.write(0);
    lcd.write(1);
#endif
}

static void display_4bit_setup()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
    lcd.begin(16, 2);
#endif
}

// Check if we should show an important info to the user
static bool handle_important_info_expire()
{
    if (show_important_info_until)
    {
        if (millis() < show_important_info_until)
        {
            // Important info still active
            return true;
        }

        // Important info expired
        show_important_info_until = 0;
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
        display_nokia_setup();
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
        display_4bit_setup();
#endif
    }

    // No important info shown
    return false;
}

static void display_4bit_update()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
    lcd.setCursor(0,0);
    lcd.print(voltage_display,1);
    lcd.print(" ");
    lcd.print(battery_percent_fromcapacity,0);
    lcd.print("%  ");
    lcd.setCursor(0,1);
    lcd.print(power,0);
    lcd.print("/");
    lcd.print(power_set);
    lcd.print("W      ");
#endif
}

static void display_nokia_update()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    lcd.setCursor(0,0);
    lcd.print(voltage_display,1);

    lcd.setCursor(6,0);
    if ((current_display<9.5)&&(current_display>0))
        {lcd.print(" ");}
    lcd.print(current_display,1);

    lcd.setCursor(10,0);
    if (battery_percent_fromcapacity<100)
        {lcd.print(" ");}
    if (battery_percent_fromcapacity<9.5)
        {lcd.print(" ");}
    lcd.print(battery_percent_fromcapacity);

    lcd.setCursor(0,1);
    if (power<99.5)
        {lcd.print(" ");}
    if (power<9.5)
        {lcd.print(" ");}
    lcd.print(power,0);

    lcd.setCursor(9,1);
    if (wh<99.5)
        {lcd.print(" ");}
    if (wh<9.5)
        {lcd.print(" ");}
    lcd.print(wh,0);

    lcd.setCursor(0,3);
    if (spd<9.5)
        {lcd.print(" ");}
    lcd.print(spd,1);

    lcd.setCursor(5,3);
    if (km<99.5)
        {lcd.print(" ");}
    if (km<9.5)
        {lcd.print(" ");}
    lcd.print(km,1);

    lcd.setCursor(11,3);
    if (cad<100)
        {lcd.print(" ");}
    if (cad<10)
        {lcd.print(" ");}
    lcd.print(cad,10);

    lcd.setCursor(0,4);
    if ( spd > 5.0)
        lcd.print(power/spd,1);
    else
        lcd.print("---");
    lcd.print("/");
    if ( km > 0.1)
        lcd.print(wh/km,1);
    else
        lcd.print("---");
    lcd.print(" ");

    lcd.setCursor(0,5);
//lcd.print(millis()/60000.0,1);   //uncomment this to display minutes since startup
//lcd.print(" Minuten");
#ifdef SUPPORT_BMP085
    //lcd.print(temperature,1);
    //lcd.print(" ");
    lcd.print(slope,0);
    lcd.print("% ");
    lcd.print((int)altitude);
    lcd.print(" ");
#endif
#ifdef SUPPORT_HRMI
    lcd.print((byte) pulse_human);
    lcd.print(" ");
#endif
    lcd.print(range,0);
    lcd.print("km ");
#if HARDWARE_REV >=2
    lcd.setCursor(13,5);
    if (digitalRead(bluetooth_pin)==1)
        {lcd.write(2);}
    else
        {lcd.print(" ");}
#endif
#endif // (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
}

void jlcd_update(byte battery, unsigned int wheeltime, byte error, int power)
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
    if (mySerial.available())
    {
        jlcd_last_transmission=millis();
        jlcd_receivecounter++;
        byte receivedbyte=mySerial.read();
        if (receivedbyte == 0x46)                         //start of new transmission frame detected-->last one is complete?
        {
            if (jlcd_receivecounter==6)                  //--> yes it is
            {
                jlcd_zerocounter=0;
                jlcd_lighton=(jlcd_received[1]&(byte)128)>>7;
                jlcd_received[1]=jlcd_received[1]&(byte)127;
                if (jlcd_received[1]<6)                      //set the assist-level (via poti-stat)
                    poti_stat=map(jlcd_received[1],1,5,0,1023);
                if (jlcd_received[1]==16)                    //16 means walk-mode
                    throttle_stat=200;
                else
                    throttle_stat=0;
                jlcd_maxspeed=10+jlcd_received[2]>>3;        //this is the max-speed set by J-LCD
                jlcd_wheelsize=jlcd_received[2]&(byte)7;     //this is the wheel-size set by J-LCD
            }
            jlcd_receivecounter=0;
            //-------------------------------------------Output to J-LCD start
            mySerial.write(0X46);
            mySerial.write(battery);
            mySerial.write((byte)(power/12.7));
            mySerial.write(highByte(wheeltime));
            mySerial.write(lowByte(wheeltime));
            mySerial.write(0X7D);
            mySerial.write(error);
            mySerial.write(battery^(byte)(power/12.7)^highByte(wheeltime)^lowByte(wheeltime)^0X7D^error); //this is XOR-checksum
            //-------------------------------------------Output to J-LCD end
        }
        else
            jlcd_received[jlcd_receivecounter]=receivedbyte;
        if (jlcd_receivecounter>5)
            jlcd_receivecounter=0;

    }
    if ((millis()-jlcd_last_transmission)>3000)
    {
        throttle_stat=0;
        poti_stat=0;
#if HARDWARE_REV >=2
        digitalWrite(fet_out,HIGH);              //J-LCD turned off
#endif
    }
#endif //(DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)

}


void display_init()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA_5PIN)
    // Note: Pin 13 is currently the SCE line, see PCD8544_nano.h
    pinMode(13,OUTPUT);
    digitalWrite(13,LOW);
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    display_nokia_setup();
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
    display_4bit_setup();
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
    mySerial.begin(9600);
#endif
}

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)

///////////////////////// additional code for a graphical screen design on the Nokia Display by m--k  ///////////////////////////////////////////////
//all bitmaps are created using this great bitmap generator http://www.introtoarduino.com/utils/pcd8544.html
//big characters 0-9 for displaying speed in 9x16 pixel format.
//big characters for the numbers 0..9  in 9x15 pixel size, used for displaying the speed
const byte bitmapBigNumber[10][2 * 9] = {{  0xFC, 0xFE, 0x07, 0x03, 0x03, 0x03, 0x07, 0xFE, 0xFC,   0x1F, 0x3F, 0x70, 0x60, 0x60, 0x60, 0x70, 0x3F, 0x1F},
    {   0x00, 0x18, 0x1C, 0x0E, 0xFF, 0xFF, 0x00, 0x00, 0x00,   0x00, 0x00, 0x60, 0x60, 0x7F, 0x7F, 0x60, 0x60, 0x00},
    {   0x0C, 0x0E, 0x07, 0x03, 0x03, 0x83, 0xC7, 0xFE, 0x7C,   0x70, 0x78, 0x7C, 0x6E, 0x67, 0x63, 0x61, 0x60, 0x60},
    {   0x0C, 0x0E, 0x07, 0x03, 0xC3, 0xC3, 0xE7, 0xFE, 0x3C,   0x18, 0x38, 0x70, 0x60, 0x60, 0x60, 0x71, 0x3F, 0x1F},
    {   0xC0, 0xE0, 0x70, 0x38, 0x1C, 0xFE, 0xFF, 0x00, 0x00,   0x07, 0x07, 0x06, 0x06, 0x06, 0x7F, 0x7F, 0x06, 0x06},
    {   0x7F, 0x7F, 0x63, 0x63, 0x63, 0x63, 0xE3, 0xC3, 0x83,   0x1C, 0x3C, 0x70, 0x60, 0x60, 0x60, 0x70, 0x3F, 0x1F},
    {   0xFC, 0xFE, 0x87, 0xC3, 0xC3, 0xC3, 0xC7, 0x8E, 0x0C,   0x1F, 0x3F, 0x71, 0x60, 0x60, 0x60, 0x71, 0x3F, 0x1F},
    {   0x03, 0x03, 0x03, 0x03, 0xC3, 0xF3, 0x7F, 0x1F, 0x07,   0x00, 0x70, 0x7C, 0x1F, 0x07, 0x01, 0x00, 0x00, 0x00},
    {   0x3C, 0xFE, 0xE7, 0xC3, 0xC3, 0xC3, 0xE7, 0xFE, 0x3C,   0x1F, 0x3F, 0x71, 0x60, 0x60, 0x60, 0x71, 0x3F, 0x1F},
    {   0x7C, 0xFE, 0xC7, 0x83, 0x83, 0x83, 0xC7, 0xFE, 0xFC,   0x00, 0x60, 0x61, 0x71, 0x31, 0x19, 0x1C, 0x0F, 0x07}
};
const byte bitmapBigComma[2 * 3] = {    0x00, 0x00, 0x00, 0xC0, 0xF0, 0x30};  //comma character, 3 bits wide, for big numbers
const byte bitmapBigkmh[2 * 9] = { 0xBC, 0x90, 0xA8, 0x80, 0xB8, 0x88, 0xB8, 0x88, 0xB8, 0x00, 0x00, 0x00, 0x3E, 0x08, 0x38, 0x00, 0x00, 0x00}; // km/h
const byte bitmapBigSpace[2 * 9] = {    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // " "

const byte bitmapBrakeSymbol[10] = {0x3C, 0x66, 0xC3, 0x18, 0x3C, 0x3C, 0x18, 0xC3, 0x66, 0x3C}; //Symbol for showing that the bikes brake is active
const byte bitmapBrakeSymbolClear[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //10x0 empty rectangle for clearing the brake symbol
const byte character_average_symbol[5] = {0x38, 0x64, 0x54, 0x4C, 0x38};

void drawSpeed(float speed, byte xpos, byte ypos) //print the speed in big 9x16 pixel characters, e.g. "27,6"
{
    int speed_digits = (int)(speed*10.0+0.5); //calculate the speed with 3 digits, with rounding, e.g. 20,29123 -> 203  -> display "20,3"
    lcd.setCursorInPixels(xpos,ypos);
    if(speed_digits<100) //<10kmh
    {
        lcd.drawBitmap(bitmapBigSpace, 9,2);
    }
    else
    {
        if(speed_digits<1000) lcd.drawBitmap(bitmapBigNumber[speed_digits/100], 9,2);  //max supported speed is 99,9kmh, should be enough!
    }
    lcd.setCursorInPixels(xpos+10,ypos);
    lcd.drawBitmap(bitmapBigNumber[(speed_digits/10)%10], 9,2);
    lcd.setCursorInPixels(xpos+19,ypos);
    lcd.drawBitmap(bitmapBigComma, 3,2);
    lcd.setCursorInPixels(xpos+23,ypos);
    lcd.drawBitmap(bitmapBigNumber[speed_digits%10], 9,2);
    lcd.setCursorInPixels(xpos+33,ypos);
    lcd.drawBitmap(bitmapBigkmh, 9,2);
}

void printTime(unsigned long sec)  //print time in exactly 5 characters: in the format "mm:ss" or "hh:mm", if the time is >1 hour
{
    word hours = sec/3600UL;
    byte minutes = (sec/60UL) % 60UL; //numberOfMinutes(val);
    byte seconds = sec % 60UL; //numberOfSeconds(val);
    word first,second; //only 2 numbers are displayed: either hours:minutes or minutes:seconds

    if(hours>0)
    {first=hours; second=minutes;}
    else
    {first=minutes; second=seconds;}
    if(first<10) lcd.print("0");
    lcd.print(first);
    lcd.print(":");
    if(second<10) lcd.print("0");
    lcd.print(second);
}

void printTripDistance(float km)  // print distance in exactly 6 characters, left aligned: "1234km" or "123km " or "12,1km" or "9,1km "
{
    if (km<10.0) {lcd.print(km,1); lcd.print("km ");}
    else
    {
        if (km<100)
        {lcd.print(km,1); lcd.print("km");}
        else
        {
            if(km<1000.0)
            {
                lcd.print(km,0); lcd.print("km ");
            }
            else {lcd.print(km,0); lcd.print("km");}
        }
    }
}

static void display_nokia_update_graphic()
{
    //print range in km in the top left corner
    lcd.setCursorInPixels(0,0);
    lcd.print(range,0); lcd.print("km ");

    lcd.setCursorInPixels(42-10,0);
    if(brake_stat==0) //if brake is active
    {lcd.drawBitmap(bitmapBrakeSymbol, 10,1);}
    else  //clear the brake symbol
    {lcd.drawBitmap(bitmapBrakeSymbolClear, 10,1);}
#if HARDWARE_REV >=2
    lcd.setCursor(8,0);
    if (digitalRead(bluetooth_pin)==1)
    {lcd.write(2);}
    else
    {lcd.print(" ");}
#endif

    //print the electrical power (Watt) in the top right corner
    lcd.setCursorInPixels(84-4*6,0);
    if (power<99.5) {lcd.print(" ");}
    if (power<9.5)  {lcd.print(" ");}
    lcd.print(power,0);
    lcd.print("W");

    drawSpeed(spd, 20, 2); //centered vertically

    //print the trip distance in the lower left corner
    lcd.setCursorInPixels(0,5);
    printTripDistance(km); //todo: implement real trip distance! currently the distance since last changing of the battery is displayed.

    //print a vertical bargraph on the right side:
    //- the full range of the bargraph equals to the max selectable power (power_max or power_poti_max)
    //- the limit line shows the target power that is selected by the user (power_set_for_display)
    //- the bar shows the currenlty measured electrical power that the moter consumes
    lcd.setCursorInPixels(72,1);
    lcd.drawVerticalBar((word)(max(power_max,power_poti_max)), (word)(max(power_set,0)), (word)(power), 11, 4);

    //print battery percent left
    lcd.setCursorInPixels(0,1);
    lcd.drawVerticalBar((word)(100), (word)(battery_percent_fromvoltage), (word)(battery_percent_fromcapacity), 11, 4);

    //print the trip-time in the lower right corner (displayed in the last 5 characters in this line)
    lcd.setCursorInPixels(84-5*6,5);
    //todo: implement real trip time measurement, including timeouts. Currently it is only the time after the last reset
    printTime(millis() / 1000UL);  //millis can be used, because they roll over only after 50 days
}
#endif

void display_update()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    if (handle_important_info_expire())
        return;

    if (spd>0)
        nokia_screen=NOKIA_SCREEN_GRAPHIC;
    else
        nokia_screen=NOKIA_SCREEN_TEXT;
    if (nokia_screen!=nokia_screen_last)
    {
        if (nokia_screen==NOKIA_SCREEN_TEXT)
            display_nokia_setup();
        else
            lcd.clear();
        nokia_screen_last=nokia_screen;
    }
    switch(nokia_screen)
    {
        case NOKIA_SCREEN_GRAPHIC: display_nokia_update_graphic();  break;
        case NOKIA_SCREEN_TEXT: display_nokia_update();  break;
    }
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
    display_4bit_update();
#endif
#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
    jlcd_update(2,wheel_time,0,power);
#endif
}
