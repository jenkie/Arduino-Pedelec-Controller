/*
This program configures the bluetooth module with custom name and pin for
use with the Arduino Pedelec Controller

1st step: Connect Bluetooth module to Arduino Pedelec Controller
2nd step: Wait 1 minute after uploading the program (bluetooth module should
shut down and repower during this time), then you are ready.

Written by Jens Kießling (jenkie)

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#define HARDWARE_REV 21  //place your hardware revision here: 1-5 means hardware-revision 1.x, 2x means 2.x
int SerialSpeed=9600;    //set the current speed of your bluetooth module here, usually 9600    

#if HARDWARE_REV<20
HardwareSerial btSerial=Serial;
int btPin=7;
#else
HardwareSerial btSerial=Serial1;
int btPin=13;
#endif

void setup()
{
#if HARDWARE_REV>=20
    pinMode(38,OUTPUT);
    digitalWrite(38, 1);           // turn on whole system on
    delay(5000);
#endif
    btSerial.begin(SerialSpeed);   
    delay(1000);
    pinMode(btPin, OUTPUT);
    digitalWrite(btPin,HIGH);
    delay(5000);
    btSerial.print("AT");
    delay(1000);
    btSerial.print("AT+VERSION?");
    delay(1000);
    btSerial.print("AT+ORGL");
    delay(1000);
    btSerial.print("AT+RMAAD");
    delay(1000);
    btSerial.print("AT+ROLE=0");
    delay(1000);
    btSerial.print("AT+PSWD=1234"); // Set pin to 1234 or anything of your choice
    delay(5000);
    btSerial.print("AT+NAMEArduinoPedelec"); //use your own name if you want
    delay(5000);
    btSerial.print("AT+BAUD8"); // Set baudrate to 115200, do not change
    delay(5000);
    digitalWrite(btPin,LOW);
    delay(10000);
    digitalWrite(btPin,HIGH);
#if HARDWARE_REV>=20
    digitalWrite(38, 0);           // turn on whole system off
#endif

}

void loop()
{

}
