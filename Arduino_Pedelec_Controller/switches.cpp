/*
Switch handling functions: short and long press detection
(c) 2012-2013 jenkie and Thomas Jarosch / pedelecforum.de

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
#include "switches.h"
#include "config.h"
#include "display.h"
#include "display_backlight.h"

struct switch_state
{
    unsigned long first_press_time;    // time when switch was pressed down (to decide if long or short press)
    bool previous_state;               // state of the switch in the previous loop run
    bool action_enabled;               // disable switch if long-press action has been done until switch is released
} swt_throttle, swt_display1, swt_display2;

void init_switches()
{
    memset(&swt_throttle, 0, sizeof(struct switch_state));
    memset(&swt_display1, 0, sizeof(struct switch_state));
    memset(&swt_display2, 0, sizeof(struct switch_state));
}

// Generic switch handler. Returns a 'switch_result'
enum button_state { BUTTON_ON=0, BUTTON_OFF=1 };
enum switch_result { PRESSED_NONE=0, PRESSED_SHORT=1, PRESSED_LONG=2 };
static enum switch_result _handle_switch(switch_state *state, boolean switch_current)
{
    enum switch_result res = PRESSED_NONE;
    const unsigned long now = millis();

    if (switch_current==BUTTON_ON)
    {
        if (state->previous_state==BUTTON_OFF)
        {
            // first press
            state->first_press_time=now;
        }
        else if ((now - state->first_press_time)>1000 && state->action_enabled)
        {
            state->action_enabled = false;
            res = PRESSED_LONG;
        }
    }
    else if (state->previous_state==BUTTON_ON && (now - state->first_press_time)<1000 && state->action_enabled)
    {
        state->action_enabled = false;
        res = PRESSED_SHORT;
    }
    else
        state->action_enabled = true;

    state->previous_state = switch_current;

    return res;
}

void handle_switch_thr(boolean current_state)
{
    switch(_handle_switch(&swt_throttle, current_state))
    {
        case PRESSED_LONG:
            break;
        case PRESSED_SHORT:
#ifdef SUPPORT_SOFT_POTI
            // Set soft poti if throttle value changed
            if (poti_stat != throttle_stat)
            {
#ifdef SUPPORT_DISPLAY_BACKLIGHT
                enable_custom_backlight(5000);  //switch backlight on for five seconds
#endif
                poti_stat = throttle_stat;
                if (poti_stat == 0)
                    display_show_important_info("Tempomat reset", 0);
                else
                    display_show_important_info("Tempomat set", 0);
            }
#endif
            break;
        case PRESSED_NONE:
        default:
            break;
    }
}

void handle_switch_disp(boolean current_state)
{
    switch(_handle_switch(&swt_display1, current_state))
    {
        case PRESSED_LONG:
            // Shut down system
#if HARDWARE_REV >=2
            display_show_important_info(msg_shutdown, 60);
            digitalWrite(fet_out,HIGH);
#endif
            break;
        case PRESSED_SHORT:
            // Toggle bluetooth and backlight
#if HARDWARE_REV >=2
            digitalWrite(bluetooth_pin, !digitalRead(bluetooth_pin));   //not available in 1.1!
#endif
#ifdef SUPPORT_DISPLAY_BACKLIGHT
            enable_custom_backlight(60000);  //switch backlight on for one minute
#endif
            break;
        case PRESSED_NONE:
        default:
            break;
    }
}

void handle_switch_disp2(boolean current_state)
{
    switch(_handle_switch(&swt_display2, current_state))
    {
        case PRESSED_LONG:
            break;
        case PRESSED_SHORT:
            break;
        case PRESSED_NONE:
        default:
            break;
    }
}
