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

#if (SERIAL_MODE & SERIAL_MODE_ANDROID)
// Serial comm variables

char firstchar = 0;
const byte numberlength = 12;
char numberstring[numberlength];
byte num_i=0;
byte validcommand = 0;
byte i = 0;

byte com_i;
byte com_type; // 0=float, 1=int
float samplefloat;

struct serial_float { char mnemonic[3]; float *pointer;};
serial_float sc_float[] =
{
    {{"sf"},&samplefloat},
};
int n_float = sizeof(sc_float)/sizeof(serial_float);

struct serial_int { char mnemonic[3]; int *pointer;};
serial_int sc_int[] =
{
    {{"ps"},&poti_stat},     //poti stat                  //at the moment the only valid command :) gets and sets poti stat
};
int n_int = sizeof(sc_int)/sizeof(serial_int);




void find_commands() //a command is always AA# or AA? where AA# sets # to variable AA and AA? returns value of AA
{
    validcommand = 0;
    for (i=0; i < n_float ; i++)    //try float mnemonics
    {
        if (sc_float[i].mnemonic[0] == firstchar && sc_float[i].mnemonic[1] == inchar)
        {
            com_i = i;
            com_type = 0;
            validcommand = 1;
            return;
        }
    }
    for (i=0; i < n_int ; i++)    //try int mnemonics
    {
        if (sc_int[i].mnemonic[0] == firstchar && sc_int[i].mnemonic[1] == inchar)
        {
            com_i = i;
            com_type = 1;
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
            switch(com_type)
            {
                case 0:              //float
                    *sc_float[com_i].pointer = atof(numberstring);
                    Serial.print(sc_float[com_i].mnemonic);
                    Serial.println(*sc_float[com_i].pointer,6);
                    break;
                case 1:             //int
                    if (com_i==0) //poti-stat!
                        *sc_int[com_i].pointer = min(atoi(numberstring)*1023.0/power_poti_max,1023);
                    else
                        *sc_int[com_i].pointer = atoi(numberstring);
                    Serial.print(sc_int[com_i].mnemonic);
                    Serial.println(*sc_int[com_i].pointer);
                    break;
            }
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
        if (inchar == 63 )   //"?"
        {
            switch(com_type)
            {
                case 0:              //float
                    Serial.print(sc_float[com_i].mnemonic);
                    Serial.println(*sc_float[com_i].pointer,6);
                    validcommand = 0;
                    break;
                case 1:             //int
                    Serial.print(sc_int[com_i].mnemonic);
                    Serial.println(*sc_int[com_i].pointer);
                    validcommand = 0;
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


#endif


