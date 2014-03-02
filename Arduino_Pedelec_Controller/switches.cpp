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
#include "config.h"
#include "switches.h"
#include "display.h"
#include "display_backlight.h"
#include "menu.h"
#include "globals.h"

struct switch_state
{
    unsigned long first_press_time;    // time when switch was pressed down (to decide if long or short press)
    bool previous_state;               // state of the switch in the previous loop run
    bool action_enabled;               // disable switch if long-press action has been done until switch is released
    sw_action action_short_press; // action to execute on short press
    sw_action action_long_press; // action to execute on long press
} switch_states[_SWITCHES_COUNT];

// Generic switch handler. Returns a 'switch_result'
enum button_state { BUTTON_ON=0, BUTTON_OFF=1 };
enum switch_result { PRESSED_NONE=0, PRESSED_SHORT=1, PRESSED_LONG=2 };

// Forward declarations
static enum switch_result _read_switch(switch_state *state, boolean switch_current);
static void _handle_menu_switch(const enum switch_name sw, const enum switch_result res);

void init_switches()
{
    memset(&switch_states, 0, sizeof(struct switch_state) * _SWITCHES_COUNT);

    // Store switch actions in the right place
    switch_states[SWITCH_THROTTLE].action_short_press = SW_THROTTLE_SHORT_PRESS;
    switch_states[SWITCH_THROTTLE].action_long_press = SW_THROTTLE_LONG_PRESS;

    switch_states[SWITCH_DISPLAY1].action_short_press = SW_DISPLAY1_SHORT_PRESS;
    switch_states[SWITCH_DISPLAY1].action_long_press = SW_DISPLAY1_LONG_PRESS;

    switch_states[SWITCH_DISPLAY2].action_short_press = SW_DISPLAY2_SHORT_PRESS;
    switch_states[SWITCH_DISPLAY2].action_long_press = SW_DISPLAY2_LONG_PRESS;
}

//
// Switch actions start here.
// Those can be executed when a button is pressed (short or long)
//
static void action_set_soft_poti()
{
#ifdef SUPPORT_SOFT_POTI
  int power_poti;
  byte i=0;
  char buffer[12]="Poti       ";

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

        {
#if CONTROL_MODE == CONTROL_MODE_TORQUE                      //human power control mode
#ifdef SUPPORT_XCELL_RT
        buffer[9]='%';
        power_poti = poti_stat/1023.0* curr_power_poti_max; //power_poti_max is in this control mode interpreted as percentage. Example: power_poti_max=200 means; motor power = 200% of human power
#endif
#endif

#if CONTROL_MODE == CONTROL_MODE_NORMAL                      //constant power mode
        buffer[9]='W';
        power_poti = poti_stat/1023.0* curr_power_poti_max;
#endif

#if CONTROL_MODE == CONTROL_MODE_LIMIT_WH_PER_KM            //wh/km control mode
        buffer[9]=0;
        buffer[10]=1;
        power_poti = poti_stat / 1023.0 * whkm_max;        //power currently set by poti in relation to speed and maximum wattage per km
#endif 
        do {       
            buffer[7-i++] = (char)(((int)'0')+(power_poti % 10)); 
         } while ((power_poti /= 10) > 0);

        display_show_important_info(buffer, 1); 
        }       
    }
#endif
}

static void action_shutdown_system()
{
    // Shut down system
#if HARDWARE_REV >=2
    display_show_important_info(msg_shutdown, 60);
    save_eeprom();
    digitalWrite(fet_out,HIGH);
#endif
}

static void action_enable_backlight_long()
{
    // Toggle backlight
#ifdef SUPPORT_DISPLAY_BACKLIGHT
    enable_custom_backlight(60000);  //switch backlight on for one minute
#endif
}

static void action_toggle_bluetooth()
{
    // Toggle bluetooth
#if HARDWARE_REV >=2
    digitalWrite(bluetooth_pin, !digitalRead(bluetooth_pin));   //not available in 1.1
#endif
}

static void action_enter_menu()
{
    if (menu_active)
        return;

#ifdef SUPPORT_DISPLAY_BACKLIGHT
    enable_custom_backlight(15 * 1000);  //switch backlight on for fifteen minute
#endif

    // Activate on the go menu
    menu_activity_expire = 0;
    menu_active = true;
    menu_changed = true;

    // Reset to top level menu
    while (menu_system.back());
}

static void action_set_profile(const boolean new_profile)
{
    current_profile = new_profile;

    activate_new_profile();
}

static void execute_action(const sw_action action)
{
    switch(action)
    {
        case ACTION_NONE:
            break;
        case ACTION_SET_SOFT_POTI:
            action_set_soft_poti();
            break;
        case ACTION_SHUTDOWN_SYSTEM:
            action_shutdown_system();
            break;
        case ACTION_ENABLE_BACKLIGHT_LONG:
            action_enable_backlight_long();
            break;
        case ACTION_TOGGLE_BLUETOOTH:
            action_toggle_bluetooth();
            break;
        case ACTION_ENTER_MENU:
            action_enter_menu();
            break;
        case ACTION_PROFILE_1:
            action_set_profile(0);
            break;
        case ACTION_PROFILE_2:
            action_set_profile(1);
            break;
        case ACTION_PROFILE:
            action_set_profile(!current_profile);
            break;
        default:
            display_show_important_info("Unknown action!", 2);
            break;
    }
}

// Switch handling code starts here
void handle_switch(const switch_name sw_name, boolean current_state)
{
    const enum switch_result res = _read_switch(&switch_states[sw_name], current_state);
    if (res == PRESSED_NONE)
        return;

    // Handle control to menu system?
    if (menu_active)
    {
        _handle_menu_switch(sw_name, res);
        return;
    }

    // normal switch action
    switch(res)
    {
        case PRESSED_LONG:
            execute_action(switch_states[sw_name].action_long_press);
            break;
        case PRESSED_SHORT:
            execute_action(switch_states[sw_name].action_short_press);
            break;
        case PRESSED_NONE:
        default:
            break;
    }
}

// Workhose of switch handling: Detect short
// or long presses, also debounces the switches.
static enum switch_result _read_switch(switch_state *state, boolean switch_current)
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
    else if (state->previous_state==BUTTON_ON
        && (now - state->first_press_time)>10
        && (now - state->first_press_time)<1000
        && state->action_enabled)
    {
        state->action_enabled = false;
        res = PRESSED_SHORT;
    }
    else
        state->action_enabled = true;

    state->previous_state = switch_current;

    return res;
}

static void _handle_menu_switch(const enum switch_name sw, const enum switch_result res)
{
#ifdef SUPPORT_DISPLAY_BACKLIGHT
    enable_custom_backlight(15 * 1000);  //switch backlight on for fifteen seconds
#endif

    switch(res)
    {
        case PRESSED_SHORT:
            if (sw == MENU_BUTTON_DOWN)
                menu_system.next(true);
            else if (sw == MENU_BUTTON_UP)
                menu_system.prev(true);

            menu_changed = true;
            break;
        case PRESSED_LONG:
            menu_system.select();
            menu_changed = true;
            break;
        default:
            break;
    }
}
