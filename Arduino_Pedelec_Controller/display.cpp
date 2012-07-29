/*
Generic display init and update functions
Written by jenkie and Thomas Jarosch

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
#include "config.h"


#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
#include "PCD8544_nano.h"                    //for Nokia Display
static PCD8544 lcd;                          //for Nokia Display
#endif



#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
#include "LiquidCrystalDogm.h"             //for 4bit (e.g. EA-DOGM) Display
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);   //for 4bit (e.g. EA-DOGM) Display
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_J_LCD)
#include <SoftwareSerial.h>                //for Kingmeter J-LCD
SoftwareSerial mySerial(10, 11);           // RX (YELLOW cable of J-LCD), TX (GREEN-Cable) 
byte jlcd_received[]={0,0,0,0,0,0};
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
    lcd.print(battery_percent,0);
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
    if (handle_important_info_expire())
        return;

    lcd.setCursor(0,0);
    lcd.print(voltage_display,1);

    lcd.setCursor(6,0);
    if ((current_display<9.5)&&(current_display>0))
        {lcd.print(" ");}
    lcd.print(current_display,1);

    lcd.setCursor(10,0);
    if (battery_percent<99.5)
        {lcd.print(" ");}
    if (battery_percent<9.5)
        {lcd.print(" ");}
    lcd.print(battery_percent,0);

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

void jlcd_update(byte battery, unsigned int wheeltime, byte error)
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_J_LCD)
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
        mySerial.write((byte)0x00);
        mySerial.write(highByte(wheeltime));
        mySerial.write(lowByte(wheeltime));
        mySerial.write(0X7D);
        mySerial.write(error);
        mySerial.write(battery^(byte)0x00^highByte(wheeltime)^lowByte(wheeltime)^0X7D^error); //this is XOR-checksum
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
#endif //(DISPLAY_TYPE & DISPLAY_TYPE_J_LCD)

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

#if (DISPLAY_TYPE & DISPLAY_TYPE_J_LCD)
    mySerial.begin(9600);
#endif
}

void display_update()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
        display_nokia_update();
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
        display_4bit_update();
#endif
#if (DISPLAY_TYPE & DISPLAY_TYPE_J_LCD)
        jlcd_update(5,wheel_time,0);
#endif
}
