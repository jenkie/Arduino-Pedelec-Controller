//place all your personal configurations here and keep this file when updating!   
#ifndef CONFIG_H
#define CONFIG_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#define DISPLAY_TYPE 0      //display type 0:Nokia5110 5-pin-mode 1: Nokia5110 4-pin-mode (SCE pin tied to GND) 2: 16x2 LCD 4bit-mode
#define HARDWARE_REV 3      //place your hardware revision (1-3) here: x means hardware-revision 1.x
#define SUPPORT_BMP085      //uncomment if BMP085 available


//Config Options-----------------------------------------------------------------------------------------------------
const int pas_tolerance=1;               //0... increase to make pas sensor slower but more tolerant against speed changes
const int throttle_offset=50;            //Offset for throttle output where Motor starts to spin (0..255 = 0..5V)
const int throttle_max=200;              //Maximum input value for motor driver (0..255 = 0..5V)
const boolean startingaidenable = true;  //enable starting aid?
const float vcutoff=33.0;                //cutoff voltage in V;
const float wheel_circumference = 2.202; //wheel circumference in m
const int spd_max1=27.0;                 //speed cutoff start in Km/h
const int spd_max2=30.0;                 //speed cutoff stop (0W) in Km/h
const int power_max=500;                 //Maximum power in W
const unsigned int idle_shutdown_secs = 30 * 60;           // Idle shutdown in seconds. Max is ~1080 minutes or 18 hours
const double capacity = 166.0;           //battery capacity in watthours for range calculation
double pid_p=0.0;                        //pid p-value, default: 0.0
double pid_i=2.0;                        //pid i-value, default: 2.0
double pid_p_throttle=0.05;              //pid p-value for throttle mode
double pid_i_throttle=2.5;               //pid i-value for throttle mode

#endif
