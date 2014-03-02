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



void setup()
{
    Serial.begin(9600); //set the current speed of your bluetooth module here, usually 9600

    delay(1000);
    pinMode(7, OUTPUT);
    digitalWrite(7,HIGH);
    delay(5000);
    Serial.print("AT");
    delay(5000);
    Serial.print("AT+VERSION");
    delay(5000);
    Serial.print("AT+PIN1234"); // Set pin to 1234 or anything of your choice
    delay(5000);
    Serial.print("AT+NAMEArduinoPedelec"); //use your own name if you want
    delay(5000);
    Serial.print("AT+BAUD8"); // Set baudrate to 115200, do not change
    delay(5000);
    digitalWrite(7,LOW);
    delay(10000);
    digitalWrite(7,HIGH);
}

void loop()
{

}
