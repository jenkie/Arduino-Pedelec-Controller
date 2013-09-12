/*
Arduino Pedelec "Forumscontroller" for Hardware 1.1-1.4
written by jenkie and others / pedelecforum.de
Copyright (C) 2012-2013

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
- pedaling-enabled, pid-controlled motor-power which can be set by a potentiometer
- starting aid up to 6 km/h
- limit maximum speed
- limit maximum electric power to 500 W (which means "250 W Nennleistung" :P)
- display: Nokia 5510
- sends Amarino-compatible data over bluetooth
*/

#include "config.h"          //place all your personal configurations there and keep that file when updating!   
#include "globals.h"
#include "display.h"         //display output functions
#include "display_backlight.h"  // LCD display backlight support
#include "EEPROM.h"
#include "EEPROMAnything.h"  //to enable data storage when powered off
#include "PID_v1_nano.h"
#include "switches.h"        //contains switch handling functions
#include "menu.h"            //on the go menu
#include "bluetooth.h"       //bluetooth communication stuff

#ifdef SUPPORT_BMP085
#include <Wire.h>
#include "BMP085.h"          //library for altitude and temperature measurement using http://www.watterott.com/de/Breakout-Board-mit-dem-BMP085-absoluten-Drucksensor     
BMP085 bmp;
#endif

#ifdef SUPPORT_DSPC01
#include "DSPC01_nano.h"    
DSPC01 dspc;
long int dspc_timer;
boolean dspc_mode=0;  //is false if temperature, true if altitude
#endif

#ifdef SUPPORT_HRMI
#include <Wire.h>
#include "hrmi_funcs.h"
#endif

#if defined(SUPPORT_POTI) && defined(SUPPORT_SOFT_POTI)
#error You either have poti or soft-poti support. Disable one of them.
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
#include <SoftwareSerial.h>      //for Kingmeter J-LCD
#endif

struct savings   //add variables if you want to store additional values to the eeprom
{
    float voltage;
    float wh; //watthours
    float kilometers; //trip kilometers
    float mah; //milliamphours
    unsigned long odo; //overall kilometers in units of wheel roundtrips
};
savings variable = {0.0,0.0,0.0,0}; //variable stores last voltage and capacity read from EEPROM

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
#if HARDWARE_REV >= 3
const int voltage_in = A0;           //Voltage read-Pin
const int fet_out = A1;              //FET: Pull high to switch off
const int current_in = A2;           //Current read-Pin
const int option_pin = A3;            //Analog option
#endif
const int poti_in = A6;              //PAS Speed-Poti-Pin
const int throttle_in = A7;          //Throttle read-Pin
const int pas_in = 2;                //PAS Sensor read-Pin
const int wheel_in = 3;              //Speed read-Pin
const int brake_in = 4;              //Brake-In-Pin
const int switch_thr = 5;            //Throttle-Switch read-Pin
const int throttle_out = 6;          //Throttle out-Pin
const int bluetooth_pin = 7;         //Bluetooth-Supply, do not use in Rev. 1.1!!!
const int switch_disp = 8;           //Display switch
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA_4PIN)
const int switch_disp_2 = 12;        //second Display switch with Nokia-Display in 4-pin-mode
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
int looptime=0;                //Loop Time in milliseconds (for testing)
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
volatile unsigned long wheel_time = 0;  //time for one revolution of the wheel
volatile unsigned long last_pas_event = millis();  //last change-time of PAS sensor status
#define pas_time 60000/pas_magnets //conversion factor for pas_time to rpm (cadence)
volatile boolean pedaling = false;  //pedaling? (in forward direction!)
boolean firstrun = true;  //first run of loop?
boolean variables_saved = false; //has everything been saved after Switch-Off detected?
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
const int torquevalues_count=8;
volatile int torquevalues[torquevalues_count]= {0,0,0,0,0,0,0,0}; //stores the 8 torque values per pedal roundtrip
volatile byte torqueindex=0;        //index to write next torque value
volatile boolean readtorque=false;  //true if pas-interrupt received -> read torque in main loop. unfortunately analogRead gives wrong values inside the PAS-interrupt-routine
#endif

#if (SERIAL_MODE & SERIAL_MODE_MMC)           //communicate with mmc-app
String mmc_command="";
byte mmc_value=0;
boolean mmc_nextisvalue=false;
#endif

#if (SERIAL_MODE & SERIAL_MODE_ANDROID)        
char inchar = 0;      //curent read char
#endif

//declarations for profile switching
const int *ptr_startingaid_speed = &startingaid_speed;
const int *ptr_spd_max1= &spd_max1;                 
const int *ptr_spd_max2=&spd_max2;               
const int *ptr_power_max=&power_max;                 
const int *ptr_power_poti_max=&power_poti_max;            
const double *ptr_capacity = &capacity;    
boolean current_profile=0; //0: blue profile, 1: red profile



// Forward declarations for compatibility with new gcc versions
void pas_change();
void speed_change();
void send_serial_data();
void set_profile();
void handle_dspc();

//Setup---------------------------------------------------------------------------------------------------------------------
void setup()
{
#ifdef SUPPORT_DSPC01
    dspc.begin(A5,A4);
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

    Serial.begin(115200);     //bluetooth-module requires 115200
    pinMode(pas_in, INPUT);
    pinMode(wheel_in, INPUT);
    pinMode(switch_thr, INPUT);
    pinMode(switch_disp, INPUT);
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA_4PIN)
    pinMode(switch_disp_2, INPUT);
    digitalWrite(switch_disp_2, HIGH);    // turn on pullup resistors on display-switch 2
#endif
#ifdef SUPPORT_BRAKE 
    pinMode(brake_in, INPUT);
#endif
    pinMode(option_pin,INPUT);
#if HARDWARE_REV >= 2
    pinMode(fet_out,OUTPUT);
    pinMode(bluetooth_pin,OUTPUT);
    digitalWrite(bluetooth_pin, LOW);     // turn bluetooth off
    digitalWrite(fet_out, LOW);           // turn on whole system on (write high to fet_out if you want to power off)
#endif
#ifdef SUPPORT_BRAKE 
    digitalWrite(brake_in, HIGH);         // turn on pullup resistors on brake
#endif
    digitalWrite(switch_thr, HIGH);       // turn on pullup resistors on throttle-switch
    digitalWrite(switch_disp, HIGH);      // turn on pullup resistors on display-switch
    digitalWrite(wheel_in, HIGH);         // turn on pullup resistors on wheel-sensor
    digitalWrite(pas_in, HIGH);           // turn on pullup resistors on pas-sensor
#ifdef SUPPORT_DISPLAY_BACKLIGHT
    pinMode(display_backlight_pin, OUTPUT);
#endif

    EEPROM_readAnything(0,variable);      //read stored variables
    odo=variable.odo;                     //load overall kilometers from eeprom
    display_show_welcome_msg();

#ifdef SUPPORT_PAS
    attachInterrupt(0, pas_change, CHANGE); //attach interrupt for PAS-Sensor
#endif
    attachInterrupt(1, speed_change, RISING); //attach interrupt for Wheel-Sensor
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
#ifndef SUPPORT_PAS
    pedaling=true;
#endif

#ifdef SUPPORT_XCELL_RT
    torque_zero=analogRead(option_pin);   
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
    poti_stat=analogRead(poti_in);                       // 0...1023
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
    display_update();
#endif

#if (SERIAL_MODE & SERIAL_MODE_ANDROID)
   if (Serial.available() > 0){
      inchar = Serial.read();
      next(inchar);
    }
#endif

#ifdef SUPPORT_THROTTLE
    throttle_stat = constrain(map(analogRead(throttle_in),throttle_offset,throttle_max,0,1023),0,1023);   // 0...1023
#endif
#ifdef SUPPORT_BRAKE
#ifdef INVERT_BRAKE
    brake_stat = !digitalRead(brake_in);
#else
    brake_stat = digitalRead(brake_in);
#endif
#endif
//voltage, current, power
    voltage = analogRead(voltage_in)*voltage_amplitude+voltage_offset; //check with multimeter, change in config.h if needed!
#if HARDWARE_REV <= 2
    // Read in current and auto-calibrate the shift offset:
    // There is a constant offset depending on the
    // Arduino / resistor value, so we automatically
    // shift it to zero on the scale.
    int raw_current = analogRead(current_in);
    if (raw_current < lowest_raw_current)
        lowest_raw_current = raw_current;
    current = (raw_current-lowest_raw_current)*current_amplitude_R11; //check with multimeter, change in config.h if needed!
    current = constrain(current,0,30);
#endif
#if HARDWARE_REV >= 3
    current = (analogRead(current_in)-512)*current_amplitude_R13+current_offset;    //check with multimeter, change in config.h if needed!
    current = constrain(current,-30,30);
#endif

    voltage_display = 0.99*voltage_display + 0.01*voltage; //averaged voltage for display
    current_display = 0.99*current_display + 0.01*current; //averaged voltage for display
    power=current*voltage;

#ifdef SUPPORT_XCELL_RT
    if (readtorque==true)
    {
        torquevalues[torqueindex]=analogRead(option_pin)-torque_zero;
        torqueindex++;
        torque=0.0;
        for (int i = 0; i < torquevalues_count; i++)
        {
            torque+=torquevalues[i];
        }
        if (torqueindex==torquevalues_count)
            torqueindex=0;

        readtorque=false;
        torque=abs((torque)*0.061);
        power_human=0.20943951*cad*torque;   //power=2*pi*cadence*torque/60s*2 (*2 because only left side torque is measured by x-cell rt)
    }
#endif

//handle switches----------------------------------------------------------------------------------------------------------
    handle_switch(SWITCH_THROTTLE, digitalRead(switch_thr));
    handle_switch(SWITCH_DISPLAY1, digitalRead(switch_disp));
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA_4PIN)
    handle_switch(SWITCH_DISPLAY2, digitalRead(switch_disp_2));
#endif

//Check if Battery was charged since last power down-----------------------------------------------------------------------
    if (firstrun==true)
    {
        if (variable.voltage>(voltage - 2))                //charging detected if voltage is 2V higher than last stored voltage
        {
            wh=variable.wh;
            km=variable.kilometers;
            mah=variable.mah;
        }
        if (voltage<6.0)                                   //do not write new data to eeprom when on USB Power
        {variables_saved=true;}
    }
    firstrun=false;                                     //first loop run done (ok, up to this line :))


//Are we pedaling?---------------------------------------------------------------------------------------------------------
#ifdef SUPPORT_PAS
    if (((millis()-last_pas_event)>500)||(pas_failtime>pas_tolerance))
    {pedaling = false;}                               //we are not pedaling anymore, if pas did not change for > 0,5 s
    cad=cad*pedaling;
#endif

    if ((millis()-last_wheel_time)>3000)               //wheel did not spin for 3 seconds --> speed is zero
    {
        spd=0;
        wheel_time=0;
    }


//Power control-------------------------------------------------------------------------------------------------------------
    power_throttle = throttle_stat / 1023.0 * *ptr_power_max;         //power currently set by throttle

#if CONTROL_MODE == CONTROL_MODE_TORQUE                      //human power control mode
#ifdef SUPPORT_XCELL_RT
    power_poti = poti_stat/102300.0* *ptr_power_poti_max*power_human*(1+spd/20.0); //power_poti_max is in this control mode interpreted as percentage. Example: power_poti_max=200 means; motor power = 200% of human power
#endif
#endif

#if CONTROL_MODE == CONTROL_MODE_NORMAL                      //constant power mode
    power_poti = poti_stat / 1023.0 * *ptr_power_poti_max;
#endif

#if CONTROL_MODE == CONTROL_MODE_LIMIT_WH_PER_KM            //wh/km control mode
    power_poti = poti_stat / 1023.0 * whkm_max * spd;        //power currently set by poti in relation to speed and maximum wattage per km
#endif

#ifdef SUPPORT_HRMI                                          //limit heart rate to specified range if possible
    if (pulse_human>0)
    {
        power_poti=min(power_poti+*ptr_power_max*constrain((pulse_human-pulse_min)/pulse_range,0.0,1.0),*ptr_power_max);
    }
#endif

    power_poti = min(power_poti,thermal_limit+(*ptr_power_max-thermal_limit)*constrain(spd/thermal_safe_speed,0,1)); //thermal limiting

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
    {factor_speed=constrain(1-(spd-*ptr_spd_max1)/(*ptr_spd_max2-*ptr_spd_max1),0,1);} //linear decrease of maximum power for speeds higher than spd_max1
    else
    {
        if (startingaidenable==true)                         //starting aid activated
        {factor_speed=constrain((*ptr_startingaid_speed-spd)/2,0,1);}
        else
        {factor_speed=0;}                                    //no starting aid
    }

    if (power_set>*ptr_power_max*factor_speed)
    {power_set=*ptr_power_max*factor_speed;}                  //Maximum allowed power including Speed-Cutoff
    if ((((poti_stat<=throttle_stat)||(pedaling==false))&&(throttle_stat<5))||(brake_stat==0))  //power_set is set to -60W when you stop pedaling or brake (this is the pid-input)
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
    if ((pedaling==false)&&(throttle_stat<5)||power_set<=0||spd>*ptr_spd_max2)
#else
    if (throttle_stat<5||spd>*ptr_spd_max2)
#endif
    {throttle_write=0;}
    analogWrite(throttle_out,throttle_write);

//Save capacity to EEPROM
    if ((voltage<20.0)&&(variables_saved==false))    //save to EEPROM when Switch-Off detected
    {
        variable.voltage=voltage_2s;   //save the voltage value 2 seconds before switch-off-detection
        variable.wh=wh;          //save watthours drawn from battery
        variable.kilometers=km;        //save trip kilometers
        variable.mah=mah;        //save milliamperehours drawn from battery
        variable.odo=odo;
        EEPROM_writeAnything(0,variable);
        variables_saved=true;
    }

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
                    Serial.println("ok");
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
        set_profile();                                   //update current profile
        voltage_2s=voltage_1s;                           //update voltage history
        voltage_1s=voltage;                              //update voltage history

#ifdef SUPPORT_BMP085
        temperature = bmp.readTemperature();
        altitude = bmp.readAltitude()-altitude_start;
#endif

        battery_percent_fromcapacity = constrain((1-wh/ *ptr_capacity)*100,0,100);     //battery percent calculation from battery capacity. For voltage-based calculation see above
        range=constrain(*ptr_capacity/wh*km-km,0.0,200.0);               //range calculation from battery capacity
        wh+=current*(millis()-last_writetime)/3600000.0*voltage;  //watthours calculation
        wh_human+=(millis()-last_writetime)/3600000.0*power_human;  //human watthours calculation
        mah+=current*(millis()-last_writetime)/3600.0;  //mah calculation

#if !(DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
#if !(DISPLAY_TYPE & DISPLAY_TYPE_NONE)
        display_update();
#endif
#endif

        send_serial_data();                                        //sends data over serial port depending on SERIAL_MODE

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
                display_show_important_info("Idle shutdown. Good night.", 60);
                digitalWrite(fet_out,HIGH);
            }
        }

// Emergency power down to protect battery from undervoltage.
// Also checks averaged voltage to prevent ADC read errors killing the system.
// Don't shut down on USB power, too.
        if (voltage < vemergency_shutdown && voltage_display < vemergency_shutdown
                && voltage > 6.0)
        {
            display_show_important_info("Battery undervoltage detected. Emergency shutdown.", 60);
            delay(1000);
            digitalWrite(fet_out,HIGH);
        }
#endif
#ifdef SUPPORT_HRMI
        pulse_human=getHeartRate();
#endif
        last_writetime=millis();
//slow loop end------------------------------------------------------------------------------------------------------
    }
}

#ifdef SUPPORT_PAS
void pas_change()       //Are we pedaling? PAS Sensor Change------------------------------------------------------------------------------------------------------------------
{
    if (last_pas_event>(millis()-10)) return;
    boolean pas_stat=digitalRead(pas_in);
    if (pas_stat)
    {
        pas_off_time=millis()-last_pas_event;
#ifdef SUPPORT_XCELL_RT
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


void send_serial_data()  //send serial data----------------------------------------------------------
{
#if (SERIAL_MODE & SERIAL_MODE_ANDROID)
    Serial.print(voltage,1);
    Serial.print(";");
    Serial.print(current,1);
    Serial.print(";");
    Serial.print((int)power);
    Serial.print(";");
    Serial.print(spd,1);
    Serial.print(";");
    Serial.print(km,3);
    Serial.print(";");
    Serial.print((int)cad);
    Serial.print(";");
    Serial.print((int)wh);
    Serial.print(";");
    Serial.print((int)power_human);
    Serial.print(";");
    Serial.print((int)wh_human);
    Serial.print(";");
    Serial.print((int)(poti_stat/1023.0**ptr_power_poti_max));
    Serial.print(";");
    Serial.println(CONTROL_MODE);
#endif

#if (SERIAL_MODE & SERIAL_MODE_LOGVIEW)
    Serial.print("$1;1;0;");
    Serial.print(voltage,2);
    Serial.print(";");
    Serial.print(current,2);
    Serial.print(";");
    Serial.print(wh,1);
    Serial.print(";");
    Serial.print(spd,2);
    Serial.print(";");
    Serial.print(km,3);
    Serial.print(";");
    Serial.print(cad);
    Serial.print(";");
    Serial.print(0);   //arbitrary user data here
    Serial.print(";");
    Serial.print(0);   //arbitrary user data here
    Serial.print(";");
    Serial.print(0);   //arbitrary user data here
    Serial.print(";");
    Serial.print(0);  //arbitrary user data here
    Serial.print(";");
    Serial.print(0);   //arbitrary user data here
    Serial.print(";");
    Serial.print(0);   //arbitrary user data here
    Serial.print(";0");
    Serial.println(13,DEC);
#endif

#if (SERIAL_MODE & SERIAL_MODE_DEBUG)
    Serial.print("Voltage");
    Serial.print(voltage,2);
    Serial.print(" Current");
    Serial.print(current,1);
    Serial.print(" Power");
    Serial.print(power,0);
    Serial.print(" PAS_On");
    Serial.print(pas_on_time);
    Serial.print(" PAS_Off");
    Serial.print(pas_off_time);
    Serial.print(" PAS_factor");
    Serial.print((float)pas_on_time/pas_off_time);
    Serial.print(" Speed");
    Serial.print(spd);
    Serial.print(" Brake");
    Serial.print(brake_stat);
    Serial.print(" Poti");
    Serial.print(poti_stat);
    Serial.print(" Throttle");
    Serial.print(throttle_stat);
/*
    Serial.print(" TEMP");
    Serial.print(temperature);
    Serial.print(" ALTI");
    Serial.print(altitude);
    Serial.print("/");
    Serial.print(altitude_start);
*/
    //now: data for Arduino Pedelec Configurator
    //0:voltage 1:current 2:pasfactor*100 3:option-pin 4:poti 5:throttle 6: brake
    Serial.print("---raw---");
    Serial.print(analogRead(voltage_in));Serial.print(";");
    Serial.print(analogRead(current_in));Serial.print(";");
    Serial.print(((int)(100*(double)pas_on_time/(double)pas_off_time)));Serial.print(";");
    Serial.print(analogRead(option_pin));Serial.print(";");
    Serial.print(analogRead(poti_in));Serial.print(";");
    Serial.print(analogRead(throttle_in));Serial.print(";");
    Serial.println(digitalRead(brake_in));
   
#endif

#if (SERIAL_MODE & SERIAL_MODE_MMC)
    Serial.print((int)(voltage_display*10));
    Serial.print("\t");
    Serial.print((int)(current*1000));
    Serial.print("\t");
    Serial.print((int)(spd*10));
    Serial.print("\t");
    Serial.print((long)(mah*3600));
    Serial.print("\t");
    Serial.print((int)0); //naximum current
    Serial.print("\t");
    Serial.print((long)(km*1000/wheel_circumference)); //distance
    Serial.print("\t");
    Serial.println((int)0); //profile
#endif
}

void set_profile()
{
  if (current_profile==0)
  {
  ptr_startingaid_speed = &startingaid_speed;
  ptr_spd_max1= &spd_max1;                 
  ptr_spd_max2=&spd_max2;               
  ptr_power_max=&power_max;                 
  ptr_power_poti_max=&power_poti_max;            
  ptr_capacity = &capacity; 
  } 
  else
  {
  ptr_startingaid_speed = &startingaid_speed_2;
  ptr_spd_max1= &spd_max1_2;                 
  ptr_spd_max2=&spd_max2_2;               
  ptr_power_max=&power_max_2;                 
  ptr_power_poti_max=&power_poti_max_2;            
  ptr_capacity = &capacity_2;  
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
