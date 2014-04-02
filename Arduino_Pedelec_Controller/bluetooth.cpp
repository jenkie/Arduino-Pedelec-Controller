/*
Bluetooth communication functions
Written by Jens Kießling / jenkie

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
*/

#include "config.h"
#include "bluetooth.h"
#include "globals.h"

extern boolean variables_saved;
char firstchar = 0;
const byte numberlength = 5;
char numberstring[numberlength];
byte num_i=0;
byte validcommand = 0;
byte i = 0;

byte com_i; //index of current command

struct serial_command {char mnemonic[3];};
serial_command serial_commands[] =
{
    {{"ps"}},     //0: poti stat, gets and sets poti stat
    {{"od"}},     //1: total kilometers (odo), gets and sets total kilometers
};
int n_commands = sizeof(serial_commands)/sizeof(serial_command); //number of commands that we have

void find_commands() //a command is always AA# or AA? where AA# sets # to variable AA and AA? returns value of AA
{
    validcommand = 0;
    for (i=0; i < n_commands ; i++)    //try int mnemonics
    {
        if (serial_commands[i].mnemonic[0] == firstchar && serial_commands[i].mnemonic[1] == inchar)
        {
            com_i = i;
            validcommand = 1;
            return;
        }
    }
    if (validcommand == 0)
    {
        //unrecognized command
        firstchar = 0;
    }
}

void next(char inchar)
{
    if (inchar == 10 || inchar == 13)  // LF, CR
    {
        if (validcommand && numberstring[0] != '\0')
        {
            // if command and number, write now!
            switch(com_i)
            {
                case 0:              //poti_stat
                    poti_stat = min(atoi(numberstring)*1023.0/power_poti_max,1023);
                    break;
                case 1:              //total kilometers
                    odo = atoi(numberstring)*1000.0/wheel_circumference;
                    variables_saved=false;
                    save_eeprom();
                    break;
            }
            Serial.print(serial_commands[com_i].mnemonic);
            Serial.println(MY_F("OK"));
        }
        validcommand = 0;
        firstchar = 0;
        for (num_i = 0; num_i < numberlength; num_i++)
        {
            numberstring[num_i] = '\0';
        }
        num_i = 0;
        return;
    }
    if (validcommand)
    {
        if (inchar == '?')
        {
            validcommand = 0;
            Serial.print(serial_commands[com_i].mnemonic);
            switch(com_i)
            {
                case 0:             //poti_stat
                    Serial.println(poti_stat);
                    break;
                case 1:             //total kilometers
                    Serial.println(odo/1000.0*wheel_circumference,0);
                    break;
            }
        }
        else
        {
            numberstring[num_i++]=inchar;
            return;
        }
    }
    else   //no comand found, read letters
    {
        if (firstchar == 0)                 //save first letter
        {
            firstchar = inchar;
            return;
        }
        else                                //second letter
        {
            find_commands();
        }
    }
}


