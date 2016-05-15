/*
Generic display init and update functions
Written by jenkie and Thomas Jarosch
Functions for the Nokia graphical screen mainly by m--k
King-Meter library and support written by Michael Fabry

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

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
#include "display_kingmeter.h"
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)
#include "display_bafang.h"
#endif


display_mode_type display_mode = DISPLAY_MODE_GRAPHIC; //startup screen
display_mode_type display_mode_last = DISPLAY_MODE_TEXT; //last screen type
boolean display_force_text = false;

display_view_type display_view = DISPLAY_VIEW_MAIN;
display_view_type display_view_last = display_view;

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
#include "PCD8544_nano.h"                    //for Nokia Display
static PCD8544 lcd;                          //for Nokia Display
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
#include "LiquidCrystalDogm.h"             //for 4bit (e.g. EA-DOGM) Display
static LiquidCrystal lcd(13, 12, 11, 10, 9, 8);   //for 4bit (e.g. EA-DOGM) Display
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_SERIAL)
#include "serial_lcd.h"
static SerialLCD lcd(serial_display_16x2_pin);                      //16x2 New Haven display connected via one serial pin
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)         // For King-Meter J-LCD, SW-LCD, KM5s-LCD, EBS-LCD2
KINGMETER_t KM;                                     // Context of the King-Meter object
#if HARDWARE_REV < 20
#include <SoftwareSerial.h>
static SoftwareSerial mySerial(10, 11);             // RX (YELLOW cable), TX (GREEN cable)
SoftwareSerial* displaySerial =& mySerial;
#else
HardwareSerial* displaySerial=&Serial2;
#endif
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)            // For Bafang BBS0x displays
BAFANG_t BF;                                     // Context of the Bafang object
#if HARDWARE_REV < 20
#include <SoftwareSerial.h>
static SoftwareSerial mySerial(10, 11);             // RX (YELLOW cable), TX (GREEN cable)
SoftwareSerial* displaySerial =& mySerial;
#else
HardwareSerial* displaySerial=&Serial2;
#endif
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_BMS)||(DISPLAY_TYPE & DISPLAY_TYPE_BMS3)
#if HARDWARE_REV < 20
#include <SoftwareSerial.h>                         // For BMS Battery S-LCD and S-LCD3
static SoftwareSerial mySerial(10, 11);             // RX , TX
SoftwareSerial* displaySerial=&mySerial;
#else
HardwareSerial* displaySerial=&Serial2;
#endif
#if (DISPLAY_TYPE & DISPLAY_TYPE_BMS)
byte slcd_received[]= {0,0,0,0,0,0};
#else
byte slcd_received[]= {0,0,0,0,0,0,0,0,0,0,0,0,0};
#endif
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
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA) || (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    display_show_important_info(FROM_FLASH(msg_welcome), 5);

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    lcd.setCursor(0, 5);
    const byte max_len = 12;
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    lcd.setCursor(0, 1);
    const byte max_len = 14;
#endif

    // Show total mileage (right aligned)
    unsigned long total_km = (odo * wheel_circumference/1000);
    byte number_len = calc_number_length(total_km);
    // Safety check
    if (number_len >= max_len)
        return;

    number_len = max_len - number_len;
    while(number_len > 1)
    {
        lcd.print(MY_F(" "));
        --number_len;
    }

    lcd.print(total_km);
    lcd.print(MY_F(" km"));
#ifdef SUPPORT_BATTERY_CHARGE_COUNTER    
    lcd.setCursor(0, 4);
    lcd.print(charge_count);
    lcd.print(MY_F(" bat cycl"));
#endif
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

void display_prev_view()
{
    if (display_view == DISPLAY_VIEW_MAIN)
        display_view = _DISPLAY_VIEW_END;

    byte prev_view = static_cast<byte>(display_view) - 1;
    display_view = static_cast<display_view_type>(prev_view);

#ifdef SUPPORT_DISPLAY_BACKLIGHT
    enable_backlight();
#endif

    display_update();
}

void display_next_view()
{
    // enums can't be incremented directly
    byte next_view = static_cast<byte>(display_view) + 1;
    display_view = static_cast<display_view_type>(next_view);

    if (display_view == _DISPLAY_VIEW_END)
        display_view = DISPLAY_VIEW_MAIN;

#ifdef SUPPORT_DISPLAY_BACKLIGHT
    enable_backlight();
#endif

    display_update();
}

#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_SERIAL)
void display_16x_serial_enable_backlight()
{
    lcd.setBacklight(8);
}

void display_16x_serial_disable_backlight()
{
    lcd.setBacklight(1);
}
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
// TODO: Add PROGMEM support
static const byte serial_break_symbol[8] = { 0x0,0xa,0x11,0x15,0x11,0xa,0x0,0x0 }; //Symbol for showing that the bikes brake is active
static const byte serial_batt_symbol[8] = { 0xe,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x0 };  //Symbol for the battery
static const byte serial_profile2_symbol[8] = { 0x1f,0x11,0x1d,0x19,0x17,0x11,0x1f,0x0 }; //Symbol for showing that the second profile is active
#endif

static void display_16x2_setup()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_LCD_4BIT)
    lcd.begin(16, 2);
#endif
#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2_SERIAL)
    lcd.init();
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    // Online editor for custom chars (5x8 pixels):
    // http://www.quinapalus.com/hd44780udg.html
    lcd.createChar(0x01, serial_break_symbol);
    lcd.createChar(0x02, serial_batt_symbol);
    lcd.createChar(0x03, serial_profile2_symbol);
#endif
}

static void display_nokia_setup()    //setup main view of nokia display
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    lcd.begin(84, 48);
    lcd.createChar(0, glyph1);
    lcd.createChar(1, glyph2);
    lcd.createChar(2, glyph3);
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
        display_mode_last = DISPLAY_MODE_IMPORTANT_INFO;
    }

    // No important info shown
    return false;
}

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA) || (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
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
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
static void display_16x2_view_main()
{
    /*
        // DEBUG code
        spd = 25;
        power = 198;
        battery_percent_fromcapacity = 21;
        km = 137.8;
    */
    lcd.setCursor(0,0);
    if (spd<10)
    {lcd.print(MY_F(" "));}
    lcd.print(round(spd),0);
    lcd.print(MY_F(" km/h  "));

    double power_display = power;
    if (power_display > 999)
        power_display = 999;
    else if (power_display < 0)
        power_display = 0;

    if (power_display<10)
    {lcd.print(MY_F(" "));}
    if (power_display<100)
    {lcd.print(MY_F(" "));}
    if (power_display < 0)
        lcd.print(MY_F("0"));
    else
        lcd.print(power_display,0);
    lcd.print(MY_F(" W "));

    // Break status
    if(brake_stat==0)
        lcd.write(0x01);
    else if (current_profile)           // second profile active?
        lcd.write(0x03);
    else
        lcd.print(MY_F(" "));

    lcd.setCursor(0,1);
    // Custom battery symbol
    lcd.write(0x02);
    lcd.print(battery_percent_fromcapacity);
    // Note: two extra spaces to clear chars when the capacity gets lower
    lcd.print(MY_F("%  "));

    lcd.setCursor(8,1);
    lcd.print(km,1);
    lcd.print(MY_F(" km"));
}

static void display_16x2_view_time()
{
    lcd.setCursor(0,0);
    lcd.print(MY_F("Time: "));
    printTime(millis() / 1000UL);  //millis can be used, because they roll over only after 50 days

#ifdef SUPPORT_RTC
    lcd.setCursor(0,1);
    lcd.print("Clock: ");

    if (now.hh<10)
        lcd.print(MY_F("0"));
    lcd.print(now.hh);
    lcd.print(MY_F(":"));
    if (now.mm<10)
        lcd.print(MY_F("0"));
    lcd.print(now.mm);
#endif
}

static void display_16x2_view_battery()
{
    lcd.setCursor(0,0);
    lcd.print(voltage_display,1);
    lcd.print(MY_F(" V - "));

    if ((current_display<9.5)&&(current_display>0))
    {lcd.print(MY_F(" "));}
    lcd.print(current_display,1);
    lcd.print(MY_F(" A "));

    lcd.setCursor(0,1);

    lcd.print(wh,0);
    lcd.print(MY_F(" wh"));
    lcd.print(MY_F("  "));

    if (km > 0.1)
        lcd.print(wh/km,1);
    else
        lcd.print(MY_F("---"));
    lcd.print(MY_F(" AVG "));
}

bool show_altitude = false;
static void display_16x2_view_environment()
{
#if defined(SUPPORT_BMP085) || defined(SUPPORT_DSPC01)
#ifndef(SUPPORT_TEMP_SENSOR)
    lcd.setCursor(0,0);
    lcd.print(MY_F("Temp: "));
    lcd.print((int)temperature);
    lcd.print(MY_F(" C"));
#endif
    lcd.setCursor(0,1);

    // switch between altitude and slope very five seconds
    byte current_second = (millis() / 1000) % 60UL;
    if (current_second % 5 == 0)
        show_altitude = !show_altitude;
    if (show_altitude)
    {
        lcd.print(MY_F("Altitude: "));
        lcd.print((int)altitude);
        lcd.print(MY_F("m"));
        lcd.print(MY_F(" "));
    }
    else
    {
        lcd.print(MY_F("Slope: "));
        lcd.print(slope,0);
        lcd.print(MY_F("%    "));
    }
#endif
#if defined(SUPPORT_TEMP_SENSOR)
    lcd.setCursor(0,0);
    lcd.print(MY_F("T1: "));
    lcd.print(sensors.getTempCByIndex(0),1);
    lcd.print(MY_F(" C "));
    lcd.print(MY_F("T2: "));
    lcd.print((int)sensors.getTempCByIndex(1),1);
    lcd.print(MY_F(" C "));
#endif
}

static void display_16x2_view_human()
{
    lcd.setCursor(0,0);
    lcd.print(MY_F("CAD: "));
    if (cad<100)
    {lcd.print(MY_F(" "));}
    if (cad<10)
    {lcd.print(MY_F(" "));}
    lcd.print(cad,10);

#ifdef SUPPORT_HRMI
    lcd.setCursor(0,1);
    lcd.print(MY_F("Pulse: "));
    lcd.print((byte) pulse_human);
    lcd.print(MY_F(" "));
#endif
}

static void display_16x2_update()
{
    // View changed?
    if (display_view_last != display_view)
    {
        lcd.clear();
        display_view_last = display_view;
    }

    switch (display_view)
    {
        case DISPLAY_VIEW_TIME:
            display_16x2_view_time();
            break;
        case DISPLAY_VIEW_BATTERY:
            display_16x2_view_battery();
            break;
#if defined(SUPPORT_BMP085) || defined(SUPPORT_DSPC01) || defined(SUPPORT_TEMP_SENSOR)
        case DISPLAY_VIEW_ENVIRONMENT:
            display_16x2_view_environment();
            break;
#endif
        case DISPLAY_VIEW_HUMAN:
            display_16x2_view_human();
            break;
        case DISPLAY_VIEW_MAIN:
        default:
            display_16x2_view_main();
            break;
    }
}
#endif

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

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
void kingmeter_update(void)
{
    /* Prepare Tx parameters */

    if(battery_percent_fromcapacity > 10)
    {
        KM.Tx.Battery = KM_BATTERY_NORMAL;
    }
    else
    {
        KM.Tx.Battery = KM_BATTERY_LOW;
    }

    if(wheel_time < KM_MAX_WHEELTIME)
    {
        // Adapt wheeltime to match displayed speedo value according config.h setting      
        KM.Tx.Wheeltime_ms = (uint16_t) (((float) wheel_time) * (((float) KM.Settings.WheelSize_mm) / (wheel_circumference * 1000)));
    }
    else
    {
        KM.Tx.Wheeltime_ms = KM_MAX_WHEELTIME;
    }

    KM.Tx.Error = KM_ERROR_NONE;

    KM.Tx.Current_x10 = (uint16_t) (current_display * 10);


    /* Receive Rx parameters/settings and send Tx parameters */
    KingMeter_Service(&KM);


    /* Apply Rx parameters */

    #ifdef SUPPORT_LIGHTS_SWITCH
    if(KM.Rx.Headlight == KM_HEADLIGHT_OFF)
    {
        digitalWrite(lights_pin, 0);
    }
    else // KM_HEADLIGHT_ON, KM_HEADLIGHT_LOW, KM_HEADLIGHT_HIGH
    {
        digitalWrite(lights_pin, 1);
    }
    #endif

    if(KM.Rx.PushAssist == KM_PUSHASSIST_ON)
    {
        #if (DISPLAY_TYPE == DISPLAY_TYPE_KINGMETER_901U)
        throttle_stat = map(KM.Rx.AssistLevel, 0, 255, 0,1023);
        #else
        throttle_stat = 200;
        #endif
    }
    else
    {
        throttle_stat = 0;
        poti_stat     = map(KM.Rx.AssistLevel, 0, 255, 0,1023);
    }


    /* Shutdown in case we received no message in the last 3s */

    if((millis() - KM.LastRx) > 3000)
    {
        poti_stat     = 0;
        throttle_stat = 0;
        #if HARDWARE_REV >=2
        save_shutdown();
        #endif
    }
}
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)
void bafang_update(void)
{
    Bafang_Service(&BF);
    /* Apply Rx parameters */

    #ifdef SUPPORT_LIGHTS_SWITCH
    if(BF.Rx.Headlight == KM_HEADLIGHT_OFF)
    {
        digitalWrite(lights_pin, 0);
    }
    else
    {
        digitalWrite(lights_pin, 1);
    }
    #endif

    if(BF.Rx.PushAssist == true)
    {
        throttle_stat = 200;
    }
    else
    {
        throttle_stat = 0;
        poti_stat     = map(BF.Rx.AssistLevel, 0, 9, 0,1023);
    }


    /* Shutdown in case we received no message in the last 3s */

    if((millis() - BF.LastRx) > 3000)
    {
        poti_stat     = 0;
        throttle_stat = 0;
        #if HARDWARE_REV >=2
        save_shutdown();
        #endif
    }
}
#endif


static void slcd_update(byte battery, unsigned int wheeltime, byte error)
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_BMS)
    if (displaySerial->available())
    {
        slcd_last_transmission=millis();
        slcd_receivecounter++;
        byte receivedbyte=displaySerial->read();
        if (receivedbyte==0xE) //end of transmission frame detected
        {slcd_receivecounter=6;}
        if(slcd_receivecounter>6) //transmission buffer overflow
        {slcd_receivecounter=0;}
        if (slcd_receivecounter == 6)            //start of new transmission frame detected
        {
            slcd_zerocounter=0;
            slcd_lighton=(slcd_received[1]&(byte)128)>>7;
#ifdef SUPPORT_LIGHTS_SWITCH
            digitalWrite(lights_pin, slcd_lighton);
#endif
            slcd_received[1]=slcd_received[1]&(byte)127;
            if (slcd_received[1]<6)                      //set the assist-level (via poti-stat)
                poti_stat=map(slcd_received[1],0,5,0,1023);
            if (slcd_received[1]==6)                    //16 means walk-mode
                throttle_stat=200;
            else
                throttle_stat=0;
            //-------------------------------------------Output to S-LCD start
            displaySerial->write((byte)0x41);
            displaySerial->write((byte)battery);
            displaySerial->write((byte)0xFF);
            displaySerial->write(highByte(wheeltime));
            displaySerial->write(lowByte(wheeltime));
            displaySerial->write(error);
            displaySerial->write((byte)battery^(byte)0xFF^highByte(wheeltime)^lowByte(wheeltime)^error); //this is XOR-checksum
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
        save_shutdown();
#endif
    }
#endif //(DISPLAY_TYPE & DISPLAY_TYPE_BMS)

}

static void slcd3_update(byte battery, unsigned int wheeltime, byte error, byte power, byte symbols)
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_BMS3)
    if (displaySerial->available())
    {
        slcd_last_transmission=millis();
        slcd_receivecounter++;
        byte receivedbyte=displaySerial->read();
        if (receivedbyte==0xE) //end of transmission frame detected
        {slcd_receivecounter=12;}
        if(slcd_receivecounter>12) //transmission buffer overflow
        {slcd_receivecounter=0;}
        if (slcd_receivecounter == 12)            //start of new transmission frame detected
        {
            slcd_zerocounter=0;
            slcd_lighton=(slcd_received[1]&(byte)128)>>7;
#ifdef SUPPORT_LIGHTS_SWITCH
            digitalWrite(lights_pin, slcd_lighton);
#endif
            slcd_received[1]=slcd_received[1]&(byte)127;
            if (slcd_received[1]<6)                      //set the assist-level (via poti-stat)
                poti_stat=map(slcd_received[1],0,5,0,1023);
            if (slcd_received[1]==6)                    //6 means walk-mode
                throttle_stat=200;
            else
                throttle_stat=0;
            //-------------------------------------------Output to S-LCD start
            displaySerial->write((byte)0x41);
            displaySerial->write((byte)battery);
            displaySerial->write((byte)0x24);
            displaySerial->write(highByte(wheeltime));
            displaySerial->write(lowByte(wheeltime));
            displaySerial->write(error);
            displaySerial->write((byte)battery^(byte)0x24^highByte(wheeltime)^lowByte(wheeltime)^error^power^symbols); //this is XOR-checksum
            displaySerial->write(symbols);
            displaySerial->write(power);
            displaySerial->write((byte)0x00);
            displaySerial->write((byte)0x00);
            displaySerial->write((byte)0x00);
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
        save_shutdown();
#endif
    }
#endif //(DISPLAY_TYPE & DISPLAY_TYPE_BMS3)

}


void display_init()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    display_nokia_setup();
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    display_16x2_setup();
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
    KingMeter_Init(&KM, displaySerial);
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)
    Bafang_Init(&BF, displaySerial);
#endif


#if ((DISPLAY_TYPE == DISPLAY_TYPE_BMS) || (DISPLAY_TYPE == DISPLAY_TYPE_BMS3))
    displaySerial->begin(9600);
#endif
}


#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)

///////////////////////// additional code for a graphical screen design on the Nokia Display by m--k  ///////////////////////////////////////////////
//all bitmaps are created using this great bitmap generator http://www.introtoarduino.com/utils/pcd8544.html
//big characters 0-9 for displaying speed in 9x16 pixel format.
//big characters for the numbers 0..9  in 9x15 pixel size, used for displaying the speed
static const byte bitmapBigNumber[10][2 * 9] PROGMEM =
{
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
static const PROGMEM byte bitmapHeartSymbol[5] = {0x0c, 0x1e, 0x3c, 0x1e, 0x0c}; //heart symbol

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
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
static void display_nokia_setup_main()    //setup main view of nokia display
{
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
}
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
static void display_nokia_view_main()
{
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
}
#endif// (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
static void display_nokia_view_graphic()
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
    lcd.print(MY_F(":"));
    if (now.mm<10)
        lcd.print(MY_F("0"));
    lcd.print(now.mm);
#else
    printTime(millis() / 1000UL);  //millis can be used, because they roll over only after 50 days
#endif
}
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
static void display_nokia_view_human()
{

    lcd.setCursor(3,2); //print cadence centered
    lcd.print(MY_F(" CAD "));
    lcd.print(cad);
    lcd.print(MY_F("  "));
#if defined(SUPPORT_XCELL_RT) //show left bar with human and battery wh
    lcd.setCursor(3,3);  //print human power centered
    lcd.print(MY_F("   W ")); lcd.print(power_human,0);
    lcd.print(MY_F("   "));
    //print motor wh in top left corner
    lcd.setCursorInPixels(0,0);
    lcd.print(wh,0); lcd.print(MY_F(" WhMotor"));
    //print human wh in bottom left corner
    lcd.setCursorInPixels(0,5);
    lcd.print(wh_human,0); lcd.print(MY_F(" WhHuman"));

    //print bar graph left side, showing human wh (solid) vs motor wh (clear)
    lcd.setCursorInPixels(0,1);
    lcd.drawVerticalBar((word)(wh+wh_human), (word)(wh_human), (word)(0), 11, 4); 
#endif

#if defined(SUPPORT_HRMI)
    lcd.setCursorInPixels(72,1);
    lcd.drawVerticalBar((word)(pulse_range), (word)(pulse_human-pulse_min), (word)(0), 11, 4); 
    lcd.setCursor(12,0);
    lcd.print(pulse_human);
    lcd.setCursorInPixels(75,5);
    lcd.drawBitmap(bitmapHeartSymbol, 5,1);
#endif
}
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
static void display_nokia_view_environment()
{
#if defined(SUPPORT_BMP085) || defined(SUPPORT_DSPC01)
//#ifndef(SUPPORT_TEMP_SENSOR)
    lcd.setCursor(0,0);
    lcd.print(MY_F("Temp: "));
    lcd.print((int)temperature);
    lcd.print(MY_F(" C"));
//#endif
    lcd.setCursor(0,2);
    // switch between altitude and slope very five seconds
    lcd.print(MY_F("Start: "));
    lcd.print((int)(altitude_start));
    lcd.print(MY_F("m")); 
    lcd.setCursor(0,3);    
    lcd.print(MY_F("Altitude: "));
    lcd.print((int)(altitude));
    lcd.print(MY_F("m"));
    lcd.print(MY_F(" "));
    lcd.setCursor(0,4);
    lcd.print(MY_F("Slope: "));
    lcd.print(slope,0);
    lcd.print(MY_F("%    "));
#endif
#if defined(SUPPORT_TEMP_SENSOR)
    lcd.setCursor(0,0);
    lcd.print(MY_F("T1: "));
    lcd.print(sensors.getTempCByIndex(0),1);
    lcd.print(MY_F(" C "));
    lcd.print(MY_F("T2: "));
    lcd.print((int)sensors.getTempCByIndex(1),1);
    lcd.print(MY_F(" C "));
#endif
}
#endif

static void display_nokia_update()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
      // View changed?
    if (display_view_last != display_view)
    {
        lcd.clear();
        if (display_view==DISPLAY_VIEW_MAIN)
          display_nokia_setup_main();
        display_view_last = display_view;
    }

    switch (display_view)
    {
#if (defined(SUPPORT_BMP085) || defined(SUPPORT_DSPC01) || defined(SUPPORT_TEMP_SENSOR))&& defined(DV_ENVIRONMENT)
        case DISPLAY_VIEW_ENVIRONMENT:
            display_nokia_view_environment();
            break;
#endif
#if defined(DV_GRAPHIC)
        case DISPLAY_VIEW_GRAPHIC:
            display_nokia_view_graphic();
            break;
#endif
#if defined(DV_HUMAN)
        case DISPLAY_VIEW_HUMAN:
            display_nokia_view_human();
            break;
#endif
        case DISPLAY_VIEW_MAIN:
        default:
            display_nokia_view_main();
            break;
    }
#endif
}

void display_update()
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA) || (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    if (handle_important_info_expire())
        return;

    if (menu_active)
        display_mode=DISPLAY_MODE_MENU;
    //else if (spd>0 && !display_force_text)
        //display_mode=DISPLAY_MODE_GRAPHIC;
    else
        display_mode=DISPLAY_MODE_TEXT;

    if (display_mode!=display_mode_last)
    {
        lcd.clear();
        display_mode_last=display_mode;
        display_view_last=_DISPLAY_VIEW_END;
    }

    switch(display_mode)
    {
        case DISPLAY_MODE_MENU:
            display_menu();
            break;
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
        case DISPLAY_MODE_TEXT:
            display_nokia_update();
            break;
#elif (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
        case DISPLAY_MODE_GRAPHIC:
        case DISPLAY_MODE_TEXT:
            display_16x2_update();
            break;
#endif
    }
#endif // (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA) || (DISPLAY_TYPE & DISPLAY_TYPE_16X2)


#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
    kingmeter_update();
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)
    bafang_update();
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_BMS)
    slcd_update(map(battery_percent_fromcapacity,0,100,0,16),wheel_time,0);
#endif


#if (DISPLAY_TYPE & DISPLAY_TYPE_BMS3)
    slcd3_update(map(battery_percent_fromcapacity,0,100,0,16),wheel_time, 0, max(power/9.75,0), (byte)0x20*(!brake_stat));
#endif
}




void display_debug(HardwareSerial* localSerial)
{
#if (SERIAL_MODE & SERIAL_MODE_DISPLAYDEBUG)

    #if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    // ToDo: Print Rx data of Nokia diplay here
    #endif


    #if (DISPLAY_TYPE & DISPLAY_TYPE_16X2)
    // ToDo: Print Rx data of 16x2 diplay here
    #endif

    
    #if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
    localSerial->print(MY_F("PASdir "));
    localSerial->print(KM.Settings.PAS_RUN_Direction, HEX);
    localSerial->print(MY_F("  PAStol "));
    localSerial->print(KM.Settings.PAS_SCN_Tolerance);
    localSerial->print(MY_F("  PASrat "));
    localSerial->print(KM.Settings.PAS_N_Ratio);
    localSerial->print(MY_F("  HND_HL "));
    localSerial->print(KM.Settings.HND_HL_ThrParam, HEX);
    localSerial->print(MY_F("  HND_HF "));
    localSerial->print(KM.Settings.HND_HF_ThrParam, HEX);
    localSerial->print(MY_F("  SlowSt "));
    localSerial->print(KM.Settings.SYS_SSP_SlowStart);
    localSerial->print(MY_F("  SpdMags "));
    localSerial->print(KM.Settings.SPS_SpdMagnets);
    localSerial->print(MY_F("  UnderVol "));
    localSerial->print(((float)KM.Settings.VOL_1_UnderVolt_x10)/10);
    localSerial->print(MY_F("  WhSize "));
    localSerial->print(KM.Settings.WheelSize_mm);
    localSerial->print(MY_F("  |  Assist "));
    localSerial->print(KM.Rx.AssistLevel);
    localSerial->print(MY_F("  Light "));
    localSerial->print(KM.Rx.Headlight, HEX);
    localSerial->print(MY_F("  Batt "));
    localSerial->print(KM.Rx.Battery, HEX);
    localSerial->print(MY_F("  Push "));
    localSerial->print(KM.Rx.PushAssist, HEX);
    localSerial->print(MY_F("  PwrAss "));
    localSerial->print(KM.Rx.PowerAssist, HEX);
    localSerial->print(MY_F("  Throttle "));
    localSerial->print(KM.Rx.Throttle, HEX);
    localSerial->print(MY_F("  Cruise "));
    localSerial->print(KM.Rx.CruiseControl, HEX);
    localSerial->print(MY_F("  OverSpd "));
    localSerial->print(KM.Rx.OverSpeed, HEX);
    localSerial->print(MY_F("  SpdLim "));
    localSerial->print(((float)KM.Rx.SPEEDMAX_Limit_x10)/10);
    localSerial->print(MY_F("  CurrLim "));
    localSerial->println(((float)KM.Rx.CUR_Limit_x10)/10);
    #endif
    
    #if (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)
    localSerial->print(MY_F("  Assist "));
    localSerial->print(BF.Rx.AssistLevel);
    localSerial->print(MY_F("  Light "));
    localSerial->print(BF.Rx.Headlight, HEX);
    localSerial->print(MY_F("  Push "));
    localSerial->println(BF.Rx.PushAssist, HEX);
    #endif

    
    #if (DISPLAY_TYPE & DISPLAY_TYPE_BMS)
    // ToDo: Print Rx data of BMS diplay here
    #endif
    
    
    #if (DISPLAY_TYPE & DISPLAY_TYPE_BMS3)
    // ToDo: Print Rx data of BMS3 diplay here
    #endif

#endif
}
