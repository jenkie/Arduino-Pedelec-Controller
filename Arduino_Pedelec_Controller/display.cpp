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
#include "config.h"
#include "display.h"
#include "display_backlight.h"
#include "menu.h"
#include "globals.h"

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
#include "PCD8544_nano.h"                    //for Nokia Display
static PCD8544 lcd;                          //for Nokia Display
nokia_screen_type nokia_screen = NOKIA_SCREEN_GRAPHIC; //startup screen on the Nokia display
nokia_screen_type nokia_screen_last = NOKIA_SCREEN_TEXT; //last screen type on the Nokia display
boolean display_force_text = false;
#endif



#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
#include "LiquidCrystalDogm.h"             //for 4bit (e.g. EA-DOGM) Display
static LiquidCrystal lcd(13, 12, 11, 10, 9, 8);   //for 4bit (e.g. EA-DOGM) Display
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_SERIAL)
#include "serial_lcd.h"
static SerialLCD lcd(serial_display_16x2_pin);                      //16x2 New Haven display connected via one serial pin
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
#include <SoftwareSerial.h>                //for Kingmeter J-LCD
static SoftwareSerial mySerial(10, 11);           // RX (YELLOW cable of J-LCD), TX (GREEN-Cable)
byte jlcd_received[]= {0,0,0,0,0,0};
byte jlcd_receivecounter=0;
byte jlcd_maxspeed=0;                      //max speed set on display
byte jlcd_wheelsize=0;                     //wheel size set on display
boolean jlcd_lighton=false;                //backlight switched on?
byte jlcd_zerocounter=0;
unsigned long jlcd_last_transmission=millis(); //last time jlcd sent data--> still on?
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_BMS)
#include <SoftwareSerial.h>                //for BMS Battery S-LCD
static SoftwareSerial mySerial(10, 11);           // RX (YELLOW cable of S-LCD), TX (GREEN-Cable)
byte slcd_received[]= {0,0,0,0,0,0};
byte slcd_receivecounter=0;
boolean slcd_lighton=false;                //backlight switched on?
byte slcd_zerocounter=0;
unsigned long slcd_last_transmission=millis(); //last time Slcd sent data--> still on?
#endif

static const PROGMEM byte glyph1[] = {0x0b, 0xfc, 0x4e, 0xac, 0x0b}; //symbol for wh/km part 1
static const PROGMEM byte glyph2[] = {0xc8, 0x2f, 0x6a, 0x2e, 0xc8}; //symbol for wh/km part 2
static const PROGMEM byte glyph3[] = {0x44, 0x28, 0xfe, 0x6c, 0x28}; //bluetooth-symbol       check this out: http://www.carlos-rodrigues.com/projects/pcd8544/

unsigned long show_important_info_until = 0;
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA) || (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
static void prepare_important_info(int duration_secs)
{
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
}
#endif

void display_show_important_info(const char *str, int duration_secs)
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA) || (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    prepare_important_info(duration_secs);
    lcd.print(str);
#endif
}

void display_show_important_info(const __FlashStringHelper *str, int duration_secs)
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA) || (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    prepare_important_info(duration_secs);
    lcd.print(str);
#endif
}


// Calculate length of number.
// Don't use sprintf() here is it sucks up 1.5kb of code space.
static byte calc_number_length(const unsigned long x)
{
    unsigned long i=1, j=10;

    while ( x > ( j-1 ) )
    {
        i=i+1;
        j=j*10;

        // endless loop protection
        if (i > 20)
            break;
    }

    return i;
}

void display_show_welcome_msg()
{
    display_show_important_info(FROM_FLASH(msg_welcome), 5);

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    lcd.setCursor(0, 5);

    // Show total mileage (right aligned)
    unsigned long total_km = (odo * wheel_circumference/1000);
    byte number_len = calc_number_length(total_km);
    // Safety check
    if (number_len >= 12)
        return;

    number_len = 12 - number_len;
    while(number_len > 1)
    {
        lcd.print(MY_F(" "));
        --number_len;
    }

    lcd.print(total_km);
    lcd.print(MY_F(" km"));
#endif
}

#if defined(SUPPORT_BMP085) || defined(SUPPORT_DSPC01)
/*
    Own function to display temperature / altitude
    on the welcome screen. If you just see "welcome"
    and nothing else, the I2C communication is hanging.
*/
void display_show_welcome_msg_temp()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    lcd.setCursor(0,0);
    lcd.print((int)(temperature));
    lcd.print(MY_F(" C / "));

    lcd.print((int)(altitude_start));
    lcd.print(MY_F("m"));
#endif
}
#endif

static void display_nokia_setup()    //first time setup of nokia display
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    lcd.begin(84, 48);
    lcd.createChar(0, glyph1);
    lcd.createChar(1, glyph2);
    lcd.createChar(2, glyph3);
    lcd.setCursor(4,0);
    lcd.print(MY_F("V"));
    lcd.setCursor(13,0);
    lcd.print(MY_F("%"));
    lcd.setCursor(3,1);
    lcd.print(MY_F("W"));
    lcd.setCursor(12,1);
    lcd.print(MY_F("Wh"));
    lcd.setCursor(0,2);
    lcd.print(MY_F(" SPD   KM  CAD"));
    lcd.setCursor(12,4);
    lcd.write(0);
    lcd.write(1);
#endif
}

static void display_16x2_setup()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
    lcd.begin(16, 2);
#endif
#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_SERIAL)
    lcd.init();
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
        nokia_screen_last = NOKIA_SCREEN_IMPORTANT_INFO;
#endif
#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
        lcd.clear();
#endif
    }

    // No important info shown
    return false;
}

static void display_16x2_update()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    lcd.setCursor(0,0);
    lcd.print(voltage_display,1);
    lcd.print(MY_F(" "));
    lcd.print(battery_percent_fromcapacity);
    lcd.print(MY_F("%  "));
    lcd.setCursor(0,1);
    lcd.print(power,0);
    lcd.print(MY_F("/"));
    lcd.print(power_set);
    lcd.print(MY_F("W      "));
#endif
}

static void display_menu()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA) || (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    // Check if user has been idle for too long
    if (menu_activity_expire && millis() > menu_activity_expire)
    {
        menu_active = false;
        return;
    }
    if (!menu_changed)
        return;

    menu_changed = false;
    menu_activity_expire = millis() + menu_idle_timeout_secs * 1000;

    // Display the menu
    Menu const* menu = menu_system.get_current_menu();
    MenuComponent const* selected = menu->get_selected();

    lcd.clear();

    // Scroll to currently selected item for long menus
    byte current_lcd_row = 0;
    byte items_to_skip = 0;


#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    const byte num_screen_rows = 6;
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    const byte num_screen_rows = 2;
#endif

    if (menu->get_cur_menu_component_num() >= num_screen_rows)
        items_to_skip = menu->get_cur_menu_component_num() - num_screen_rows + 1;

    for (byte i = 0; i < menu->get_num_menu_components(); ++i)
    {
        // Handle scrolling of long menus
        if (items_to_skip)
        {
            --items_to_skip;
            continue;
        }

        lcd.setCursor(0, current_lcd_row);

        MenuComponent const *item = menu->get_menu_component(i);
        if (item == selected)
            lcd.print(MY_F("> "));
        else
            lcd.print(MY_F("  "));

        lcd.print(FROM_FLASH((item->get_name())));

        ++current_lcd_row;
        if (current_lcd_row == num_screen_rows)
            break;
    }
#endif
}

static void display_nokia_update()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    lcd.setCursor(0,0);
    lcd.print(voltage_display,1);

    lcd.setCursor(6,0);
    if ((current_display<9.5)&&(current_display>0))
        {lcd.print(MY_F(" "));}
    lcd.print(current_display,1);

    lcd.setCursor(10,0);
    if (battery_percent_fromcapacity<100)
        {lcd.print(MY_F(" "));}
    if (battery_percent_fromcapacity<9.5)
        {lcd.print(MY_F(" "));}
    lcd.print(battery_percent_fromcapacity);

    lcd.setCursor(0,1);
    if (power<99.5)
        {lcd.print(MY_F(" "));}
    if (power<9.5)
        {lcd.print(MY_F(" "));}
    lcd.print(power,0);

    lcd.setCursor(9,1);
    if (wh<99.5)
        {lcd.print(MY_F(" "));}
    if (wh<9.5)
        {lcd.print(MY_F(" "));}
    lcd.print(wh,0);

    lcd.setCursor(0,3);
    if (spd<9.5)
        {lcd.print(MY_F(" "));}
    lcd.print(spd,1);

    lcd.setCursor(5,3);
    if (km<99.5)
        {lcd.print(MY_F(" "));}
    if (km<9.5)
        {lcd.print(MY_F(" "));}
    lcd.print(km,1);

    lcd.setCursor(11,3);
    if (cad<100)
        {lcd.print(MY_F(" "));}
    if (cad<10)
        {lcd.print(MY_F(" "));}
    lcd.print(cad,10);

    lcd.setCursor(0,4);
    if ( spd > 5.0)
        lcd.print(power/spd,1);
    else
        lcd.print(MY_F("---"));
    lcd.print(MY_F("/"));
    if ( km > 0.1)
        lcd.print(wh/km,1);
    else
        lcd.print(MY_F("---"));
    lcd.print(MY_F(" "));

    lcd.setCursor(0,5);
//lcd.print(millis()/60000.0,1);   //uncomment this to display minutes since startup
//lcd.print(" Minuten");
#if defined(SUPPORT_BMP085) || defined(SUPPORT_DSPC01)
    //lcd.print(temperature,1);
    //lcd.print(" ");
    lcd.print(slope,0);
    lcd.print(MY_F("% "));
    lcd.print((int)altitude);
    lcd.print(MY_F(" "));
#endif
#ifdef SUPPORT_HRMI
    lcd.print((byte) pulse_human);
    lcd.print(MY_F(" "));
#endif
    lcd.print(range,0);
    lcd.print(MY_F("km "));
#if HARDWARE_REV >=2
    lcd.setCursor(13,5);
    if (digitalRead(bluetooth_pin)==1)
        {lcd.write(2);}
    else
        {lcd.print(MY_F(" "));}
#endif
#endif // (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
}

static void jlcd_update(byte battery, unsigned int wheeltime, byte error, int power)
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
            mySerial.write(0X9F);
            mySerial.write(error);
            mySerial.write(battery^(byte)(power/12.7)^highByte(wheeltime)^lowByte(wheeltime)^0X9F^error); //this is XOR-checksum
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

static void slcd_update(byte battery, unsigned int wheeltime, byte error)
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_BMS)
    if (mySerial.available())
    {
        slcd_last_transmission=millis();
        slcd_receivecounter++;
        byte receivedbyte=mySerial.read();
        if (receivedbyte==0xE) //end of transmission frame detected
          {slcd_receivecounter=6;}
        if(slcd_receivecounter>6) //transmission buffer overflow
          {slcd_receivecounter=0;}
        if (slcd_receivecounter == 6)            //start of new transmission frame detected
        {
            slcd_zerocounter=0;
            slcd_lighton=(slcd_received[1]&(byte)128)>>7;
            slcd_received[1]=slcd_received[1]&(byte)127;
            if (slcd_received[1]<6)                      //set the assist-level (via poti-stat)
                poti_stat=map(slcd_received[1],0,5,0,1023);
            if (slcd_received[1]==6)                    //16 means walk-mode
                throttle_stat=200;
            else
                throttle_stat=0;
            //-------------------------------------------Output to S-LCD start
            mySerial.write((byte)0x41);
            mySerial.write((byte)battery);
            mySerial.write((byte)0xFF);
            mySerial.write(highByte(wheeltime));
            mySerial.write(lowByte(wheeltime));
            mySerial.write(error);
            mySerial.write((byte)battery^(byte)0xFF^highByte(wheeltime)^lowByte(wheeltime)^error); //this is XOR-checksum
            //-------------------------------------------Output to S-LCD end
        }
        else
            slcd_received[slcd_receivecounter]=receivedbyte;
    }
    if ((millis()-slcd_last_transmission)>3000)
    {
        throttle_stat=0;
        poti_stat=0;
#if HARDWARE_REV >=2
        digitalWrite(fet_out,HIGH);              //S-LCD turned off
#endif
    }
#endif //(DISPLAY_TYPE & DISPLAY_TYPE_BMS)

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
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    display_16x2_setup();
#endif

#if ((DISPLAY_TYPE==DISPLAY_TYPE_KINGMETER)||(DISPLAY_TYPE==DISPLAY_TYPE_BMS))
    mySerial.begin(9600);
#endif
}

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)

///////////////////////// additional code for a graphical screen design on the Nokia Display by m--k  ///////////////////////////////////////////////
//all bitmaps are created using this great bitmap generator http://www.introtoarduino.com/utils/pcd8544.html
//big characters 0-9 for displaying speed in 9x16 pixel format.
//big characters for the numbers 0..9  in 9x15 pixel size, used for displaying the speed
static const byte bitmapBigNumber[10][2 * 9] PROGMEM = {
    {   0xFC, 0xFE, 0x07, 0x03, 0x03, 0x03, 0x07, 0xFE, 0xFC,   0x1F, 0x3F, 0x70, 0x60, 0x60, 0x60, 0x70, 0x3F, 0x1F},
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
static const PROGMEM byte bitmapBigComma_p[2 * 3] = {    0x00, 0x00, 0x00, 0xC0, 0xF0, 0x30};  //comma character, 3 bits wide, for big numbers
static const PROGMEM byte bitmapBigkmh_p[2 * 9] = { 0xBC, 0x90, 0xA8, 0x80, 0xB8, 0x88, 0xB8, 0x88, 0xB8, 0x00, 0x00, 0x00, 0x3E, 0x08, 0x38, 0x00, 0x00, 0x00}; // km/h
static const PROGMEM byte bitmapBigSpace_p[2 * 9] = {    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // " "

static const PROGMEM byte bitmapBrakeSymbol_p[10] = {0x3C, 0x66, 0xC3, 0x18, 0x3C, 0x3C, 0x18, 0xC3, 0x66, 0x3C}; //Symbol for showing that the bikes brake is active
static const PROGMEM byte bitmapBrakeSymbolClear_p[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //10x0 empty rectangle for clearing the brake symbol

static void drawSpeed(float speed, byte xpos, byte ypos) //print the speed in big 9x16 pixel characters, e.g. "27,6"
{
    int speed_digits = (int)(speed*10.0+0.5); //calculate the speed with 3 digits, with rounding, e.g. 20,29123 -> 203  -> display "20,3"
    lcd.setCursorInPixels(xpos,ypos);
    if(speed_digits<100) //<10kmh
    {
        lcd.drawBitmap(bitmapBigSpace_p, 9,2);
    }
    else
    {
        if(speed_digits<1000) lcd.drawBitmap(bitmapBigNumber[speed_digits/100], 9,2);  //max supported speed is 99,9kmh, should be enough!
    }
    lcd.setCursorInPixels(xpos+10,ypos);
    lcd.drawBitmap(bitmapBigNumber[(speed_digits/10)%10], 9,2);
    lcd.setCursorInPixels(xpos+19,ypos);
    lcd.drawBitmap(bitmapBigComma_p, 3,2);
    lcd.setCursorInPixels(xpos+23,ypos);
    lcd.drawBitmap(bitmapBigNumber[speed_digits%10], 9,2);
    lcd.setCursorInPixels(xpos+33,ypos);
    lcd.drawBitmap(bitmapBigkmh_p, 9,2);
}

static void printTime(unsigned long sec)  //print time in exactly 5 characters: in the format "mm:ss" or "hh:mm", if the time is >1 hour
{
    word hours = sec/3600UL;
    byte minutes = (sec/60UL) % 60UL; //numberOfMinutes(val);
    byte seconds = sec % 60UL; //numberOfSeconds(val);
    word first,second; //only 2 numbers are displayed: either hours:minutes or minutes:seconds

    if(hours>0)
    {first=hours; second=minutes;}
    else
    {first=minutes; second=seconds;}
    if(first<10) lcd.print(MY_F("0"));
    lcd.print(first);
    lcd.print(MY_F(":"));
    if(second<10) lcd.print(MY_F("0"));
    lcd.print(second);
}

static void printTripDistance(float km)  // print distance in exactly 6 characters, left aligned: "1234km" or "123km " or "12,1km" or "9,1km "
{
    if (km<10.0) {lcd.print(km,1); lcd.print(MY_F("km "));}
    else
    {
        if (km<100)
        {lcd.print(km,1); lcd.print(MY_F("km"));}
        else
        {
            if(km<1000.0)
            {
                lcd.print(km,0); lcd.print(MY_F("km "));
            }
            else {lcd.print(km,0); lcd.print(MY_F("km"));}
        }
    }
}

static void display_nokia_update_graphic()
{
    //print range in km in the top left corner
    lcd.setCursorInPixels(0,0);
    lcd.print(range,0); lcd.print(MY_F("km "));

    lcd.setCursorInPixels(42-10,0);
    if(brake_stat==0) //if brake is active

    {lcd.drawBitmap(bitmapBrakeSymbol_p, 10,1);}
    else  //clear the brake symbol
    {lcd.drawBitmap(bitmapBrakeSymbolClear_p, 10,1);}
    lcd.setCursor(7,5);
    lcd.print(current_profile+1);
#if HARDWARE_REV >=2
    lcd.setCursor(8,0);
    if (digitalRead(bluetooth_pin)==1)
    {lcd.write(2);}
    else
    {lcd.print(MY_F(" "));}
#endif

    //print the electrical power (Watt) in the top right corner
    lcd.setCursorInPixels(84-4*6,0);
    if (power<99.5) {lcd.print(MY_F(" "));}
    if (power<9.5)  {lcd.print(MY_F(" "));}
    lcd.print(power,0);
    lcd.print(MY_F("W"));

    drawSpeed(spd, 20, 2); //centered vertically

    //print the trip distance in the lower left corner
    lcd.setCursorInPixels(0,5);
    printTripDistance(km); //todo: implement real trip distance! currently the distance since last changing of the battery is displayed.

    //print a vertical bargraph on the right side:
    //- the full range of the bargraph equals to the max selectable power (power_max or power_poti_max)
    //- the limit line shows the target power that is selected by the user (power_set_for_display)
    //- the bar shows the currenlty measured electrical power that the moter consumes
    lcd.setCursorInPixels(72,1);
    lcd.drawVerticalBar((word)(max(curr_power_max,curr_power_poti_max)), (word)(max(power_set,0)), (word)(power), 11, 4);

    //print battery percent left
    lcd.setCursorInPixels(0,1);
    lcd.drawVerticalBar((word)(100), (word)(constrain((voltage_display-vcutoff)/(vmax-vcutoff)*100,0,100)), (word)(battery_percent_fromcapacity), 11, 4);

    //print the trip-time in the lower right corner (displayed in the last 5 characters in this line)
    lcd.setCursorInPixels(84-5*6,5);
    //todo: implement real trip time measurement, including timeouts. Currently it is only the time after the last reset
#ifdef SUPPORT_RTC
    if (now.hh<10)
      lcd.print(MY_F("0"));
    lcd.print(now.hh);
    lcd.print(":");
    if (now.mm<10)
      lcd.print(MY_F("0"));
    lcd.print(now.mm);
#else  
    printTime(millis() / 1000UL);  //millis can be used, because they roll over only after 50 days
#endif
}
#endif

void display_update()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA) || (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    if (handle_important_info_expire())
        return;
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    if (menu_active)
        nokia_screen=NOKIA_SCREEN_MENU;
    else if (spd>0 && !display_force_text)
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
        case NOKIA_SCREEN_MENU:
            display_menu();
            break;
        case NOKIA_SCREEN_GRAPHIC:
            display_nokia_update_graphic();
            break;
        case NOKIA_SCREEN_TEXT:
            display_nokia_update();
            break;
    }
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    if (menu_active)
        display_menu();
    else
        display_16x2_update();
#endif
#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
    jlcd_update(2,wheel_time,0,power);
#endif
#if (DISPLAY_TYPE & DISPLAY_TYPE_BMS)
    slcd_update(map(battery_percent_fromcapacity,0,100,0,16),wheel_time,0);
#endif

}
