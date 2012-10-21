/*
Arduino Pedelec "Forumscontroller" for Hardware 1.1-1.3
written by jenkie / pedelecforum.de
Copyright (C) 2012

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
#include "display.h"         //display output functions
#include "display_backlight.h"  // LCD display backlight support
#include "EEPROM.h"
#include "EEPROMAnything.h"  //to enable data storage when powered off
#include "PID_v1_nano.h"

#ifdef SUPPORT_BMP085
    #include <Wire.h>
    #include "BMP085.h"          //library for altitude and temperature measurement using http://www.watterott.com/de/Breakout-Board-mit-dem-BMP085-absoluten-Drucksensor     
    BMP085 bmp;
#endif

#if defined(SUPPORT_POTI) && defined(SUPPORT_SOFT_POTI)
#error You either have poti or soft-poti support. Disable one of them.
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_J_LCD)
#include <SoftwareSerial.h>      //for Kingmeter J-LCD
#endif

struct savings   //add variables if you want to store additional values to the eeprom
{
    float voltage;
    float capacity;
    float kilometers;
};
savings variable = {0.0,0.0,0.0}; //variable stores last voltage and capacity read from EEPROM

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
#if HARDWARE_REV == 3
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
float poti_stat = 0.0;         //Poti reading
volatile int pas_on_time = 0;  //High-Time of PAS-Sensor-Signal (needed to determine pedaling direction)
volatile int pas_off_time = 0; //Low-Time of PAS-Sensor-Signal  (needed to determine pedaling direction)
volatile int pas_failtime = 0; //how many subsequent "wrong" PAS values?
volatile int cad=0;            //Cadence
int looptime=0;                //Loop Time in milliseconds (for testing)
float battery_percent=0.0;     //battery capacity
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
float wh=0.0;                  //watthours drawn from battery
float temperature = 0.0;       //temperature
float altitude = 0.0;          //altitude
float altitude_start=0.0;      //altitude at start
float last_altitude;           //height 
float slope = 0.0;             //current slope
volatile float km=0.0;         //trip-km
volatile float spd=0.0;        //speed
float range = 0.0;             //expected range
unsigned long switch_disp_pressed;       //time when display switch was pressed down (to decide if long or short press)
boolean switch_disp_last=false; //was display switch already pressed since last loop run?
unsigned long last_writetime = millis();  //last time display has been refreshed
volatile unsigned long last_wheel_time = millis(); //last time of wheel sensor change 0->1
volatile unsigned long wheel_time = 0;  //time for one revolution of the wheel
volatile unsigned long last_pas_event = millis();  //last change-time of PAS sensor status
volatile boolean pedaling = false;  //pedaling? (in forward direction!)
boolean firstrun = true;  //first run of loop?
boolean variables_saved = false; //has everything been saved after Switch-Off detected?
boolean brake_stat = true; //brake activated?
PID myPID(&power, &pid_out,&pid_set,pid_p,pid_i,0, DIRECT);
unsigned int idle_shutdown_count = 0;
unsigned long idle_shutdown_last_wheel_time = millis();
double torque=0.0;           //cyclist's torque
double power_human=0.0;      //cyclist's power
#ifdef SUPPORT_XCELL_RT
volatile int torquevalues[10]={0,0,0,0,0,0,0,0,0,0}; //stores the 10 torque values per pedal roundtrip
volatile byte torqueindex=0;        //index to write next torque value
volatile boolean readtorque=false;  //true if pas-interrupt received -> read torque in main loop. unfortunately analogRead gives wrong values inside the PAS-interrupt-routine
#endif




// Forward declarations for compatibility with new gcc versions
void pas_change();
void speed_change();
void send_serial_data();

//Setup---------------------------------------------------------------------------------------------------------------------
void setup()
{
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
    pinMode(brake_in, INPUT);
    pinMode(option_pin,INPUT);
#if HARDWARE_REV >= 2
    pinMode(fet_out,OUTPUT);
    pinMode(bluetooth_pin,OUTPUT);
    digitalWrite(bluetooth_pin, LOW);     // turn bluetooth off
    digitalWrite(fet_out, LOW);           // turn on whole system on (write high to fet_out if you want to power off)
#endif
    digitalWrite(brake_in, HIGH);         // turn on pullup resistors on brake
    digitalWrite(switch_thr, HIGH);       // turn on pullup resistors on throttle-switch
    digitalWrite(switch_disp, HIGH);      // turn on pullup resistors on display-switch
    digitalWrite(wheel_in, HIGH);         // turn on pullup resistors on wheel-sensor
    digitalWrite(pas_in, HIGH);           // turn on pullup resistors on pas-sensor
#ifdef SUPPORT_DISPLAY_BACKLIGHT
    pinMode(display_backlight_pin, OUTPUT);
#endif
    display_show_important_info(msg_welcome, 1);
#ifdef SUPPORT_PAS
    attachInterrupt(0, pas_change, CHANGE); //attach interrupt for PAS-Sensor
#endif
    attachInterrupt(1, speed_change, RISING); //attach interrupt for Wheel-Sensor
    EEPROM_readAnything(0,variable);      //read stored variables
    myPID.SetMode(AUTOMATIC);             //initialize pid
    myPID.SetOutputLimits(0,1023);        //initialize pid
#ifdef SUPPORT_BMP085
    Wire.begin();                         //initialize i2c-bus
    bmp.begin();                          //initialize barometric altitude sensor
    altitude_start=bmp.readAltitude();    //initialize barometric altitude sensor
#endif
#ifndef SUPPORT_PAS
    pedaling=true;
#endif
}

void loop()
{
    looptime=millis();
//Readings-----------------------------------------------------------------------------------------------------------------
#ifdef SUPPORT_POTI
    poti_stat=analogRead(poti_in);                       // 0...1023
#endif

#if (DISPLAY_TYPE & DISPLAY_TYPE_J_LCD)
    display_update();
#endif

#ifdef SUPPORT_THROTTLE
    throttle_stat = constrain(map(analogRead(throttle_in),throttle_offset,throttle_max,0,1023),0,1023);   // 0...1023
#endif
    brake_stat = digitalRead(brake_in);
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
    torquevalues[torqueindex]=analogRead(option_pin)-torque_offset;
    torqueindex++;
    torque=0.0; 
    for (int i = 0; i < 10; i++) 
      {
        torque+=torquevalues[i];
      }
    if (torqueindex==10)
      {torqueindex=0;}
    readtorque=false;
    torque=abs((torque)*0.049);
    power_human=0.104719755*cad*torque;   //power=2*pi*cadence*torque/60s
}
#endif

//Check if Battery was charged since last power down-----------------------------------------------------------------------
    if (firstrun==true)
    {
        if (variable.voltage>(voltage - 2))                //charging detected if voltage is 2V higher than last stored voltage
        {
            wh=variable.capacity;
            km=variable.kilometers;
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
#if CONTROL_MODE == CONTROL_MODE_TORQUE                      //human power control mode
#ifdef SUPPORT_XCELL_RT
    power_throttle = throttle_stat / 1023.0 * power_max;     //power currently set by throttle
    power_poti = poti_stat/200.0 * power_human;
    if ((power_throttle) > (power_poti))                     //IF power set by throttle IS GREATER THAN power set by poti
    {
        myPID.SetTunings(pid_p_throttle,pid_i_throttle,0);   //THEN throttle mode: throttle sets power with "agressive" p and i parameters        power_set=throttle_stat/1023.0*power_max;
        power_set = power_throttle;
    }
    else                                                     //ELSE poti mode: poti sets power with "soft" p and i parameters
    {
        myPID.SetTunings(pid_p,pid_i,0);
        power_set = power_poti; 
    }
#endif
#endif
#if CONTROL_MODE == CONTROL_MODE_NORMAL             //power-control-mode
    power_throttle = throttle_stat / 1023.0 * power_max;     //power currently set by throttle
    power_poti = poti_stat / 1023.0 * power_poti_max;        //power currently set by poti
    if ((power_throttle) > (power_poti))                     //IF power set by throttle IS GREATER THAN power set by poti
    {
        myPID.SetTunings(pid_p_throttle,pid_i_throttle,0);   //THEN throttle mode: throttle sets power with "agressive" p and i parameters        power_set=throttle_stat/1023.0*power_max;
        power_set = power_throttle;
    }
    else                                                     //ELSE poti mode: poti sets power with "soft" p and i parameters
    {
        myPID.SetTunings(pid_p,pid_i,0);
        power_set = power_poti; 
    }
#endif
#if CONTROL_MODE == CONTROL_MODE_LIMIT_WH_PER_KM    //wh/km control mode
    power_throttle = throttle_stat / 1023.0 * power_max;     //power currently set by throttle
    power_poti = poti_stat / 1023.0 * whkm_max * spd;        //power currently set by poti in relation to speed and maximum wattage per km    
    if ((power_throttle) > (power_poti))                     //IF power set by throttle IS GREATER THAN power set by poti 
    {
        myPID.SetTunings(pid_p_throttle,pid_i_throttle,0);   //THEN throttle mode: throttle sets power with "agressive" p and i parameters
        power_set = power_throttle;
    }
    else                                                     //ELSE poti mode: poti sets power with "soft" p and i parameters
    {
        myPID.SetTunings(pid_p,pid_i,0);
        power_set = power_poti;
    }
#endif

//Speed cutoff-------------------------------------------------------------------------------------------------------------

    if (pedaling==true)
        {factor_speed=constrain(1-(spd-spd_max1)/(spd_max2-spd_max1),0,1);} //linear decrease of maximum power for speeds higher than spd_max1
    else
    {
        if (startingaidenable==true)                         //starting aid activated
            {factor_speed=constrain((startingaid_speed-spd)/2,0,1);}
        else
            {factor_speed=0;}                                    //no starting aid
    }

    if (power_set>power_max*factor_speed)
        {power_set=power_max*factor_speed;}                  //Maximum allowed power including Speed-Cutoff
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

    throttle_write=map(pid_out*brake_stat*factor_volt,0,1023,motor_offset,motor_max);
    if ((pedaling==false)&&(throttle_stat<5))
        {throttle_write=0;}
    analogWrite(throttle_out,throttle_write);

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA_4PIN)
    if (digitalRead(switch_disp_2)==0)  //switch backlight on for one minute
    {
#ifdef SUPPORT_DISPLAY_BACKLIGHT
        enable_custom_backlight(60000);
#endif
    }
#endif

    if (digitalRead(switch_disp)==0)  //switch on/off bluetooth if switch is pressed
    {
        if (switch_disp_last==false)
        {
            switch_disp_last=true;
            switch_disp_pressed=millis();
        }
        else if ((millis()-switch_disp_pressed)>1000)
        {
#if HARDWARE_REV >=2
#ifdef SUPPORT_SOFT_POTI
            // Work around missing shutdown state machine
            // Otherwise it might show "Tempomat set".
            poti_stat = throttle_stat;
#endif
            display_show_important_info(msg_shutdown, 60);
            digitalWrite(fet_out,HIGH);
#endif
        }
    }
    else
    {
        if (switch_disp_last==true)
        {
            switch_disp_last=false;
#ifdef SUPPORT_SOFT_POTI
            // Set soft poti if throttle value changed
            if (poti_stat != throttle_stat)
            {
                poti_stat = throttle_stat;
                if (poti_stat == 0)
                    display_show_important_info("Tempomat reset", 0);
                else
                    display_show_important_info("Tempomat set", 0);
            }
#endif
#if HARDWARE_REV >=2
            digitalWrite(bluetooth_pin, !digitalRead(bluetooth_pin));   //not available in 1.1!
#endif
        }
    }

//Save capacity to EEPROM
    if ((voltage<20.0)&&(variables_saved==false))    //save to EEPROM when Switch-Off detected
    {
        variable.voltage=voltage_2s;   //save the voltage value 2 seconds before switch-off-detection
        variable.capacity=wh;          //save watthours drawn from battery
        variable.kilometers=km;        //save trip kilometers
        EEPROM_writeAnything(0,variable);
        variables_saved=true;
    }

#ifdef SUPPORT_DISPLAY_BACKLIGHT
    handle_backlight();
#endif

//slow loop start----------------------//use this subroutine to place any functions which should happen only once a second
    if (millis()-last_writetime > 1000)              //don't do this more than once a second
    {
        voltage_2s=voltage_1s;                           //update voltage history
        voltage_1s=voltage;                              //update voltage history

#ifdef SUPPORT_BMP085
        temperature = bmp.readTemperature();
        altitude = bmp.readAltitude()-altitude_start;
#endif

/* //-----battery percent calculation start, valid for turnigy 5000mAh-LiPo (polynomial fit to discharge curve at 150W)
        if (voltage_display>38.6)
            {battery_percent=-15.92628+0.71422*voltage_display-0.007398*pow(voltage_display,2);}
        else
        {
            if (voltage_display>36.76)
                {battery_percent=5414.20057-431.39368*voltage_display+11.449212*pow(voltage_display,2)-0.1012069*pow(voltage_display,3);}
            else
                {battery_percent=0.0025*pow(voltage_display-33,3);}
        }
        battery_percent=constrain(battery_percent*100,0,100);
//-----battery percent calculation end */

        battery_percent = constrain((1-wh/capacity)*100,0,100);     //battery percent calculation from battery capacity. For voltage-based calculation see above
        range=constrain(capacity/wh*km-km,0.0,200.0);               //range calculation from battery capacity
        wh=wh+current*(millis()-last_writetime)/3600000.0*voltage;  //watthours calculation

#if !(DISPLAY_TYPE & DISPLAY_TYPE_J_LCD)
        display_update();
#endif
        
        send_serial_data();                                        //sends data over serial port depending on SERIAL_MODE

#if HARDWARE_REV >= 2
// Idle shutdown
        if (last_wheel_time != idle_shutdown_last_wheel_time)
        {
            idle_shutdown_last_wheel_time = last_wheel_time;
            idle_shutdown_count = 0;
        } else
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
            digitalWrite(fet_out,HIGH);
        }
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
#ifdef SUPPORT_XCELL_RT
    cad=6000/(pas_on_time+pas_off_time);
#else
    cad=12000/(pas_on_time+pas_off_time);
#endif
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
    wheel_time=millis()-last_wheel_time;
    spd = (spd+3600*wheel_circumference/wheel_time)/2;  //a bit of averaging for smoother speed-cutoff
    if (spd<100)
        {km=km+wheel_circumference/1000.0;}
    else
        {spd=0;}
#ifdef SUPPORT_BMP085
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
    char ack=19;
    char startFlag=18; // used to communicate with Android (leads each message to Amarino)
    Serial.print(startFlag);
    Serial.print(voltage,2);
    Serial.print(";");
    Serial.print(current,1);
    Serial.print(";");
    Serial.print(power,1);
    Serial.print(";");
    Serial.print(spd,2);
    Serial.print(";");
    Serial.print(km,3);
    Serial.print(";");
    Serial.print(cad);
    Serial.print(";");
    Serial.print(wh);
    Serial.print(";");
    Serial.print(torque,1);
    Serial.print(";");
    Serial.print(power_human,1);
    Serial.print(ack);
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
    Serial.println(throttle_stat); 
#endif
}




