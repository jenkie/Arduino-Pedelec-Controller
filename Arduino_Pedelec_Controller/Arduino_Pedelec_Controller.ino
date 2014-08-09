/*
Arduino Pedelec "Forumscontroller" for Hardware 1.1-2.0
written by jenkie and others / pedelecforum.de
Copyright (C) 2012-2014

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

Features:
- pedaling-enabled, true-power-control throttle
- pedaling-enabled, pid-controlled motor-power which can be set by a potentiometer or buttons
- configurable starting aid
- configurable speed limit
- configurable power limit
- switchable profiles
- several displays: BMS, Kingmeter, Nokia LCD, Serial LCD
- optional Bluetooth module to communicate with Android app
*/

#include "config.h"          //place all your personal configurations there and keep that file when updating!   
#include "globals.h"
#include "display.h"         //display output functions
#include "display_backlight.h"  // LCD display backlight support
#include "EEPROM.h"
#include "PID_v1_nano.h"
#include "switches.h"        //contains switch handling functions
#include "menu.h"            //on the go menu
#include "serial_command.h"       //serial (bluetooth) communication stuff

#ifdef SUPPORT_BMP085
#include <Wire.h>
#include "BMP085.h"          //library for altitude and temperature measurement using http://www.watterott.com/de/Breakout-Board-mit-dem-BMP085-absoluten-Drucksensor     
BMP085 bmp;
#endif

#ifdef SUPPORT_DSPC01
#include "DSPC01_nano.h"
DSPC01 dspc;
unsigned long dspc_timer = 0;
boolean dspc_mode=0;  //is false if temperature, true if altitude
#endif

#ifdef SUPPORT_RTC
#include <Wire.h>
#include "ds1307.h"
RTC_DS1307 rtc;
Time now;
#endif

#ifdef SUPPORT_HRMI
#include <Wire.h>
#include "hrmi_funcs.h"
#endif

#if defined(SUPPORT_POTI) && defined(SUPPORT_SOFT_POTI)
#error You either have poti or soft-poti support. Disable one of them.
#endif

#if defined(SUPPORT_POTI) && defined(SUPPORT_SWITCH_ON_POTI_PIN)
#error You either have a poti or a switch on the poti pin. Disable one of them.
#endif

#if defined(SUPPORT_POTI) && defined(SUPPORT_POTI_SWITCHES)
#error You either have a poti or switches to control the poti value
#endif

#if defined(SUPPORT_LIGHTS_SWITCH) && defined(SUPPORT_XCELL_RT)
#error Software controlled lights switch is not compatible with X-CELL RT support
#endif

#if defined(SUPPORT_LIGHTS_SWITCH) && HARDWARE_REV < 3
#error Lights switch is only possible on FC hardware rev 3 or newer
#endif

#if ((DISPLAY_TYPE==DISPLAY_TYPE_KINGMETER)||(DISPLAY_TYPE==DISPLAY_TYPE_BMS)||(DISPLAY_TYPE==DISPLAY_TYPE_BMS3))
#if HARDWARE_REV < 20
#include <SoftwareSerial.h>      //for Kingmeter J-LCD and BMS S-LCD
#endif
#endif


// #define DEBUG_MEMORY_USAGE               // enable this define to print memory usage in SERIAL_MODE_DEBUG

struct savings   //add variables if you want to store additional values to the eeprom
{
    float voltage;
    float wh; //watthours
    float kilometers; //trip kilometers
    float mah; //milliamphours
    unsigned long odo; //overall kilometers in units of wheel roundtrips
};
savings variable = {0.0, 0.0, 0.0, 0.0, 0}; //variable stores last voltage and capacity read from EEPROM
savings variable_new = {0.0, 0.0, 0.0, 0.0, 0}; //variable_new stores new EEPROM values

//Pin Assignments-----------------------------------------------------------------------------------------------------
#if HARDWARE_REV == 1
const int option_pin = A0;         //Analog option
const int voltage_in = A1;         //Voltage read-Pin
const int current_in = A2;         //Current read-Pin
#endif
#if HARDWARE_REV == 2
const int fet_out = A0;              //FET: Pull high to switch off
const int voltage_in = A1;           //Voltage read-Pin
const int option_pin = A2;           //Analog option
const int current_in = A3;           //Current read-Pin
#endif
#if ((HARDWARE_REV >= 3)&&(HARDWARE_REV <= 5))
const int voltage_in = A0;           //Voltage read-Pin
const int fet_out = A1;              //FET: Pull high to switch off
const int current_in = A2;           //Current read-Pin
const int option_pin = A3;           //Analog option
const int lights_pin = A3;           //Software controlled lights switch
#endif
#if HARDWARE_REV <= 5
const int poti_in = A6;              //PAS Speed-Poti-Pin
const int throttle_in = A7;          //Throttle read-Pin
const int pas_in = 2;                //PAS Sensor read-Pin
const int wheel_in = 3;              //Speed read-Pin
const int brake_in = 4;              //Brake-In-Pin
const int switch_thr = 5;            //Throttle-Switch read-Pin
const int throttle_out = 6;          //Throttle out-Pin
const int bluetooth_pin = 7;         //Bluetooth-Supply, do not use in Rev. 1.1!!!
const int switch_disp = 8;           //Display switch
#if (DISPLAY_TYPE & (DISPLAY_TYPE_NOKIA_4PIN|DISPLAY_TYPE_16X2_SERIAL))
const int switch_disp_2 = 12;        //second Display switch with Nokia-Display in 4-pin-mode
#endif
#endif
#if HARDWARE_REV == 20
const int voltage_in = A14;           //Voltage read-Pin
const int fet_out = 38;              //FET: Pull high to switch off
const int current_in = A15;           //Current read-Pin
const int option_pin = A2;            //Analog option
const int lights_pin = 44;           //Software controlled lights switch

const int poti_in = A4;              //PAS Speed-Poti-Pin
const int throttle_in = A3;          //Throttle read-Pin
const int pas_in = 3;                //PAS Sensor read-Pin
const int brake_in = 2;              //Brake-In-Pin
const int switch_thr = 5;            //Throttle-Switch read-Pin
const int throttle_out = 8;          //Throttle out-Pin
const int bluetooth_pin = 13;         //Bluetooth-Supply
const int switch_disp = 37;           //Display switch
// Currently not in use as it's hardwired to PORTH, pin 2
// const int switch_disp_2 = 48;        //second Display switch with Nokia-Display in 4-pin-mode
const int buzzer=11;
#endif



//Variable-Declarations-----------------------------------------------------------------------------------------------
double pid_p=cfg_pid_p;
double pid_i=cfg_pid_i;
double pid_p_throttle=cfg_pid_p_throttle;
double pid_i_throttle=cfg_pid_i_throttle;

double pid_out,pid_set;        //pid output, pid set value
int throttle_stat = 0;         //Throttle reading
int throttle_write=0;          //Throttle write value
int poti_stat = 0;         //Poti reading
volatile int pas_on_time = 0;  //High-Time of PAS-Sensor-Signal (needed to determine pedaling direction)
volatile int pas_off_time = 0; //Low-Time of PAS-Sensor-Signal  (needed to determine pedaling direction)
volatile int pas_failtime = 0; //how many subsequent "wrong" PAS values?
volatile int cad=0;            //Cadence
unsigned long looptime=0;      //Loop Time in milliseconds (for testing)
byte battery_percent_fromcapacity=0;     //battery capacity calculated from capacity
int lowest_raw_current = 1023; //automatic current offset calibration
float current = 0.0;           //measured battery current
float voltage = 0.0;           //measured battery voltage
float voltage_1s,voltage_2s = 0.0; //Voltage history 1s and 2s before "now"
float voltage_display = 0.0;   //averaged voltage
float current_display = 0.0;   //averaged current
double power=0.0;              //calculated power
double power_set = 0;          //Set Power
double power_poti = 0.0;   //set power, calculated with current poti setting
double power_throttle=0.0; //set power, calculated with current throttle setting
float factor_speed=1.0;        //factor controling the speed
float factor_volt=1.0;         //factor controling voltage cutoff
float wh,mah=0.0;              //watthours, mah drawn from battery
float temperature = 0.0;       //temperature
float altitude = 0.0;          //altitude
float altitude_start=0.0;      //altitude at start
float last_altitude;           //height
float slope = 0.0;             //current slope
volatile float km=0.0;         //trip-km
volatile float spd=0.0;        //speed
float range = 0.0;             //expected range
unsigned long odo=0;           //overall kilometers in units of wheel roundtrips
unsigned long last_writetime = millis();  //last time display has been refreshed
volatile unsigned long last_wheel_time = millis(); //last time of wheel sensor change 0->1
volatile unsigned long wheel_time = 65535;  //time for one revolution of the wheel
volatile unsigned long last_pas_event = millis();  //last change-time of PAS sensor status
#define pas_time 60000/pas_magnets //conversion factor for pas_time to rpm (cadence)
volatile boolean pedaling = false;  //pedaling? (in forward direction!)
boolean firstrun = true;  //first run of loop?
boolean brake_stat = true; //brake activated?
PID myPID(&power, &pid_out,&pid_set,pid_p,pid_i,0, DIRECT);
unsigned int idle_shutdown_count = 0;
unsigned long idle_shutdown_last_wheel_time = millis();
byte pulse_human=0;          //cyclist's heart rate
double torque=0.0;           //cyclist's torque
double power_human=0.0;      //cyclist's power
double wh_human=0;
#ifdef SUPPORT_XCELL_RT
int torque_zero=533;             //Offset of X-Cell RT torque sensor. Adjusted at startup
#if HARDWARE_REV<20
const int torquevalues_count=8;
volatile int torquevalues[torquevalues_count]= {0,0,0,0,0,0,0,0}; //stores the 8 torque values per pedal roundtrip
#else
const int torquevalues_count=16;
volatile int torquevalues[torquevalues_count]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; //stores the 16 torque values per pedal roundtrip
#endif
volatile byte torqueindex=0;        //index to write next torque value
volatile boolean readtorque=false;  //true if pas-interrupt received -> read torque in main loop. unfortunately analogRead gives wrong values inside the PAS-interrupt-routine
#endif

#if (SERIAL_MODE & SERIAL_MODE_MMC)           //communicate with mmc-app
String mmc_command="";
byte mmc_value=0;
boolean mmc_nextisvalue=false;
#endif

//declarations for profile switching
int curr_startingaid_speed=startingaid_speed;
int curr_spd_max1=spd_max1;
int curr_spd_max2=spd_max2;
int curr_power_max=power_max;
int curr_power_poti_max=power_poti_max;
double curr_capacity=capacity;
boolean current_profile=0; //0: blue profile, 1: red profile

// On-the-go duct tape to get you home with broken wiring.
boolean first_aid_ignore_break = false;
boolean first_aid_ignore_pas = false;
boolean first_aid_ignore_speed = false;
boolean first_aid_ignore_poti = false;
boolean first_aid_ignore_throttle = false;

// Forward declarations for compatibility with new gcc versions
void pas_change();
void pas_change_thun(boolean signal);
void speed_change();
void send_serial_data();
void handle_dspc();
void read_eeprom();
void save_eeprom();
void save_shutdown();
void handle_unused_pins();
void send_bluetooth_data(HardwareSerial bluetoothSerial);
int analogRead_noISR(uint8_t pin);

#ifdef DEBUG_MEMORY_USAGE
int memFree()
{
    extern int __heap_start, *__brkval;
    int next_pointer;

    return (int) &next_pointer - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif

//Setup---------------------------------------------------------------------------------------------------------------------
void setup()
{
#if HARDWARE_REV >= 2
    pinMode(fet_out,OUTPUT);
    digitalWrite(fet_out, FET_ON);           // turn on whole system on
#endif
#if HARDWARE_REV == 20
    pinMode(buzzer, OUTPUT);
    tone(buzzer, 261, 50);
    delay(50);
    tone(buzzer,329, 50);
    delay(50);
    tone(buzzer,440, 50);
    handle_unused_pins(); //for current saving
    Serial1.begin(115200);     //this is the bluetooth serial port
#endif
    Serial.begin(115200);     //bluetooth-module requires 115200

#ifdef DEBUG_MEMORY_USAGE
    Serial.print(MY_F("memFree before setup:"));
    Serial.print(memFree());
    Serial.print(MY_F("\n"));
#endif

#ifdef SUPPORT_DSPC01
#if HARDWARE_REV < 20
    dspc.begin(A5,A4);
#else
    dspc.begin(21,20);
#endif
    dspc.request_temperature();
    delay(200);
    temperature=dspc.temperature()/10.0;
    dspc.request_altitude();
    delay(200);
    altitude_start=dspc.altitude()/10.0;
#endif
    init_switches();
    init_menu();
    display_init();

#if (DISPLAY_TYPE & (DISPLAY_TYPE_NOKIA_4PIN|DISPLAY_TYPE_16X2_SERIAL))
#if HARDWARE_REV >= 20
    // set to INPUT
    bitClear(DDRH, 2);
    // turn on pullup resistors
    bitSet(PORTH, 2);
#else
    digitalWrite(switch_disp_2, HIGH);    // turn on pullup resistors on display-switch 2
#endif
#endif
#ifdef SUPPORT_SWITCH_ON_POTI_PIN
    digitalWrite(poti_in, HIGH);
#endif

#ifdef SUPPORT_LIGHTS_SWITCH
    pinMode(lights_pin,OUTPUT);
#ifdef SUPPORT_LIGHTS_ENABLE_ON_STARTUP
    digitalWrite(lights_pin, HIGH);     // turn lights on during startup
#else
    digitalWrite(lights_pin, LOW);
#endif
#endif

#if HARDWARE_REV >= 2
    pinMode(bluetooth_pin,OUTPUT);
#ifdef SUPPORT_BLUETOOTH_ENABLE_ON_STARTUP
    digitalWrite(bluetooth_pin, HIGH);    // turn bluetooth on during boot
#else
    digitalWrite(bluetooth_pin, LOW);     // turn bluetooth off
#endif
#endif
#ifdef SUPPORT_BRAKE
    digitalWrite(brake_in, HIGH);         // turn on pullup resistors on brake
#endif
    digitalWrite(switch_thr, HIGH);       // turn on pullup resistors on throttle-switch
    digitalWrite(switch_disp, HIGH);      // turn on pullup resistors on display-switch
#if HARDWARE_REV < 20
    digitalWrite(wheel_in, HIGH);         // turn on pullup resistors on wheel-sensor
#endif
    digitalWrite(pas_in, HIGH);           // turn on pullup resistors on pas-sensor

#if defined(SUPPORT_DISPLAY_BACKLIGHT) && !(DISPLAY_TYPE & DISPLAY_TYPE_16X2_SERIAL)
#if HARDWARE_REV >= 20
    bitSet(DDRH, 2);
    bitClear(PORTH, 2);
#else
    pinMode(display_backlight_pin, OUTPUT);
#endif
#endif

#ifdef SUPPORT_GEAR_SHIFT
    pinMode(gear_shift_pin_low_gear, OUTPUT);
    pinMode(gear_shift_pin_high_gear, OUTPUT);
    // set to auto mode
    digitalWrite(gear_shift_pin_low_gear, HIGH);
    digitalWrite(gear_shift_pin_high_gear, HIGH);
#endif

    read_eeprom();      //read stored variables
    odo=variable.odo;   //load overall kilometers from eeprom
    display_show_welcome_msg();

//setup interrupt handling
#if HARDWARE_REV < 20
#ifdef SUPPORT_PAS
    attachInterrupt(0, pas_change, CHANGE); //attach interrupt for PAS-Sensor
#endif
    attachInterrupt(1, speed_change, RISING); //attach interrupt for Wheel-Sensor
#else
    bitClear(DDRE,7);      //configure PE7 as input
    bitSet(PORTE,7);       //enable pull-up on wheel sensor
    bitSet(EICRB,6);      //trigger on rising edge INT7 for wheel sensor
    bitSet(EICRB,7);      //trigger on rising edge INT7 for wheel sensor
    EIMSK  |= (1<<INT7);  //turn on interrupt for wheel sensor
#ifdef SUPPORT_PAS
    bitClear(DDRE,5);      //configure PE5 as input
    bitSet(PORTE,5);       //enable pull-up on PAS sensor
#ifndef SUPPORT_XCELL_RT
    bitSet(EICRB,2);      //trigger on any edge INT5 for PAS sensor
    EIMSK  |= (1<<INT5);  //turn on interrupt INT5 for PAS sensor
#else
    bitClear(DDRE,6);      //configure PE6 as input
    bitSet(PORTE,6);       //enable pull-up on PAS 2 sensor
    bitSet(EICRB,2);      //trigger on rising edge INT5 for Thun sensor
    bitSet(EICRB,3);      //trigger on rising edge INT5 for Thun sensor
    bitSet(EICRB,4);      //trigger on rising edge INT6 for Thun sensor
    bitSet(EICRB,5);      //trigger on rising edge INT6 for Thun sensor
    EIMSK  |= (1<<INT5);  //turn on interrupt INT5 for PAS sensor
    EIMSK  |= (1<<INT6);  //turn on interrupt for Thun sensor
#endif
#endif
#endif

    myPID.SetMode(AUTOMATIC);             //initialize pid
    myPID.SetOutputLimits(0,1023);        //initialize pid
    myPID.SetSampleTime(10);              //compute pid every 10 ms
#ifdef SUPPORT_BMP085
    Wire.begin();                         //initialize i2c-bus
    bmp.begin();                          //initialize barometric altitude sensor
    temperature = bmp.readTemperature();
    altitude_start=bmp.readAltitude();
#endif
#if defined(SUPPORT_BMP085) || defined(SUPPORT_DSPC01)
    display_show_welcome_msg_temp();
#endif
#ifdef SUPPORT_HRMI
    hrmi_open();
#endif
#ifdef SUPPORT_RTC
    Wire.begin();
#endif
#ifndef SUPPORT_PAS
    pedaling=true;
#endif

#if defined(SUPPORT_SOFT_POTI) || defined(SUPPORT_POTI_SWITCHES)
    poti_stat = map(poti_value_on_startup_in_watts, 0, curr_power_poti_max, 0, 1023);
#endif

#ifdef SUPPORT_XCELL_RT
    torque_zero=analogRead_noISR(option_pin);
#endif

#ifdef DEBUG_MEMORY_USAGE
    Serial.print(MY_F("memFree after setup:"));
    Serial.print(memFree());
    Serial.print(MY_F("\n"));
#endif
}

void loop()
{
    looptime=millis();
//Readings-----------------------------------------------------------------------------------------------------------------
#ifdef SUPPORT_DSPC01
    handle_dspc();
#endif
#ifdef SUPPORT_POTI
    if (first_aid_ignore_poti == false)
        poti_stat = constrain(map(analogRead_noISR(poti_in),poti_offset,poti_max,0,1023),0,1023);   // 0...1023
#endif

#if ((DISPLAY_TYPE==DISPLAY_TYPE_KINGMETER)||(DISPLAY_TYPE==DISPLAY_TYPE_BMS)||(DISPLAY_TYPE==DISPLAY_TYPE_BMS3))
    display_update();
#endif

    if (Serial.available() > 0)
    {
        parse_serial(Serial.read());
    }

#ifdef SUPPORT_THROTTLE
    throttle_stat = constrain(map(analogRead_noISR(throttle_in),throttle_offset,throttle_max,0,1023),0,1023);   // 0...1023
    if (throttle_stat<5 || first_aid_ignore_throttle) //avoid noisy throttle readout
    {
        throttle_stat=0;
    }
#elif (DISPLAY_TYPE!=DISPLAY_TYPE_KINGMETER) && (DISPLAY_TYPE!=DISPLAY_TYPE_BMS)&& (DISPLAY_TYPE!=DISPLAY_TYPE_BMS3)
    // Reset throttle_stat for non-serial communicating displays if SUPPORT_THROTTLE is disabled
    // This is needed to switch off ACTION_FIXED_THROTTLE_VALUE.
    throttle_stat=0;
#endif

#ifdef SUPPORT_BRAKE
#ifdef INVERT_BRAKE
    brake_stat = !digitalRead(brake_in);
#else
    brake_stat = digitalRead(brake_in);
#endif
#endif
#ifdef SUPPORT_FIRST_AID_MENU
    if (first_aid_ignore_break)
        brake_stat = true;
#endif
//voltage, current, power
    voltage = analogRead_noISR(voltage_in)*voltage_amplitude+voltage_offset; //check with multimeter, change in config.h if needed!
#if HARDWARE_REV <= 2
    // Read in current and auto-calibrate the shift offset:
    // There is a constant offset depending on the
    // Arduino / resistor value, so we automatically
    // shift it to zero on the scale.
    int raw_current = analogRead_noISR(current_in);
    if (raw_current < lowest_raw_current)
        lowest_raw_current = raw_current;
    current = (raw_current-lowest_raw_current)*current_amplitude_R11; //check with multimeter, change in config.h if needed!
    current = constrain(current,0,30);
#endif
#if HARDWARE_REV >= 3
    current = (analogRead_noISR(current_in)-512)*current_amplitude_R13+current_offset;    //check with multimeter, change in config.h if needed!
    current = constrain(current,-30,30);
#endif

    voltage_display = 0.99*voltage_display + 0.01*voltage; //averaged voltage for display
    current_display = 0.99*current_display + 0.01*current; //averaged voltage for display
    power=current*voltage;

#ifdef SUPPORT_XCELL_RT
    if (readtorque==true)
    {
        torque=0.0;
        for (int i = 0; i < torquevalues_count; i++)
        {
            torque+=torquevalues[i];
        }
        readtorque=false;
#if HARDWARE_REV<20
        torque=abs((torque)*0.061);
#else
        torque=abs((torque)*0.03054740957);
#endif
        power_human=0.20943951*cad*torque;   //power=2*pi*cadence*torque/60s*2 (*2 because only left side torque is measured by x-cell rt)
    }
#endif

//handle switches----------------------------------------------------------------------------------------------------------
    handle_switch(SWITCH_THROTTLE, digitalRead(switch_thr));
    handle_switch(SWITCH_DISPLAY1, digitalRead(switch_disp));
#if (DISPLAY_TYPE & (DISPLAY_TYPE_NOKIA_4PIN|DISPLAY_TYPE_16X2_SERIAL))
#if HARDWARE_REV >= 20
    handle_switch(SWITCH_DISPLAY2, bitRead(PINH, 2));
#else
    handle_switch(SWITCH_DISPLAY2, digitalRead(switch_disp_2));
#endif
#endif

#ifdef SUPPORT_SWITCH_ON_POTI_PIN
    handle_switch(SWITCH_POTI, (analogRead_noISR(poti_in)>512));
#endif

//Check if Battery was charged since last power down-----------------------------------------------------------------------
    if (firstrun==true)
    {
        bool force_eeprom_load = false;
#ifndef SUPPORT_BATTERY_CHARGE_DETECTION
        force_eeprom_load = true;
#endif

        if (variable.voltage>(voltage - 2) ||                      //charging detected if voltage is 2V higher than last stored voltage
                voltage < battery_charged_min_voltage ||               //and higher than min. charged voltage
                force_eeprom_load)
        {
            wh=variable.wh;
            km=variable.kilometers;
            mah=variable.mah;
        }
        else
            display_show_important_info(FROM_FLASH(msg_battery_charged), 5);
    }
    firstrun=false;                                     //first loop run done (ok, up to this line :))

//Check power-off condition ---------------------------------------------------------------------------------------------
    if ((voltage<20.0)&&(voltage_2s>6.0)) //save to EEPROM when battery disconnect detected. Do this only if not running on usb power
    {
        save_shutdown();
    }

//Are we pedaling?---------------------------------------------------------------------------------------------------------
#ifdef SUPPORT_PAS
    if (((millis()-last_pas_event)>500)||(pas_failtime>pas_tolerance))
    {pedaling = false;}                               //we are not pedaling anymore, if pas did not change for > 0,5 s

    // First aid support: Ignore missing PAS events
    // Note: No need to fix it up in pas_change(), "pedaling" is only set to false above.
    // If we still get some cadence, show it to the rider.
    if (first_aid_ignore_pas)
        pedaling = true;

    cad=cad*pedaling;
#endif

    if ((millis()-last_wheel_time)>3000) //wheel did not spin for 3 seconds --> speed is zero
    {
        spd=0;
        wheel_time=65535;
    }

    // First aid support: Set speed to 1 km/h.
    // This will still show the graphical display.
    if (first_aid_ignore_speed)
    {
        spd=1;
    }

//Power control-------------------------------------------------------------------------------------------------------------
    power_throttle = throttle_stat / 1023.0 * curr_power_max;         //power currently set by throttle

#if CONTROL_MODE == CONTROL_MODE_TORQUE                      //human power control mode
#ifdef SUPPORT_XCELL_RT
    power_poti = poti_stat/102300.0* curr_power_poti_max*power_human*(1+spd/20.0); //power_poti_max is in this control mode interpreted as percentage. Example: power_poti_max=200 means; motor power = 200% of human power
#endif
#endif

#if CONTROL_MODE == CONTROL_MODE_NORMAL                      //constant power mode
    power_poti = poti_stat / 1023.0 * curr_power_poti_max;
#endif

#if CONTROL_MODE == CONTROL_MODE_LIMIT_WH_PER_KM            //wh/km control mode
    power_poti = poti_stat / 1023.0 * whkm_max * spd;        //power currently set by poti in relation to speed and maximum wattage per km
#endif

#ifdef SUPPORT_HRMI                                          //limit heart rate to specified range if possible
    if (pulse_human>0)
    {
        power_poti=min(power_poti+curr_power_max*constrain((pulse_human-pulse_min)/pulse_range,0.0,1.0),curr_power_max);
    }
#endif

    power_poti = min(power_poti,thermal_limit+(curr_power_max-thermal_limit)*constrain(spd/thermal_safe_speed,0,1)); //thermal limiting

    if ((power_throttle) > (power_poti))                     //IF power set by throttle IS GREATER THAN power set by poti (throttle override)
    {
        myPID.SetTunings(pid_p_throttle,pid_i_throttle,0);   //THEN throttle mode: throttle sets power with "agressive" p and i parameters        power_set=throttle_stat/1023.0*power_max;
        power_set = power_throttle;
    }
    else                                                     //ELSE poti mode: poti sets power with "soft" p and i parameters
    {
        myPID.SetTunings(pid_p,pid_i,0);
        power_set = power_poti;
    }

//Speed cutoff-------------------------------------------------------------------------------------------------------------
    if (pedaling==true)
    {factor_speed=constrain(1-(spd-curr_spd_max1)/(curr_spd_max2-curr_spd_max1),0,1);} //linear decrease of maximum power for speeds higher than spd_max1
    else
    {
        if (startingaidenable==true)                         //starting aid activated
        {factor_speed=constrain((curr_startingaid_speed-spd)/2,0,1);}
        else
        {factor_speed=0;}                                    //no starting aid
    }

    if (power_set>curr_power_max*factor_speed)
    {power_set=curr_power_max*factor_speed;}                  //Maximum allowed power including Speed-Cutoff
    if ((((poti_stat<=throttle_stat)||(pedaling==false))&&(throttle_stat==0))||(brake_stat==0))  //power_set is set to -60W when you stop pedaling or brake (this is the pid-input)
    {power_set=-60;}


//Voltage cutoff----------------------------------------------------------------------------------------------------------
    if (voltage<vcutoff)
    {factor_volt=factor_volt*0.9997;}
    else
    {factor_volt=factor_volt*0.9997+0.0003;}

//Throttle output-------------------------------------------------------------------------------------------------------

    pid_set=power_set;
    myPID.Compute();                                      //this computes the needed drive voltage for the motor controller to maintain the "power_set" based on the current "power" measurment
#ifdef SUPPORT_MOTOR_GUESS
    throttle_write=map(pid_out*brake_stat*factor_volt,0,1023,motor_offset,motor_max) + spd/spd_idle*(motor_max-motor_offset);
    throttle_write=constrain(throttle_write,0,motor_max);
#else
    throttle_write=map(pid_out*brake_stat*factor_volt,0,1023,motor_offset,motor_max);
#endif
#ifdef SUPPORT_PAS
    if ((pedaling==false)&&(throttle_stat<5)||power_set<=0||spd>curr_spd_max2)
#else
    if (throttle_stat<5||spd>curr_spd_max2)
#endif
    {throttle_write=0;}
    analogWrite(throttle_out,throttle_write);

#ifdef SUPPORT_DISPLAY_BACKLIGHT
    handle_backlight();
#endif

#if (SERIAL_MODE & SERIAL_MODE_MMC)           //communicate with mmc-app
    if(Serial.available())
    {
        while(Serial.available())
        {
            char readchar=Serial.read();
            //Serial.println((char)readchar);
            if (readchar==13)                        //command without value received
            {
                if (mmc_command=="at-ccap")            //reset capacity
                {
                    wh=0;
                    mah=0;
                }
                if (mmc_command=="at-cdist")          //reset distance
                    km=0;
                if (mmc_command=="at-0")              //anybody there?
                    Serial.println(MY_F("ok"));
                readchar=0;
                mmc_command="";
                return;
            }
            if (readchar==10)                       //ignore newline
            {return;}
            if (mmc_nextisvalue)                    //command with value received
            {
                mmc_value=(char)readchar;
                //if (mmc_command=="at-light")        //switch on and off light
                //Serial.println((char)mmc_value);

                mmc_command="";
                mmc_nextisvalue=false;
            }
            else
            {
                if (readchar==61)                        //equal-sign received
                    mmc_nextisvalue=true;
                else
                    mmc_command+=readchar;
            }

        }
    }
#endif

    // Super-fast menu system (no delay)
    if (menu_active)
        display_update();

//slow loop start----------------------//use this subroutine to place any functions which should happen only once a second
    if (millis()-last_writetime > 1000)              //don't do this more than once a second
    {
        voltage_2s=voltage_1s;                           //update voltage history
        voltage_1s=voltage;                              //update voltage history

#ifdef SUPPORT_BMP085
        temperature = bmp.readTemperature();
        altitude = bmp.readAltitude()-altitude_start;
#endif

        battery_percent_fromcapacity = constrain((1-wh/ curr_capacity)*100,0,100);     //battery percent calculation from battery capacity. For voltage-based calculation see above
        range=constrain(curr_capacity/wh*km-km,0.0,200.0);               //range calculation from battery capacity
        wh+=current*(millis()-last_writetime)/3600000.0*voltage;  //watthours calculation
        wh_human+=(millis()-last_writetime)/3600000.0*power_human;  //human watthours calculation
        mah+=current*(millis()-last_writetime)/3600.0;  //mah calculation

#if !((DISPLAY_TYPE==DISPLAY_TYPE_KINGMETER)||(DISPLAY_TYPE==DISPLAY_TYPE_BMS)||(DISPLAY_TYPE==DISPLAY_TYPE_BMS3))
#if !(DISPLAY_TYPE & DISPLAY_TYPE_NONE)
        display_update();
#endif
#endif

        send_serial_data();                                        //sends data over serial port depending on SERIAL_MODE
#if HARDWARE_REV >= 20        
        send_bluetooth_data(Serial1);
#endif

#if HARDWARE_REV >= 2
// Idle shutdown
            if (last_wheel_time != idle_shutdown_last_wheel_time)
            {
                idle_shutdown_last_wheel_time = last_wheel_time;
                idle_shutdown_count = 0;
            }
            else
            {
                ++idle_shutdown_count;
                if (idle_shutdown_count > idle_shutdown_secs)
                {
                    display_show_important_info(FROM_FLASH(msg_idle_shutdown), 60);
                    save_shutdown();
                }
            }

// Emergency power down to protect battery from undervoltage.
// Also checks averaged voltage to prevent ADC read errors killing the system.
// Don't shut down on USB power, too.
            if (voltage < vemergency_shutdown && voltage_display < vemergency_shutdown
                    && voltage > 6.0)
            {
                display_show_important_info(FROM_FLASH(msg_emergency_shutdown), 60);
                delay(1000);
                save_shutdown();
            }
#endif
#ifdef SUPPORT_HRMI
        pulse_human=getHeartRate();
#endif

#ifdef SUPPORT_RTC
        now=rtc.get_time(); //read current time
        //Serial.print(now.hh, DEC);
        //Serial.print(':');
        //Serial.print(now.mm, DEC);
        //Serial.print(':');
        //Serial.print(now.ss, DEC);
        //Serial.println();
#endif

        last_writetime=millis();
//slow loop end------------------------------------------------------------------------------------------------------
    }
}

#if HARDWARE_REV >= 20 //attach interrupts manually
ISR(INT7_vect)
{
    speed_change();
}
#ifdef SUPPORT_PAS
#ifdef SUPPORT_XCELL_RT
ISR(INT5_vect)
{
    pas_change_thun(false);
}
ISR(INT6_vect)
{
    pas_change_thun(true);
}
#else //no thun bracket
ISR(INT5_vect)
{
    pas_change();
}
#endif
#endif
#endif

#if HARDWARE_REV >= 20
#ifdef SUPPORT_XCELL_RT
void pas_change_thun(boolean signal)
{
    if (signal)
        pedaling=bitRead(PINE,5);
    else
    {
        pedaling=!bitRead(PINE,6);
        cad=7500/(millis()-last_pas_event);
        last_pas_event = millis();
    }
    torquevalues[torqueindex]=analogRead_noISR(option_pin)-torque_zero;
    torqueindex++;
    if (torqueindex==torquevalues_count)
        torqueindex=0;
    readtorque=true;
}
#endif
#endif

#ifdef SUPPORT_PAS
void pas_change()       //Are we pedaling? PAS Sensor Change------------------------------------------------------------------------------------------------------------------
{
    if (last_pas_event>(millis()-10)) return;
    boolean pas_stat=digitalRead(pas_in);
    if (pas_stat)
    {
        pas_off_time=millis()-last_pas_event;
#ifdef SUPPORT_XCELL_RT
        ADMUX = (DEFAULT << 6) | ((option_pin-14) & 0x07); //select ADC input channel
        delayMicroseconds(125); //wait 125 µs to settle at new channel. simple analogread is not working in interrupt
        torquevalues[torqueindex]=analogRead_noISR(option_pin)-torque_zero;
        torqueindex++;
        if (torqueindex==torquevalues_count)
            torqueindex=0;
        readtorque=true;
#endif
    }
    else
    {pas_on_time=millis()-last_pas_event;}
    last_pas_event = millis();
    pas_failtime=pas_failtime+1;
    cad=pas_time/(pas_on_time+pas_off_time);
    double pas_factor=(double)pas_on_time/(double)pas_off_time;
    if ((pas_factor>pas_factor_min)&&(pas_factor<pas_factor_max))
    {
        pedaling=true;
        pas_failtime=0;
    }
}
#else
#warning PAS sensor support is required for legal operation of a Pedelec  by EU-wide laws except Austria or Swiss.
#endif

void speed_change()    //Wheel Sensor Change------------------------------------------------------------------------------------------------------------------
{
#ifdef SUPPORT_FIRST_AID_MENU
    if (first_aid_ignore_speed)
        return;
#endif

    //Speed and Km
    if (last_wheel_time>(millis()-50)) return;                         //debouncing reed-sensor
    ++odo;
    wheel_time=millis()-last_wheel_time;
    spd = (spd+3600*wheel_circumference/wheel_time)/2;  //a bit of averaging for smoother speed-cutoff
    if (spd<100)
    {km=km+wheel_circumference/1000.0;}
    else
    {spd=0;}
#if defined(SUPPORT_BMP085) || defined(SUPPORT_DSPC01)
//slope-stuff start-------------------------------
    slope=0.98*slope+2*(altitude-last_altitude)/wheel_circumference;
    last_altitude=altitude;
//slope-stuff end---------------------------------
#endif
    last_wheel_time=millis();
}

void send_bluetooth_data(HardwareSerial bluetoothSerial)
{
#if (HARDWARE_REV >= 20)||((SERIAL_MODE & SERIAL_MODE_ANDROID)&&(HARDWARE_REV < 20))
if (digitalRead(bluetooth_pin))
{
    bluetoothSerial.print(voltage,1);
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.print(current,1);
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.print((int)power);
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.print(spd,1);
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.print(km,3);
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.print((int)cad);
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.print((int)wh);
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.print((int)power_human);
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.print((int)wh_human);
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.print((int)(poti_stat/1023.0*curr_power_poti_max));
    bluetoothSerial.print(MY_F(";"));
    bluetoothSerial.println(CONTROL_MODE);
}
#endif
}

void send_serial_data()  //send serial data----------------------------------------------------------
{
#if (SERIAL_MODE & SERIAL_MODE_ANDROID)&&(HARDWARE_REV<20)
    send_bluetooth_data(Serial);
#endif

#if (SERIAL_MODE & SERIAL_MODE_LOGVIEW)
    Serial.print(MY_F("$1;1;0;"));
    Serial.print(voltage,2);
    Serial.print(MY_F(";"));
    Serial.print(current,2);
    Serial.print(MY_F(";"));
    Serial.print(wh,1);
    Serial.print(MY_F(";"));
    Serial.print(spd,2);
    Serial.print(MY_F(";"));
    Serial.print(km,3);
    Serial.print(MY_F(";"));
    Serial.print(cad);
    Serial.print(MY_F(";"));
    Serial.print(0);   //arbitrary user data here
    Serial.print(MY_F(";"));
    Serial.print(0);   //arbitrary user data here
    Serial.print(MY_F(";"));
    Serial.print(0);   //arbitrary user data here
    Serial.print(MY_F(";"));
    Serial.print(0);  //arbitrary user data here
    Serial.print(MY_F(";"));
    Serial.print(0);   //arbitrary user data here
    Serial.print(MY_F(";"));
    Serial.print(0);   //arbitrary user data here
    Serial.print(MY_F(";0"));
    Serial.println(13,DEC);
#endif

#if (SERIAL_MODE & SERIAL_MODE_DEBUG)
#ifdef DEBUG_MEMORY_USAGE
    Serial.print(MY_F("memFree"));
    Serial.print(memFree());
    Serial.print(MY_F(" looptime"));
    Serial.print(millis()-looptime);
    Serial.print(MY_F(" "));
#endif
    Serial.print(MY_F("Voltage"));
    Serial.print(voltage,2);
    Serial.print(MY_F(" Current"));
    Serial.print(current,1);
    Serial.print(MY_F(" Power"));
    Serial.print(power,0);
    Serial.print(MY_F(" PAS_On"));
    Serial.print(pas_on_time);
    Serial.print(MY_F(" PAS_Off"));
    Serial.print(pas_off_time);
    Serial.print(MY_F(" PAS_factor"));
    Serial.print((float)pas_on_time/pas_off_time);
    Serial.print(MY_F(" Speed"));
    Serial.print(spd);
    Serial.print(MY_F(" Brake"));
    Serial.print(brake_stat);
    Serial.print(MY_F(" Poti"));
    Serial.print(poti_stat);
    Serial.print(MY_F(" Throttle"));
    Serial.print(throttle_stat);
    /*
        Serial.print(MY_F(" TEMP"));
        Serial.print(temperature);
        Serial.print(MY_F(" ALTI"));
        Serial.print(altitude);
        Serial.print(MY_F("/"));
        Serial.print(altitude_start);
    */
    //now: data for Arduino Pedelec Configurator
    //0:voltage 1:current 2:pasfactor*100 3:option-pin 4:poti 5:throttle 6: brake
    Serial.print(MY_F("---raw---"));
    Serial.print(analogRead_noISR(voltage_in)); Serial.print(MY_F(";"));
    Serial.print(analogRead_noISR(current_in)); Serial.print(MY_F(";"));
    Serial.print(((int)(100*(double)pas_on_time/(double)pas_off_time))); Serial.print(MY_F(";"));
    Serial.print(analogRead_noISR(option_pin)); Serial.print(MY_F(";"));
    Serial.print(analogRead_noISR(poti_in)); Serial.print(MY_F(";"));
    Serial.print(analogRead_noISR(throttle_in)); Serial.print(MY_F(";"));
    Serial.println(digitalRead(brake_in));

#endif

#if (SERIAL_MODE & SERIAL_MODE_MMC)
    Serial.print((int)(voltage_display*10));
    Serial.print(MY_F("\t"));
    Serial.print((int)(current*1000));
    Serial.print(MY_F("\t"));
    Serial.print((int)(spd*10));
    Serial.print(MY_F("\t"));
    Serial.print((long)(mah*3600));
    Serial.print(MY_F("\t"));
    Serial.print((int)0); //naximum current
    Serial.print(MY_F("\t"));
    Serial.print((long)(km*1000/wheel_circumference)); //distance
    Serial.print(MY_F("\t"));
    Serial.println((int)0); //profile
#endif
}

void activate_new_profile()
{
    if (current_profile==0)
    {
        curr_startingaid_speed =startingaid_speed;
        curr_spd_max1= spd_max1;
        curr_spd_max2=spd_max2;
        curr_power_max=power_max;
        curr_power_poti_max=power_poti_max;
        curr_capacity = capacity;
    }
    else
    {
        curr_startingaid_speed = startingaid_speed_2;
        curr_spd_max1= spd_max1_2;
        curr_spd_max2=spd_max2_2;
        curr_power_max=power_max_2;
        curr_power_poti_max=power_poti_max_2;
        curr_capacity = capacity_2;
    }
}

void handle_dspc()
{
#ifdef SUPPORT_DSPC01
    if (!dspc_mode) //altitude mode
    {
        if ((millis()-dspc_timer)>200)
        {
            altitude=dspc.altitude()/10.0-altitude_start;
            dspc_mode=1;
            dspc_timer=millis();
            dspc.request_temperature();
        }
    }
    else
    {
        if ((millis()-dspc_timer)>200)
        {
            temperature=dspc.temperature()/10.0;
            dspc_mode=0;
            dspc_timer=millis();
            dspc.request_altitude();
        }
    }
#endif
}

void save_eeprom()
{
  //save the voltage value 2 seconds before switch-off-detection    
  if (voltage_2s)
      variable_new.voltage=voltage_2s;
  else
      variable_new.voltage=voltage;

  variable_new.wh=wh;          //save watthours drawn from battery
  variable_new.kilometers=km;  //save trip kilometers
  variable_new.mah=mah;        //save milliamperehours drawn from battery
  variable_new.odo=odo;        //save total kilometers
  const byte* p_new = (const byte*)(const void*)&variable_new; //pointer to new variables to save
  const byte* p_old = (const byte*)(const void*)&variable; //pointer to current EEPROM content
  int i;
  for (i = 0; i < sizeof(variable_new); i++)
  {
    if (*p_new==*p_old) //this byte has not changed --> ignore
      *p_new++;
    else
      EEPROM.write(i, *p_new++); //this byte has changed --> write     
    p_old++;
  }  
}

void read_eeprom()
{
  byte* p = (byte*)(void*)&variable;
  int i;
  cli();
  for (i = 0; i < sizeof(variable); i++)
    *p++ = EEPROM.read(i);
  sei();
}

void save_shutdown()
{
  //power saving stuff. This is critical if battery is disconnected.
  EIMSK=0; //disable interrupts
  cli(); //disable interrupts
  ADCSRA = 0; //disable ADC 
#if HARDWARE_REV < 20
  PRR=B11101111;
#else
  PRR0=B11101111; //shut down I2C, Timers, ADCs, UARTS
  PRR1=B00111111; //shut down I2C, Timers, ADCs, UARTS   
#endif  
  digitalWrite(throttle_out,0); //turn motor off
  
  save_eeprom(); //save variables now
  
  digitalWrite(fet_out,FET_OFF); //turn off
  while(true); //there is nothing more to do -> stay in endless loop until turned off
}

void handle_unused_pins()
{
#if HARDWARE_REV >= 20
  //this saves 10-20 mA!
  DDRA=0; //set all Ports to input
  PORTA = B11111111;
  PORTB|= B01010001;
  PORTC = B11111111;
  PORTD|= B01110000;
  PORTE|= B00000100;
  PORTF|= B11000011;
  PORTG|= B00011111;
  PORTH|= B10000000;
  PORTJ|= B11111100;
  PORTK|= B00111111;
  PORTL|= B11000111;
#endif
}

int analogRead_noISR(uint8_t pin) //this function disables globals interrupt before analogRead because analogRead does not like interrupts
{
cli();
int temp=analogRead_noISR(pin);
sei();
return temp;
}
