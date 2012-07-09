
/*
Arduino Pedelec "Forumscontroller" test and EEPROM-initialization program for Hardware 1.1-1.3
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

EEPROMAnything is taken from here: http://www.arduino.cc/playground/Code/EEPROMWriteAnything
*/

#define DISPLAY_TYPE 0      //display type 0:Nokia5110 5-pin-mode 1: Nokia5110 4-pin-mode (SCE pin tied to GND) 2: 16x2 LCD 4bit-mode
#define HARDWARE_REV 2      //place your hardware revision (1-3) here: x means hardware-revision 1.x


#include "EEPROM.h"          //
#include "EEPROMAnything.h"  //to enable data storage when powered off
#if DISPLAY_TYPE <= 1
#include "PCD8544_nano.h"                    //for Nokia Display
static PCD8544 lcd;                          //for Nokia Display
#endif
#if DISPLAY_TYPE == 2
#include "LiquidCrystalDogm.h"             //for 4bit (e.g. EA-DOGM) Display
LiquidCrystal lcd(13, 12, 11, 10, 9, 8);   //for 4bit (e.g. EA-DOGM) Display
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
#if DISPLAY_TYPE == 1
const int switch_disp_2 = 13;        //second Display switch with Nokia-Display in 4-pin-mode
#endif

//Config Options-----------------------------------------------------------------------------------------------------
const int pas_tolerance=1;           //0... increase to make pas sensor slower but more tolerant against speed changes
const int throttle_offset=50;        //Offset for throttle output where Motor starts to spin (0..255 = 0..5V)
const int throttle_max=200;          //Maximum input value for motor driver (0..255 = 0..5V)
const boolean startingaidenable = true; //enable starting aid?
const float vcutoff=33.0;            //cutoff voltage in V;
const float wheel_circumference = 2.202; //wheel circumference in m
const int spd_max1=27.0;             //speed cutoff start in Km/h
const int spd_max2=30.0;             //speed cutoff stop (0W) in Km/h
const int power_max=500;             //Maximum power in W
const double capacity = 166.0;       //battery capacity in watthours for range calculation
double pid_p=0.0;              //pid p-value, default: 0.0
double pid_i=2.0;              //pid i-value, default: 2.0
double pid_p_throttle=0.05;    //pid p-value for throttle mode
double pid_i_throttle=2.5;     //pid i-value for throttle mode

//Variable-Declarations-----------------------------------------------------------------------------------------------
int throttle_stat = 0;         //Throttle reading
int throttle_write=0;          //Throttle write value
float poti_stat;               //PAS-Poti setting
volatile int pas_on_time = 0;  //High-Time of PAS-Sensor-Signal (needed to determine pedaling direction)
volatile int pas_off_time = 0; //Low-Time of PAS-Sensor-Signal  (needed to determine pedaling direction)
volatile int pas_failtime = 0; //how many subsequent "wrong" PAS values?
volatile int cad=0;            //Cadence
int looptime=0;                //Loop Time in milliseconds (for testing)
float current = 0.0;           //measured battery current
float voltage = 0.0;           //measured battery voltage
double power=0.0;              //calculated power
volatile float spd=0.0;        //speed
unsigned long switch_disp_pressed;       //time when display switch was pressed down (to decide if long or short press)
boolean switch_disp_last=false; //was display switch already pressed since last loop run?
unsigned long last_writetime = millis();  //last time display has been refreshed
volatile unsigned long last_wheel_time = millis(); //last time of wheel sensor change 0->1
volatile unsigned long last_pas_event = millis();  //last change-time of PAS sensor status
volatile boolean pedaling = false;  //pedaling? (in forward direction!)
boolean firstrun = true;  //first run of loop?
boolean variables_saved = false; //has everything been saved after Switch-Off detected?
boolean brake_stat = true; //brake activated?

// Forward declarations for compatibility with new gcc versions
void display_nokia_setup();
void pas_change();
void speed_change();

//Setup---------------------------------------------------------------------------------------------------------------------
void setup()
{
#if DISPLAY_TYPE == 0
    pinMode(13,OUTPUT);
    digitalWrite(13,LOW);
#endif
#if DISPLAY_TYPE <= 1
    display_nokia_setup();    //for Nokia Display
#endif
#if DISPLAY_TYPE == 2
    lcd.begin(16, 2);        //for 4bit (e.g. EA-DOGM) Display
#endif
    Serial.begin(115200);     //bluetooth-module requires 115200
    pinMode(pas_in, INPUT);
    pinMode(wheel_in, INPUT);
    pinMode(switch_thr, INPUT);
    pinMode(switch_disp, INPUT);
#if DISPLAY_TYPE == 1
    pinMode(switch_disp_2, INPUT);
    digitalWrite(switch_disp_2, HIGH);    // turn on pullup resistors on display-switch 2
#endif
    pinMode(brake_in, INPUT);
    pinMode(option_pin,OUTPUT);
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
    attachInterrupt(0, pas_change, CHANGE); //attach interrupt for PAS-Sensor
    attachInterrupt(1, speed_change, RISING); //attach interrupt for Wheel-Sensor
    EEPROM_readAnything(0,variable);      //read stored variables
}

void loop()
{
    looptime=millis();
//Readings-----------------------------------------------------------------------------------------------------------------
    poti_stat=analogRead(poti_in);                       // 0...1023
    throttle_stat = constrain(map(analogRead(throttle_in),196,832,0,1023),0,1023);   // 0...1023
    brake_stat = digitalRead(brake_in);
//voltage, current, power
    voltage = analogRead(voltage_in)*0.05859375;          //check with multimeter and change if needed!
#if HARDWARE_REV <= 2
    current = analogRead(current_in)*0.0296217305; //check with multimeter and change if needed!
    current = constrain(current,0,30);
#endif
#if HARDWARE_REV >= 3
    current = (analogRead(current_in)-512)*0.0740543263;        //check with multimeter and change if needed!
    current = constrain(current,-30,30);
#endif
    power=current*voltage;

//This initializes the EEPROM-Values to 0.0-----------------------------------------------------------------------
    if (firstrun==true)
    {
        variable.voltage=0.0;   
        variable.capacity=0.0;          
        variable.kilometers=0.0;        
        EEPROM_writeAnything(0,variable);
        variables_saved=true;                             //do not write new data to eeprom when on USB Power
    }
    firstrun=false;                                     //first loop run done (ok, up to this line :))
                   
//Throttle output-------------------------------------------------------------------------------------------------------

    throttle_write=map(throttle_stat,0,1023,throttle_offset,throttle_max); //be careful if motor connected!
    analogWrite(throttle_out,throttle_write);

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
            digitalWrite(fet_out,HIGH);
#endif
        }
    }
    else
    {
        if (switch_disp_last==true)
        {
            switch_disp_last=false;
#if HARDWARE_REV >=2
            digitalWrite(bluetooth_pin, !digitalRead(bluetooth_pin));   //not available in 1.1!
#endif
        }
    }

//Show something on the LCD and Serial Port
    if (millis()-last_writetime > 500)             
    {
       digitalWrite(option_pin,!digitalRead(option_pin));  //switch lamp on and off
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
       lcd.setCursor(0,0);
       lcd.print("Hello World");
       last_writetime=millis();}  
    }


void pas_change()       //Are we pedaling? PAS Sensor Change------------------------------------------------------------------------------------------------------------------
{
    if (last_pas_event>(millis()-10)) return;
    if (digitalRead(pas_in)==true)
        {pas_off_time=millis()-last_pas_event;}
    else
        {pas_on_time=millis()-last_pas_event;}
    last_pas_event = millis();
    pas_failtime=pas_failtime+1;
    cad=12000/(pas_on_time+pas_off_time);
    if ((pas_on_time>1.2*pas_off_time)&&(pas_on_time<3*pas_off_time))   //when pedaling forward the pas_on_time is with my sensor approximately 2 times the pas_off_time......
    {
        pedaling=true;
        pas_failtime=0;
    }

}

void speed_change()    //Wheel Sensor Change------------------------------------------------------------------------------------------------------------------
{
//Speed and Km
    if (last_wheel_time>(millis()-50)) return;                         //debouncing reed-sensor
    spd = (spd+3600*wheel_circumference/((millis()-last_wheel_time)))/2;  //a bit of averaging for smoother speed-cutoff
    last_wheel_time=millis();
}

void display_nokia_setup()    //first time setup of nokia display------------------------------------------------------------------------------------------------------------------
{
#if DISPLAY_TYPE <= 1
    lcd.begin(84, 48);
#endif
}

