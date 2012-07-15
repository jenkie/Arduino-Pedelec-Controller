//place all your personal configurations here and keep this file when updating!   
#ifndef CONFIG_H
#define CONFIG_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#define DISPLAY_TYPE_NOKIA_5PIN (1<<0)       //Nokia 5110 5 pin mode
#define DISPLAY_TYPE_NOKIA_4PIN (1<<1)       //Nokia 5110 4 pin mode (SCE pin tied to GND)
#define DISPLAY_TYPE_NOKIA (DISPLAY_TYPE_NOKIA_5PIN|DISPLAY_TYPE_NOKIA_4PIN)
#define DISPLAY_TYPE_16X2_LCD_4BIT (1<<2)    //16x2 LCD 4bit-mode
#define DISPLAY_TYPE DISPLAY_TYPE_NOKIA_5PIN //Set your display type here

#define HARDWARE_REV 3      //place your hardware revision (1-3) here: x means hardware-revision 1.x
#define SUPPORT_BMP085      //uncomment if BMP085 available

#define CONTROL_MODE_NORMAL 0            //Normal mode: poti and throttle control motor power
#define CONTROL_MODE_LIMIT_WH_PER_KM 1   //Limit wh/km consumption: poti controls wh/km, throttle controls power to override poti
#define CONTROL_MODE CONTROL_MODE_NORMAL //Set your control mode here

//Config Options-----------------------------------------------------------------------------------------------------
const int pas_tolerance=1;               //0... increase to make pas sensor slower but more tolerant against speed changes
const int throttle_offset=50;            //Offset for throttle output where Motor starts to spin (0..255 = 0..5V)
const int throttle_max=200;              //Maximum input value for motor driver (0..255 = 0..5V)
const boolean startingaidenable = true;  //enable starting aid?
const float vcutoff=33.0;                //cutoff voltage in V;
const float vemergency_shutdown = 28.0;  //emergency power off situation to save the battery from undervoltage
const float wheel_circumference = 2.202; //wheel circumference in m
const int spd_max1=23.0;                 //speed cutoff start in Km/h
const int spd_max2=25.0;                 //speed cutoff stop (0W) in Km/h
const int power_max=500;                 //Maximum power in W
const int whkm_max=30;                   //Maximum wh/km consumption in CONTROL_MODE_LIMIT_WH_PER_KM (controls poti-range)
const unsigned int idle_shutdown_secs = 30 * 60;           // Idle shutdown in seconds. Max is ~1080 minutes or 18 hours
const double capacity = 166.0;           //battery capacity in watthours for range calculation
const double pas_factor_min=1.2;         //Use pas_factor from hardware-test here with some tolerances. Both values have to be eihter larger or smaller than 1.0 and not 0.0!
const double pas_factor_max=3;           //Use pas_factor from hardware-test here with some tolerances. Both values have to be eihter larger or smaller than 1.0 and not 0.0!
double pid_p=0.0;                        //pid p-value, default: 0.0
double pid_i=2.0;                        //pid i-value, default: 2.0
double pid_p_throttle=0.05;              //pid p-value for throttle mode
double pid_i_throttle=2.5;               //pid i-value for throttle mode

#endif
