/*
Bluetooth communication functions
Written by Jens Kießling and Thomas Jarosch

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

//a command is always AA# or AA? where AA# sets # to variable AA and AA? returns value of AA
struct serial_command {char mnemonic[3];};
serial_command serial_commands[] =
{
    {{"ps"}},     //0: poti stat, gets and sets poti stat
    {{"od"}},     //1: total kilometers (odo), gets and sets total kilometers
};
const byte n_commands = sizeof(serial_commands)/sizeof(serial_command); //number of commands that we have

enum parse_states
{
    Reset,
    SearchCommand,
    StoreNumber,
    ErrorWaitReturn,
};

parse_states parse_state = Reset;
char cmdbuf[2];
byte cmd_index;

char numberstring[6];
static byte number_pos;

/**
 * @brief Look for valid commands in command input buffer
 * Also sets "cmd_index".
 *
 * @return bool True if command found, false otherwise
 */
static bool find_commands()
{
    for (byte i=0; i < n_commands; ++i)
    {
        if (serial_commands[i].mnemonic[0] == cmdbuf[0] && serial_commands[i].mnemonic[1] == cmdbuf[1])
        {
            cmd_index = i;
            return true;
        }
    }

    return false;
}

static void handle_command()
{
    // sanity check
    if (number_pos == 0 || cmd_index == -1)
    {
        Serial.println(MY_F("ERROR"));
        return;
    }

    Serial.print(serial_commands[cmd_index].mnemonic);
    // Info command?
    if (numberstring[0] == '?')
    {
        switch(cmd_index)
        {
            case 0:             //poti_stat
                Serial.println(poti_stat);
                break;
            case 1:             //total kilometers
                Serial.println(odo/1000.0*wheel_circumference,0);
                break;
        }

        return;
    }

    // Write command?
    switch(cmd_index)
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
    Serial.println(MY_F("OK"));
}

void parse_serial(const char &read_c)
{
    byte i;

    switch(parse_state)
    {
        case Reset:
            memset(cmdbuf, 0, sizeof(cmdbuf));
            memset(numberstring, 0, sizeof(numberstring));
            cmd_index = -1;
            number_pos = 0;

            parse_state = SearchCommand;
            // fall through to "search command"

        case SearchCommand:
            // skip return chars
            if (read_c == 10 || read_c == 13)
            {
                parse_state = Reset;
                break;
            }

            if (cmdbuf[0] == 0)
            {
                // store first character
                cmdbuf[0] = read_c;
            } else
            {
                cmdbuf[1] = read_c;

                // Look for valid commands
                if (find_commands() == true)
                    parse_state = StoreNumber;
                else
                    parse_state = ErrorWaitReturn;
            }
            break;

        case StoreNumber:
            if (read_c == 10 || read_c == 13)
            {
                // User input is done
                handle_command();
                parse_state = Reset;
            } else
            {
                // Store character
                if (number_pos < sizeof(numberstring)-1)        // reserve one byte for the terminating zero (atoi())
                {
                    numberstring[number_pos] = read_c;
                    ++number_pos;
                }
            }
            break;
        case ErrorWaitReturn:
            if (read_c == 10 || read_c == 13)
            {
                Serial.println(MY_F("ERROR"));
                parse_state = Reset;
            }
            break;
    }
}
