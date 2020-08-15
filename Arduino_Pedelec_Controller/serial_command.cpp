/*
Serial (bluetooth) communication functions
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
#include "serial_command.h"
#include "globals.h"
#include "switches.h"
#ifdef SUPPORT_RTC
#include "ds1307.h"
#endif

//a command is always AA# or AA? where AA# sets # to variable AA and AA? returns value of AA
struct serial_command {char mnemonic[3];};
serial_command serial_commands[] =
{
    {{"ps"}},     //0: poti stat, gets and sets poti stat
    {{"od"}},     //1: total kilometers (odo), gets and sets total kilometers
    {{"sp"}},     //2: short button press, send button between 0 and 3
    {{"lp"}},     //3: long button press, send button between 0 and 3
    {{"hh"}},     //4: set/get hours
    {{"mm"}},     //5: set/get minutes
    {{"ss"}},     //6: set/get seconds
    {{"cc"}},     //7: set/get charge count

};
const byte n_commands = sizeof(serial_commands)/sizeof(serial_command); //number of commands that we have

enum parse_states
{
    Reset,
    SearchCommand,
    StoreNumber,
    ErrorWaitReturn,
};


struct serial_struct
{
  parse_states parse_state;
  char cmdbuf[2];
  byte cmd_index;
  char numberstring[6];
  byte number_pos;
};

serial_struct serial_port1={Reset,{0},0,{0},0};
#if HARDWARE_REV>=20
serial_struct serial_port2={Reset,{0},0,{0},0};
#endif

serial_struct* active_serial = &serial_port1;
HardwareSerial* active_serial_port = &Serial;

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
        if (serial_commands[i].mnemonic[0] == active_serial->cmdbuf[0] && serial_commands[i].mnemonic[1] == active_serial->cmdbuf[1])
        {
            active_serial->cmd_index = i;
            return true;
        }
    }

    return false;
}

static void handle_command()
{
    // sanity check
    if (active_serial->number_pos == 0 || active_serial->cmd_index == -1)
    {
        active_serial_port->println(MY_F("ERROR"));
        return;
    }

    active_serial_port->print(serial_commands[active_serial->cmd_index].mnemonic);
    // Info command?
    if (active_serial->numberstring[0] == '?')
    {
        switch(active_serial->cmd_index)
        {
            case 0:             //poti_stat
                active_serial_port->println(poti_stat);
                break;
            case 1:             //total kilometers
                active_serial_port->println(odo/1000.0*wheel_circumference,0);
                break;
            case 4:              //hours read
#ifdef SUPPORT_RTC
                active_serial_port->println(now.hh);
#endif
                break;
            case 5:              //minutes read
#ifdef SUPPORT_RTC
                active_serial_port->println(now.mm);
#endif
                break;
            case 6:              //seconds read
#ifdef SUPPORT_RTC
                active_serial_port->println(now.ss);
#endif
                break;
            case 7:              //charge count read
#ifdef SUPPORT_BATTERY_CHARGE_COUNTER
                active_serial_port->println(charge_count);
#endif
                break;
        }

        return;
    }

    // Write command?
    switch(active_serial->cmd_index)
    {
        case 0:              //poti_stat
            poti_stat = min(atoi(active_serial->numberstring)*1023.0/power_poti_max,1023);
            break;
        case 1:              //total kilometers
            odo = atoi(active_serial->numberstring)*1000.0/wheel_circumference;
            save_eeprom();
            break;
        case 2:              //short button press
            handle_switch(static_cast<switch_name>(atoi(active_serial->numberstring)), 0, PRESSED_SHORT);
            break;
        case 3:              //long button press
            handle_switch(static_cast<switch_name>(atoi(active_serial->numberstring)), 0, PRESSED_LONG);
            break;
        case 4:              //hours write
#ifdef SUPPORT_RTC
            rtc.adjust_time(atoi(active_serial->numberstring),now.mm,now.ss);
#endif
            break;
        case 5:              //minutes write
#ifdef SUPPORT_RTC
            rtc.adjust_time(now.hh,atoi(active_serial->numberstring),now.ss);
#endif
            break;
        case 6:              //seconds write
#ifdef SUPPORT_RTC
            rtc.adjust_time(now.hh,now.mm,atoi(active_serial->numberstring));
#endif
            break;
        case 7:              //charge count write
#ifdef SUPPORT_BATTERY_CHARGE_COUNTER
                charge_count=atoi(active_serial->numberstring);
                save_eeprom();
#endif
                break;
    }
    active_serial_port->println(MY_F("OK"));
}

void parse_serial(const char &read_c, const byte port)
{
    if (port==0)
    {
      active_serial=&serial_port1;
      active_serial_port=&Serial;
    }
#if HARDWARE_REV>=20
    else
    {
      active_serial=&serial_port2;
      active_serial_port=&Serial1;
    }
#endif
    byte i;

    switch(active_serial->parse_state)
    {
        case Reset:
            memset(active_serial->cmdbuf, 0, sizeof(active_serial->cmdbuf));
            memset(active_serial->numberstring, 0, sizeof(active_serial->numberstring));
            active_serial->cmd_index = -1;
            active_serial->number_pos = 0;

            active_serial->parse_state = SearchCommand;
            // fall through to "search command"

        case SearchCommand:
            // skip return chars
            if (read_c == 10 || read_c == 13)
            {
                active_serial->parse_state = Reset;
                break;
            }

            if (active_serial->cmdbuf[0] == 0)
            {
                // store first character
                active_serial->cmdbuf[0] = read_c;
            }
            else
            {
                active_serial->cmdbuf[1] = read_c;

                // Look for valid commands
                if (find_commands() == true)
                    active_serial->parse_state = StoreNumber;
                else
                    active_serial->parse_state = ErrorWaitReturn;
            }
            break;

        case StoreNumber:
            if (read_c == 10 || read_c == 13)
            {
                // User input is done
                handle_command();
                active_serial->parse_state = Reset;
            }
            else
            {
                // Store character
                if (active_serial->number_pos < sizeof(active_serial->numberstring)-1)        // reserve one byte for the terminating zero (atoi())
                {
                    active_serial->numberstring[active_serial->number_pos] = read_c;
                    ++active_serial->number_pos;
                }
            }
            break;
        case ErrorWaitReturn:
            if (read_c == 10 || read_c == 13)
            {
                active_serial_port->println(MY_F("ERROR"));
                active_serial->parse_state = Reset;
            }
            break;
    }
}
