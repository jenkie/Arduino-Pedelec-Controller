//place all your personal configurations here and keep this file when updating!
#ifndef CONFIG_H
#define CONFIG_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif
#include "switches_action.h"

#define HARDWARE_REV 22      //place your hardware revision here: 1-5 means hardware-revision 1.x, 2x means 2.x

#define DISPLAY_TYPE_NONE           (1<<0)                  // No display at all
#define DISPLAY_TYPE_NOKIA_5PIN     (1<<1)                  // Nokia 5110 5-pin mode
#define DISPLAY_TYPE_NOKIA_4PIN     (1<<2)                  // Nokia 5110 4-pin mode (SCE pin tied to GND)
#define DISPLAY_TYPE_NOKIA          (DISPLAY_TYPE_NOKIA_5PIN|DISPLAY_TYPE_NOKIA_4PIN)
#define DISPLAY_TYPE_16X2_LCD_4BIT  (1<<3)                  // 16x2 LCD 4-bit mode
#define DISPLAY_TYPE_16X2_SERIAL    (1<<6)                  // 16x2 LCD via serial connection (New Haven display)
#define DISPLAY_TYPE_16X2           (DISPLAY_TYPE_16X2_LCD_4BIT|DISPLAY_TYPE_16X2_SERIAL)
#define DISPLAY_TYPE_BMS            (1<<5)                  // BMS Battery S-LCD
#define DISPLAY_TYPE_BMS3           (1<<7)                  // BMS Battery S-LCD3
#define DISPLAY_TYPE_KINGMETER_618U (1<<4)                  // King-Meter 618U protocol (KM5s, EBS-LCD2, J-LCD, SW-LCD)
#define DISPLAY_TYPE_KINGMETER_901U (1<<8)                  // King-Meter 901U protocol (KM5s)
#define DISPLAY_TYPE_KINGMETER      (DISPLAY_TYPE_KINGMETER_618U|DISPLAY_TYPE_KINGMETER_901U)
#define DISPLAY_TYPE_BAFANG_C961    (1<<9)
#define DISPLAY_TYPE_BAFANG_C965    (1<<10)
#define DISPLAY_TYPE_BAFANG         (DISPLAY_TYPE_BAFANG_C961|DISPLAY_TYPE_BAFANG_C965)

#define DISPLAY_TYPE DISPLAY_TYPE_NOKIA_4PIN                // Set your display type here. CHANGES ONLY HERE!<-----------------------------


//Selection of available display views: comment out any view that you do not want. Can save much programming space!
#define DV_GRAPHIC
#define DV_TIME
#define DV_BATTERY
#define DV_ENVIRONMENT
#define DV_HUMAN
#define DV_ODOMETER

// If using a New Haven serial 16x2 display: The pin the display is connected to
#if HARDWARE_REV < 20
const int serial_display_16x2_pin = 10;
const int serial_display_16x2_second_unused_pin = 11;       // SoftSerial always needs an additional pin for RX
#else
const int serial_display_16x2_pin = 17;
const int serial_display_16x2_second_unused_pin = 16;       // SoftSerial always needs an additional pin for RX
#endif

#define NOKIA_LCD_CONTRAST 190                              // Set display contrast here. Values around 190 should do the job

#define SERIAL_MODE_NONE            (1<<0)                  // Don't send serial data at all
#define SERIAL_MODE_DEBUG           (1<<1)                  // Send debug data over Serial Monitor
#define SERIAL_MODE_ANDROID         (1<<2)                  // Send Arduino Pedelec HMI compatible data over serial
#define SERIAL_MODE_MMC             (1<<3)                  // Send MMC-App compatible data over serial
#define SERIAL_MODE_LOGVIEW         (1<<4)                  // Send logview-compatible data over serial 
#define SERIAL_MODE_IOS             (1<<5)                  // Send IOS-compatible data over serial
#define SERIAL_MODE_DISPLAYDEBUG    (1<<6)                  // Send display-debug data over serial

#define SERIAL_MODE SERIAL_MODE_DEBUG                       //Set your serial mode here. CHANGES ONLY HERE!<-----------------------------

//Since hardware revision 2.0 the bluetooth port uses a separate serial interface, select data here:
#define BLUETOOTH_MODE_NONE         (1<<0)                  // Don't send bluetooth data at all
#define BLUETOOTH_MODE_DEBUG        (1<<1)                  // Send debug data over bluetooth
#define BLUETOOTH_MODE_ANDROID      (1<<2)                  // Send Arduino Pedelec HMI compatible data over bluetooth
#define BLUETOOTH_MODE_MMC          (1<<3)                  // Send MMC-App compatible data over bluetooth
#define BLUETOOTH_MODE_LOGVIEW      (1<<4)                  // Send logview-compatible data over bluetooth 
#define BLUETOOTH_MODE_IOS          (1<<5)                  // Send IOS-compatible data over bluetooth
#define BLUETOOTH_MODE_DISPLAYDEBUG (1<<6)                  // Send display-debug data over bluetooth

#define BLUETOOTH_MODE BLUETOOTH_MODE_NONE                  // Set your bluetooth mode here. CHANGES ONLY HERE!<-----------------------------


// #define SUPPORT_BLUETOOTH_ENABLE_ON_STARTUP              // Uncomment to enable bluetooth during startup

// Customizable buttons for use with the on-the-go-menu.
// The menu is activated by the ACTION_ENTER_MENU action (see below).
//
// Choose from: SWITCH_THROTTLE, SWITCH_DISPLAY1 and SWITCH_DISPLAY2
//
const switch_name MENU_BUTTON_UP = SWITCH_THROTTLE;
const switch_name MENU_BUTTON_DOWN = SWITCH_DISPLAY1;

// Switch actions: Customizable actions for short and long press
//
// Choose from: ACTION_NONE, ACTION_SET_SOFT_POTI, ACTION_SHUTDOWN_SYSTEM
//              ACTION_ENABLE_BACKLIGHT_LONG, ACTION_TOGGLE_BLUETOOTH,
//              ACTION_ENTER_MENU, ACTION_PROFILE_1, ACTION_PROFILE_2, ACTION_PROFILE
//              ACTION_TOGGLE_LIGHTS, ACTION_INCREASE_POTI, ACTION_DECREASE_POTI
//              ACTION_FIXED_THROTTLE_VALUE
//
//              16x2 serial display only (at the moment):
//              ACTION_DISPLAY_PREV_VIEW, ACTION_DISPLAY_NEXT_VIEW
//
//              "Double speed" motor gear shift:
//              ACTION_GEAR_SHIFT_LOW, ACTION_GEAR_SHIFT_HIGH, ACTION_GEAR_SHIFT_AUTO
//              ACTION_GEAR_SHIFT_TOGGLE_LOW_HIGH, ACTION_GEAR_SHIFT_TOGGLE_LOW_HIGH_AUTO
//
// The file "switches_action.h" contains a list with descriptions.
//
const sw_action SW_THROTTLE_SHORT_PRESS = ACTION_SET_SOFT_POTI;
const sw_action SW_THROTTLE_LONG_PRESS  = ACTION_SHUTDOWN_SYSTEM;

// #define SUPPORT_SWITCH_ON_POTI_PIN              //uncomment if you have an additional switch on the poti pin.
                                                   //FC 1.x: Use external 10k pullup resistor or it will not work!

const sw_action SW_POTI_SHORT_PRESS = ACTION_NONE;
const sw_action SW_POTI_LONG_PRESS = ACTION_NONE;

const sw_action SW_DISPLAY1_SHORT_PRESS = ACTION_DISPLAY_NEXT_VIEW;
const sw_action SW_DISPLAY1_LONG_PRESS  = ACTION_ENTER_MENU;

const sw_action SW_DISPLAY2_SHORT_PRESS = ACTION_DISPLAY_NEXT_VIEW;
const sw_action SW_DISPLAY2_LONG_PRESS  = ACTION_ENTER_MENU;

// #define SUPPORT_DISPLAY_BACKLIGHT // uncomment for dynamic LCD display backlight support.
                                     // Normally the backlight is always on. If you enable this option,
                                     // it will only be on when something important is shown.

                                     // The Nokia 5110 display needs a 120 Ohm resistor on the backlight pin
#ifdef SUPPORT_DISPLAY_BACKLIGHT
const int display_backlight_pin = 12;   // LCD backlight. Use a free pin here, f.e. instead of display switch #2 (12, default).
#endif

// #define SUPPORT_BMP085   //uncomment if BMP085 available (barometric pressure + temperature sensor)
// #define SUPPORT_DSPC01   //uncomment if DSPC01 available (barometric altitude + temperature sensor), connect to I2C-BUS
// #define SUPPORT_RTC     //uncomment if DS1307 RTC module connected to I2C
#define SUPPORT_POTI        //uncomment if Poti connected
// #define SUPPORT_SOFT_POTI //uncomment if Poti is emulated: The switch_disp button will store the current throttle value as poti value
// #define SUPPORT_THROTTLE_AUTO_CRUISE //uncomment to automatically set poti value when throttle is held constant for a while


// #define SUPPORT_POTI_SWITCHES     //uncomment to increase/decrease the poti via switch action ACTION_INCREASE_POTI / ACTION_DECREASE_POTI
const int poti_value_on_startup_in_watts = 0;    //poti startup value in watts
const int poti_level_step_size_in_watts = 50;    //number of watts to increase / decrease poti value by switch press
const int poti_fixed_value_via_switch = 250;     //fixed number of watts for the soft poti when ACTION_SET_FIXED_POTI_VALUE is triggered
const int fixed_throttle_in_watts = 250;         //number of watts to set as throttle value if ACTION_FIXED_THROTTLE_VALUE is hold down (=starting aid via switch)

#define SUPPORT_THROTTLE    //uncomment if Throttle connected
#define SUPPORT_PAS         //uncomment if PAS-sensor connected
// #define SUPPORT_BBS         //uncomment if BBS02 PAS sensor is connected (2 wires analog to Thun)
#define BBS_GEARCHANGEPAUSE 2000 //powerless time in milliseconds to allow gear change 
// #define SUPPORT_XCELL_RT    //uncomment if X-CELL RT connected. FC1.4: pas_factor_min=0.2, pas_factor_max=0.5. FC1.5: pas_factor_min=0.5, pas_factor_max=1.5. pas_magnets=8
// #define SUPPORT_TORQUE_THROTTLE
// #define SUPPORT_HRMI         //uncomment if polar heart-rate monitor interface connected to i2c port
#define SUPPORT_BRAKE        //uncomment if brake switch connected
// #define INVERT_BRAKE         //uncomment if brake signal is low when not braking
// #define RESET_PID_ON_BRAKE   //uncomment to reset the pid regulator if you are braking or stop pedaling. If this config option is not active, the pid regulator will slowly ramp down instead, which gives you a faster kick-in of the motor if you release the brake or start pedaling again
#define DETECT_BROKEN_SPEEDSENSOR //detect broken speed sensor and disable motor (useful for legalisation)


#define SUPPORT_PROFILE_SWITCH_MENU           //uncomment to disable support for profile switching in the menu
#define SUPPORT_FIRST_AID_MENU                //uncomment if you want a on-the-go workaround menu ("Pannenhilfe")

// Software controlled lights switch. Needs FC 1.3 or newer. Only possible if X-CELL RT is not in use as they share the A3 pin
// #define SUPPORT_LIGHTS_SWITCH                   //uncomment if you want software switchable lights on pin A3 (lights_pin)
// #define SUPPORT_LIGHTS_ENABLE_ON_STARTUP   //uncomment if you want the lights turned on when the system turns on
// #define SUPPORT_LIGHTS_SWITCH_MENU       //uncomment if you want a "Toggle lights" menu entry

#define CONTROL_MODE_NORMAL 0            //Normal mode: poti and throttle control motor power
#define CONTROL_MODE_LIMIT_WH_PER_KM 1   //Limit wh/km consumption: poti controls wh/km, throttle controls power to override poti
#define CONTROL_MODE_TORQUE 2            //power = x*power of the biker, see also description of power_poti_max!
#define CONTROL_MODE CONTROL_MODE_NORMAL //Set your control mode here

//#define SUPPORT_MOTOR_SERVO      //RC Motor controller with PWM input is used. Do not forget to remove capacitor from the low pass filter of the output to the motor controller.
//#define SUPPORT_MOTOR_GUESS   //enable guess of motor drive depending on current speed. Usefull for motor controllers with speed-throttle to optimize response behaviour
#define SUPPORT_BATTERY_CHARGE_DETECTION //support detection if the battery was charged -> reset wh / trip km / mah counters if detected.
const byte battery_charged_min_voltage = 20;  //minimum battery voltage to consider it charged. Useful to prevent "false positives".
//#define SUPPORT_BATTERY_CHARGE_COUNTER //support charge counter for battery cycles. is increased every time a regarge is detected.

//#define SUPPORT_GEAR_SHIFT                 //support shifting gears on the "double speed" motor
const byte gear_shift_pin_low_gear = 5;      //pin that connect to the low gear signal ("red" cable)
const byte gear_shift_pin_high_gear = 7;     //pin that connects to the high gear signal ("green" cable)

//#define SUPPORT_TEMP_SENSOR                //uncomment if you want to use a DS18x20 temperature sensor
const byte temp_pin = A2;                     //pin connected to Data pin of the DS18x20 temperature Sensor

//#define SUPPORT_THERMISTOR                 //uncomment if you have thermistor connected
const byte thermistor_pin = A2;              //thermistor pin
const float thermistor_t0=0.00335401643;     // 1/T0 of thermistor in 1/K
const float thermistor_b=0.00025316455;      // 1/beta of thermistor in 1/K
const float thermistor_r=10;                 // r of thermistor in kOhm

//#define SUPPORT_HX711                        //uncomment this if you want to use a load cell with hx711 amplifier
const byte hx711_data=20;                    //data pin of hx711 sensor
const byte hx711_sck=21;                     //clock pin of hx711 sensor
const double hx711_scale=78514.375;         //this is the scale to apply. 

//Config Options-----------------------------------------------------------------------------------------------------
const int pas_tolerance=1;               //0... increase to make pas sensor slower but more tolerant against speed changes
const int throttle_offset=196;           //Offset voltage of throttle control when in "0" position (0..1023 = 0..5V)
const int throttle_max=832;              //Offset voltage of throttle control when in "MAX" position (0..1023 = 0..5V)
const int poti_offset=0;                 //Offset voltage of poti when in "0" position (0..1023 = 0..5V)
const int poti_max=1023;                 //Offset voltage of poti when in "MAX" position (0..1023 = 0..5V)
const int motor_offset=50;               //Offset for throttle output where Motor starts to spin (0..255 = 0..5V). Default: 50. In Servo mode this value is about 1000
const int motor_max=200;                 //Maximum input value for motor driver (0..255 = 0..5V). Default: 200. In Servo mode this value is about 2000
const int spd_idle=55;                   //idle speed of motor in km/h - may be much higher than real idle speed (depending on controller)
const boolean startingaidenable = true;  //enable starting aid?
const int startingaid_speed = 6;         //starting aid up to this speed. 6km/h is the limit for legal operation of a Pedelec by EU-wide laws
const float vmax=42.0;                   //Battery voltage when fully charged
const float vcutoff=33.0;                //cutoff voltage in V;
const float vemergency_shutdown = 28.0;  //emergency power off situation to save the battery from undervoltage
const float wheel_circumference = 2.202; //wheel circumference in m
const byte wheel_magnets=1;              //configure your number of wheel magnets here
const int spd_max1=22;                   //speed cutoff start in Km/h
const int spd_max2=25;                   //speed cutoff stop (0W) in Km/h
const int power_max=500;                 //Maximum power in W (throttle mode)
const int power_poti_max=500;            //Maximum power in W (poti mode) or maximum percentage of human power drawn by motor (torque mode)
const int thermal_limit=150;             //Maximum continuous thermal load motor can withstand
const int thermal_safe_speed=12;         //Speed above which motor is thermally safe at maximum current, see EPACSim
const int whkm_max=30;                   //Maximum wh/km consumption in CONTROL_MODE_LIMIT_WH_PER_KM (controls poti-range)
const unsigned int idle_shutdown_secs = 30 * 60;           // Idle shutdown in seconds. Max is ~1080 minutes or 18 hours
const unsigned int menu_idle_timeout_secs = 60;            // Menu inactivity timeout in seconds.
const double capacity = 166.0;           //battery capacity in watthours for range calculation
const double pas_factor_min=1.2;         //Use pas_factor from hardware-test here with some tolerances. Both values have to be eihter larger or smaller than 1.0 and not 0.0!
const double pas_factor_max=3;           //Use pas_factor from hardware-test here with some tolerances. Both values have to be eihter larger or smaller than 1.0 and not 0.0!
const int pas_magnets=5;                 //number of magnets in your PAS sensor. When using a Thun X-Cell RT set this to 8
const double cfg_pid_p=0.0;              //pid p-value, default: 0.0
const double cfg_pid_i=2.0;              //pid i-value, default: 2.0
const double cfg_pid_p_throttle=0.05;    //pid p-value for throttle mode
const double cfg_pid_i_throttle=2.5;     //pid i-value for throttle mode
const byte pulse_min=150;                //lowest value of desired pulse range in bpm
const byte pulse_range=20;               //width of desired pulse range in bpm
const int pas_timeout=500;               //time in ms after which pedaling is set to false
const int torque_throttle_min=5;         //minimum torque in Nm to trigger starting aid
const int torque_throttle_full=20;       //torque to give full throttle
#define TORQUE_ZERO 533                  //Offset of X-Cell RT torque sensor.
#define TORQUE_AUTOZERO                  //uncomment to re-zero torque sensor at startup
//Config Options for profile 2-----------------------------------------------------------------------------------------------------
const int startingaid_speed_2 = 6;
const int spd_max1_2=22;                   //speed cutoff start in Km/h
const int spd_max2_2=25;                   //speed cutoff stop (0W) in Km/h
const int power_max_2=500;                 //Maximum power in W (throttle mode)
const int power_poti_max_2=500;            //Maximum power in W (poti mode) or maximum percentage of human power drawn by motor (torque mode)
const double capacity_2 = 166.0;           //battery capacity in watthours for range calculation

// voltage and current calibration
const float voltage_amplitude = 0.0587;       // set this value according to your own voltage-calibration. Default: 0.0587
const float voltage_offset = 0.0;             // set this value according to your own voltage-calibration. Default: 0.0
const float current_offset = 0.0;             // for Rev >1.3 ONLY! set this value according to your own current-calibration. Default: 0.0
                                              // for Rev 1.1 - 1.2 the offset is automatically corrected by software!
const float current_amplitude_R11 = 0.0296;   // for Rev 1.1 - 1.2 set this value according to your own current-calibration. Default: 0.0296
const float current_amplitude_R13 = 0.0741;   // for Rev 1.3-2.1 set this value according to your own current-calibration. Default: 0.0741
const float current_amplitude_R22 = 0.04887585533;   // for Rev >= 2.2 set this value according to your own current-calibration. Default: 0.04887585533
// #define USE_EXTERNAL_CURRENT_SENSOR
// #define USE_EXTERNAL_VOLTAGE_SENSOR
const float external_voltage_offset = 0.0;  
const float external_voltage_amplitude = 0.00488758553; //default of 0.00488758553 gives Voltage = Voltage at Pin            
const float external_current_offset = 0.0;    
const float external_current_amplitude = 0.00488758553; //default of 0.00488758553 gives Current = Voltage at Pin 
const int external_current_in = A6;            //For HW Rev. 2.1: use Pin  A6
const int external_voltage_in = A7;            //For HW Rev. 2.1: use Pin  A7

const char msg_welcome[] PROGMEM = "Welcome";
const char msg_shutdown[] PROGMEM = "Live long and prosper.";

const char msg_battery_charged[] PROGMEM = "Batt. charged! Resetting counters";
const char msg_idle_shutdown[] PROGMEM = "Idle shutdown. Good night.";
const char msg_emergency_shutdown[] PROGMEM = "Battery undervoltage detected. Emergency shutdown.";

const char msg_tempomat_reset[] PROGMEM = "Tempomat reset";
const char msg_unknown_action[] PROGMEM = "Unknown action!";
const char msg_activated[] PROGMEM = "Activated";
const char msg_deactivated[] PROGMEM = "Deactivated";
#ifdef SUPPORT_XCELL_RT
const char msg_torquezero[] PROGMEM = "Re-zero torque sensor";
#endif

#ifdef SUPPORT_GEAR_SHIFT
const char msg_gear_shift_low_gear[] PROGMEM = "Low gear active";
const char msg_gear_shift_high_gear[] PROGMEM = "High gear active";
const char msg_gear_shift_auto_selection[] PROGMEM = "Auto gear shift";
#endif

#endif
