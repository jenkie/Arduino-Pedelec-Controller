
/*
Arduino Pedelec "Forumscontroller" for Hardware 1.2 and Arduino 1.0
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


What does it to:
*limit maximum electric power to 500 W (which means "250 W Nennleistung" :P)
*limit maximum speed
*starting aid up to 6 km/h
*pedaling-enabled, true-power-control throttle
*pedaling-enabled, pid-controlled motor-power which can be set by a potentiometer
*display: Nokia 5510
*sends Amarino-compatible data over bluetooth

EEPROMAnything is taken from here: http://www.arduino.cc/playground/Code/EEPROMWriteAnything

*/
#include "EEPROM.h"          //
#include "EEPROMAnything.h"  //to enable data storage when powered off
#include "PID_v1_nano.h"
#include <Wire.h>
#include "BMP085.h"          //library for altitude and temperature measurement using http://www.watterott.com/de/Breakout-Board-mit-dem-BMP085-absoluten-Drucksensor     
BMP085 bmp;

#include "PCD8544_nano.h"                    //for Nokia Display
static PCD8544 lcd;                          //for Nokia Display
//#include "LiquidCrystalDogm.h"             //for 4bit (e.g. EA-DOGM) Display
//LiquidCrystal lcd(13, 12, 11, 10, 9, 8);   //for 4bit (e.g. EA-DOGM) Display

struct savings   //add variables if you want to store additional values to the eeprom
{
    float voltage;
    float capacity;
    float kilometers;
};
savings variable = {0.0,0.0,0.0}; //variable stores last voltage and capacity read from EEPROM
static byte glyph1[] = {0x0b, 0xfc, 0x4e, 0xac, 0x0b}; //symbol for wh/km part 1
static byte glyph2[] = {0xc8, 0x2f, 0x6a, 0x2e, 0xc8}; //symbol for wh/km part 2
static byte glyph3[] = {0x44, 0x28, 0xfe, 0x6c, 0x28}; //bluetooth-symbol       check this out: http://www.carlos-rodrigues.com/projects/pcd8544/

//Config Options-----------------------------------------------------------------------------------------------------
const int fet_out = A0;              //FET: Pull high to switch off
const int voltage_in = A1;           //Voltage read-Pin
const int option_in = A2;            //analog option
const int current_in = A3;           //Current read-Pin
const int poti_in = A6;              //PAS Speed-Poti-Pin
const int throttle_in = A7;          // Throttle read-Pin

const int pas_in = 2;                //PAS Sensor read-Pin
const int wheel_in = 3;              //Speed read-Pin
const int brake_in = 4;              //Brake-In-Pin
const int switch_thr = 5;            //Throttle-Switch read-Pin
const int throttle_out = 6;          //Throttle out-Pin
const int bluetooth_pin = 7;         //Bluetooth-Supply
const int switch_disp = 8;           //Display switch

const int pas_tolerance=1;           //0... increase to make pas sensor slower but more tolerant against speed changes
const int throttle_offset=50;        //Offset for throttle output where Motor starts to spin (0..255 = 0..5V)
const int throttle_max=200;          //Maximum input value for motor driver (0..255 = 0..5V)
const boolean startingaidenable = true; //enable starting aid?
const float vcutoff=33.0;            //cutoff voltage in V;
const float wheel_circumference = 2.202; //wheel circumference in m
const int spd_max1=27.0;             //speed cutoff start in Km/h
const int spd_max2=30.0;             //speed cutoff stop (0W) in Km/h
const int power_max=500;             //Maximum power in W
double pid_p=0.0;              //pid p-value, default: 0.0
double pid_i=2.0;              //pid i-value, default: 2.0
double pid_p_throttle=0.05;    //pid p-value for throttle mode
double pid_i_throttle=2.5;     //pid i-value for throttle mode


//Variable-Declarations-----------------------------------------------------------------------------------------------
double pid_out,pid_set;        //pid output, pid set value
int throttle_stat = 0;         //Throttle reading
int throttle_write=0;          //Throttle write value
float poti_stat;               //PAS-Poti setting
volatile int pas_on_time = 0;  //High-Time of PAS-Sensor-Signal (needed to determine pedaling direction)
volatile int pas_off_time = 0; //Low-Time of PAS-Sensor-Signal  (needed to determine pedaling direction)
volatile int pas_failtime = 0; //how many subsequent "wrong" PAS values?
double power_set = 0;          //Set Power
volatile int cad=0;            //Cadence
int looptime=0;                //Loop Time in milliseconds (for testing)
float battery_percent=0.0;     //battery capacity
float current = 0.0;           //measured battery current
float voltage = 0.0;           //measured battery voltage
float voltage_1s,voltage_2s = 0.0; //Voltage history 1s and 2s before "now"
float voltage_display = 0.0;   //averaged voltage
float current_display = 0.0;   //averaged current
double power=0.0;              //calculated power
float factor_speed=1.0;        //factor controling the speed
float factor_volt=1.0;         //factor controling voltage cutoff
float wh=0.0;                  //watthours drawn from battery
float temperature = 0.0;       //temperature
float altitude = 0.0;          //altitude
float altitude_start=0.0;      //altitude at start
volatile float km=0.0;         //trip-km
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
PID myPID(&power, &pid_out,&pid_set,pid_p,pid_i,0, DIRECT);

// Forward declarations for compatibility with new gcc versions
void display_nokia_setup();
void display_nokia_update();
void pas_change();
void speed_change();
void send_android_data();

//Setup---------------------------------------------------------------------------------------------------------------------
void setup()
{
    display_nokia_setup();    //for Nokia Display
    //lcd.begin(8, 2);        //for 4bit (e.g. EA-DOGM) Display
    Serial.begin(115200);     //bluetooth-module requires 115200
    pinMode(pas_in, INPUT);
    pinMode(wheel_in, INPUT);
    pinMode(switch_thr, INPUT);
    pinMode(switch_disp, INPUT);
    pinMode(brake_in, INPUT);
    pinMode(option_in,INPUT);
    pinMode(fet_out,OUTPUT);
    pinMode(bluetooth_pin,OUTPUT);
    digitalWrite(fet_out, LOW);           // turn on whole system on (write high to fet_out if you want to power off)
    digitalWrite(bluetooth_pin, LOW);     // turn bluetooth off
    digitalWrite(brake_in, HIGH);         // turn on pullup resistors on brake
    digitalWrite(switch_thr, HIGH);       // turn on pullup resistors on throttle-switch
    digitalWrite(switch_disp, HIGH);      // turn on pullup resistors on display-switch
    digitalWrite(wheel_in, HIGH);         // turn on pullup resistors on wheel-sensor
    digitalWrite(pas_in, HIGH);           // turn on pullup resistors on pas-sensor
    attachInterrupt(0, pas_change, CHANGE); //attach interrupt for PAS-Sensor
    attachInterrupt(1, speed_change, RISING); //attach interrupt for Wheel-Sensor
    EEPROM_readAnything(0,variable);      //read stored variables
    myPID.SetMode(AUTOMATIC);             //initialize pid
    myPID.SetOutputLimits(0,1023);        //initialize pid
    Wire.begin();                         //initialize i2c-bus
    bmp.begin();                          //initialize barometric altitude sensor
    altitude_start=bmp.readAltitude();    //initialize barometric altitude sensor
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
    current = analogRead(current_in)*0.0296217305;        //check with multimeter and change if needed!
    current = constrain(current,0,30);
    voltage_display = 0.99*voltage_display + 0.01*voltage; //averaged voltage for display
    current_display = 0.99*current_display + 0.01*current; //averaged voltage for display
    power=current*voltage;

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
    if (((millis()-last_pas_event)>500)||((millis()-last_pas_event)>2*pas_on_time)||(pas_failtime>pas_tolerance))
        {pedaling = false;}                               //we are not pedaling anymore, if pas did not change for > 0,5 s
    cad=cad*pedaling;

    if ((millis()-last_wheel_time)>3000)               //wheel did not spin for 3 seconds --> speed is zero
        spd=0;


//Power control-------------------------------------------------------------------------------------------------------------
    if (throttle_stat > poti_stat)                  //throttle mode: throttle sets power with "agressive" p and i parameters
    {
        myPID.SetTunings(pid_p_throttle,pid_i_throttle,0);
        power_set=throttle_stat/1023.0*power_max;
    }
    else                                            //poti mode: poti sets power with "soft" p and i parameters
    {
        myPID.SetTunings(pid_p,pid_i,0);
        power_set=poti_stat/1023.0*power_max;
    }

//Speed cutoff-------------------------------------------------------------------------------------------------------------

    if (pedaling==true)
        {factor_speed=constrain(1-(spd-spd_max1)/(spd_max2-spd_max1),0,1);} //linear decrease of maximum power for speeds higher than spd_max1
    else
    {
        if (startingaidenable==true)                         //starting aid activated
            {factor_speed=constrain((6.0-spd)/2,0,1);}
        else
            {factor_speed=0;}                                    //no starting aid
    }

    if (power_set>power_max*factor_speed)
        {power_set=power_max*factor_speed;}                  //Maximum allowed power including Speed-Cutoff
    if (((pedaling==false)&&(throttle_stat<5))||(brake_stat==0))  //power_set is set to -60W when you stop pedaling or brake (this is the pid-input)
        {power_set=-60;}

    pid_set=power_set;
    myPID.Compute();                                      //this computes the needed drive voltage for the motor controller to maintain the "power_set" based on the current "power" measurment

//Voltage cutoff----------------------------------------------------------------------------------------------------------
    if (voltage<vcutoff)
        {factor_volt=factor_volt*0.9997;}
    else
        {factor_volt=factor_volt*0.9997+0.0003;}

//Throttle output-------------------------------------------------------------------------------------------------------

    throttle_write=map(pid_out*brake_stat*factor_volt,0,1023,throttle_offset,throttle_max);
    if ((pedaling==false)&&(throttle_stat<5))
        {throttle_write=0;}
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
            digitalWrite(fet_out,HIGH);
        }
    }
    else
    {
        if (switch_disp_last==true)
        {
            switch_disp_last=false;
            digitalWrite(bluetooth_pin, !digitalRead(bluetooth_pin));
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

//Show something on the LCD----------------------//use this subroutine to place any functions which should happen only once a second
    if (millis()-last_writetime > 1000)              //don't do this more than once a second
    {
        voltage_2s=voltage_1s;                           //update voltage history
        voltage_1s=voltage;                              //update voltage history

        temperature = bmp.readTemperature();
        altitude = bmp.readAltitude()-altitude_start;

//-----battery percent calculation start, valid for turnigy 5000mAh-LiPo (polynomial fit to discharge curve at 150W)
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
//-----battery percent calculation end
        wh=wh+current*(millis()-last_writetime)/3600000.0*voltage;  //watthours calculation
        display_nokia_update();                                     //for Nokia display
//display_4bit_update();                                    //for 4bit (e.g. EA-DOGM) Display
        last_writetime=millis();
        send_android_data();                                        //sends data over bluetooth to amarino - also visible at the serial monitor
    }
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
//if (pas_on_time>pas_off_time)
    {
        pedaling=true;
        pas_failtime=0;
    }
//Serial.print(pas_off_time);           //good for debug
//Serial.print(" ");
//Serial.print(pas_on_time);
//Serial.print(" ");
//Serial.println(brake_stat);
}

void speed_change()    //Wheel Sensor Change------------------------------------------------------------------------------------------------------------------
{
//Speed and Km
    if (last_wheel_time>(millis()-50)) return;                         //debouncing reed-sensor
    spd = (spd+3600*wheel_circumference/((millis()-last_wheel_time)))/2;  //a bit of averaging for smoother speed-cutoff
    if (spd<100)
        {km=km+wheel_circumference/1000.0;}
    else
        {spd=0;}
    last_wheel_time=millis();
}


void send_android_data()  //send adroid data----------------------------------------------------------
{
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
    Serial.print(altitude,2);
    Serial.print(";");
    Serial.print(0);
    Serial.print(ack);
}

void display_nokia_setup()    //first time setup of nokia display------------------------------------------------------------------------------------------------------------------
{
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
}

void display_4bit_update()  //update 4bit display------------------------------------------------------------------------------------------------------------------
{
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
}

void display_nokia_update()  //update nokia display------------------------------------------------------------------------------------------------------------------
{
    lcd.setCursor(0,0);
    lcd.print(voltage_display,1);

    lcd.setCursor(6,0);
    if (current_display<9.5)
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
//lcd.print(millis()/60000.0,1);
//lcd.print(" Minuten");
    lcd.print(temperature,1);
    lcd.print(" ");
    lcd.print((int)altitude);
    lcd.print(" ");
    lcd.print(analogRead(option_in));
    lcd.print("    ");
    lcd.setCursor(13,5);
    if (digitalRead(bluetooth_pin)==1)
        {lcd.write(2);}
    else
        {lcd.print(" ");}
}

