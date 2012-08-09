//place all your personal configurations here and keep this file when updating!   
#ifndef CONFIG_H
#define CONFIG_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#define HARDWARE_REV 3      //place your hardware revision (1-3) here: x means hardware-revision 1.x

#define DISPLAY_TYPE_NOKIA_5PIN (1<<0)       //Nokia 5110 5 pin mode
#define DISPLAY_TYPE_NOKIA_4PIN (1<<1)       //Nokia 5110 4 pin mode (SCE pin tied to GND)
#define DISPLAY_TYPE_NOKIA (DISPLAY_TYPE_NOKIA_5PIN|DISPLAY_TYPE_NOKIA_4PIN)
#define DISPLAY_TYPE_16X2_LCD_4BIT (1<<2)    //16x2 LCD 4bit-mode
#define DISPLAY_TYPE_J_LCD (1<<3)            //King-Meter J-LCD
#define DISPLAY_TYPE DISPLAY_TYPE_NOKIA_5PIN //Set your display type here. CHANGES ONLY HERE!<-----------------------------

#define SERIAL_MODE_DEBUG (1<<0)             //send debug data over Serial Monitor
#define SERIAL_MODE_ANDROID (1<<1)           //send Amarino-compatible data over serial/bluetooth
#define SERIAL_MODE_MMC (1<<2)               //send MMC-App compatible data over serial/bluetooth (for future use, not implemented yet)
#define SERIAL_MODE_LOGVIEW (1<<3)          //send logview-compatible data over serial (for future use, not implemented yet)
#define SERIAL_MODE SERIAL_MODE_DEBUG        //Set your serial mode here. CHANGES ONLY HERE!<-----------------------------


// #define SUPPORT_DISPLAY_BACKLIGHT // uncomment for LCD display backlight support
                                     // The Nokia 5110 display needs a 120 Ohm resistor on the backlight pin
#ifdef SUPPORT_DISPLAY_BACKLIGHT
const int display_backlight_pin = 8;   // LCD backlight. Use a free pin here, f.e. instead of display switch #2.
#endif

// #define SUPPORT_ANDROID  //uncomment if data should be sent to an Android
// #define SUPPORT_BMP085   //uncomment if BMP085 available
#define SUPPORT_POTI        //uncomment if Poti connected
// #define SUPPORT_SOFT_POTI // uncomment if Poti is emulated: The switch_disp button will store the current throttle value as poti value
#define SUPPORT_THROTTLE    //uncomment if Throttle connected
#define SUPPORT_PAS         //uncomment if PAS-sensor connected


#define CONTROL_MODE_NORMAL 0            //Normal mode: poti and throttle control motor power
#define CONTROL_MODE_LIMIT_WH_PER_KM 1   //Limit wh/km consumption: poti controls wh/km, throttle controls power to override poti
#define CONTROL_MODE CONTROL_MODE_NORMAL //Set your control mode here

//Config Options-----------------------------------------------------------------------------------------------------
const int pas_tolerance=1;               //0... increase to make pas sensor slower but more tolerant against speed changes
const int throttle_offset=196;           //Offset voltage of throttle control when in "0" position (0..1023 = 0..5V)
const int throttle_max=832;              //Offset voltage of throttle control when in "MAX" position (0..1023 = 0..5V)
const int motor_offset=50;               //Offset for throttle output where Motor starts to spin (0..255 = 0..5V)
const int motor_max=200;                 //Maximum input value for motor driver (0..255 = 0..5V)
const boolean startingaidenable = true;  //enable starting aid?
const int startingaid_speed = 6;         //starting aid up to this speed. 6km/h is the limit for legal operation of a Pedelec by EU-wide laws
const float vcutoff=33.0;                //cutoff voltage in V;
const float vemergency_shutdown = 28.0;  //emergency power off situation to save the battery from undervoltage
const float wheel_circumference = 2.202; //wheel circumference in m
<<<<<<< HEAD
const int spd_max1=23;                   //speed cutoff start in Km/h
=======
const int spd_max1=22;                   //speed cutoff start in Km/h
>>>>>>> 83be1082b2eb88c2265a9de702ff5e261f8f3944
const int spd_max2=25;                   //speed cutoff stop (0W) in Km/h
const int power_max=500;                 //Maximum power in W set by throttle, giving us quick power with a tip of a finger
const int power_poti_max=250;		 //Maximum power set by the poti, giving us a continuous support
const int whkm_max=30;                   //Maximum wh/km consumption in CONTROL_MODE_LIMIT_WH_PER_KM (controls poti-range)
const unsigned int idle_shutdown_secs = 30 * 60;           // Idle shutdown in seconds. Max is ~1080 minutes or 18 hours
const double capacity = 166.0;           //battery capacity in watthours for range calculation
const double pas_factor_min=1.2;         //Use pas_factor from hardware-test here with some tolerances. Both values have to be eihter larger or smaller than 1.0 and not 0.0!
const double pas_factor_max=3;           //Use pas_factor from hardware-test here with some tolerances. Both values have to be eihter larger or smaller than 1.0 and not 0.0!
const double cfg_pid_p=0.0;              //pid p-value, default: 0.0
const double cfg_pid_i=2.0;              //pid i-value, default: 2.0
const double cfg_pid_p_throttle=0.05;    //pid p-value for throttle mode
const double cfg_pid_i_throttle=2.5;     //pid i-value for throttle mode

// voltage and current calibration
const float voltage_amplitude = 0.0587;       // set this value according to your own voltage-calibration. Default: 0.0587
const float voltage_offset = 0.0;             // set this value according to your own voltage-calibration. Default: 0.0
const float current_offset = 0.0;             // for Rev 1.3 ONLY! set this value according to your own current-calibration. Default: 0.0
                                              // for Rev 1.1 - 1.2 the offset is corrected by software!
const float current_amplitude_R11 = 0.0296;   // for Rev 1.1 - 1.2 set this value according to your own current-calibration. Default: 0.0296
const float current_amplitude_R13 = 0.0741;   // for Rev 1.3 set this value according to your own current-calibration. Default: 0.0741

const char* const msg_welcome = "Welcome";
const char* const msg_shutdown = "Live long and prosper.";

#endif
