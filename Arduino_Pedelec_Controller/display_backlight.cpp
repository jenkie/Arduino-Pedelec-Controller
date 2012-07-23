/*
Generic LCD display backlight functions
(c) 2012 Thomas Jarosch

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
#include "display_backlight.h"
#include "config.h"

#ifdef SUPPORT_DISPLAY_BACKLIGHT
const int backlight_default_show_ms = 5 * 1000;   // Time backlight stays on by default
const int backlight_blink_pause = 500;     // Blink mode: Number of milliseconds between blink switch
const int backlight_blink_goal = 4;          // Number of blinks to do

enum backlight_states
{
    BacklightOff,                // backlight is off
    BacklightTriggerOff,         // backlight should be disabled
    BacklightOn,                 // backlight is on
    BacklightTriggerOn,          // backlight should be enabled
    BacklightBlink,              // backlight in blick mode
    BacklightTriggerBlink        // backlight should start to blink
};

backlight_states backlight_state = BacklightOff;
unsigned long backlight_next_event_ms = 0;         // When should we process next backlight state

int backlight_show_ms = 0;                                    // Custom show time if wanted
bool backlight_blink_currently_enabled = false;  // Blink mode: True if backlight is currently on
bool backlight_blink_done_stay_on = false;       // If true we stay on after blinking
int backlight_blink_count = 0;                   // Number of blinks done

static void enter_backlight_state(const backlight_states new_state)
{
    // We reached the next backlight event. Process it
    switch (new_state)
    {
        case BacklightTriggerOn:
            backlight_state = BacklightOn;
            backlight_next_event_ms = millis() + backlight_show_ms;
            digitalWrite(display_backlight_pin, HIGH);
            break;
        case BacklightTriggerBlink:
            backlight_blink_count = 0;
            backlight_blink_currently_enabled = false;
            backlight_state = BacklightBlink;
        case BacklightBlink:
            ++backlight_blink_count;
            backlight_next_event_ms = millis() + backlight_blink_pause;

            // Trigger state change if we are done blinking
            if (backlight_blink_count > backlight_blink_goal)
            {
                if (backlight_blink_done_stay_on)
                {
                    backlight_show_ms = backlight_default_show_ms;
                    backlight_state = BacklightTriggerOn;
                }
                else
                    backlight_state = BacklightTriggerOff;
            }

            backlight_blink_currently_enabled = !backlight_blink_currently_enabled;
            if (backlight_blink_currently_enabled)
                digitalWrite(display_backlight_pin, HIGH);
            else
                digitalWrite(display_backlight_pin, LOW);
            break;
        case BacklightTriggerOff:
        case BacklightOn:
        case BacklightOff:
        default:
            backlight_state = BacklightOff;
            backlight_next_event_ms = 0;
            backlight_show_ms = backlight_default_show_ms;
            backlight_blink_count = 0;
            backlight_blink_currently_enabled = false;
            backlight_blink_done_stay_on = false;
            digitalWrite(display_backlight_pin, LOW);
            break;
    }
}

void enable_backlight()
{
    backlight_show_ms = backlight_default_show_ms;
    enter_backlight_state(BacklightTriggerOn);
}

void enable_custom_backlight(int duration_ms)
{
    backlight_show_ms = duration_ms;
    enter_backlight_state(BacklightTriggerOn);
}

void blink_backlight()
{
    backlight_blink_done_stay_on = false;
    enter_backlight_state(BacklightTriggerBlink);
}

void blink_backlight_stay_on()
{
    backlight_blink_done_stay_on = true;
    enter_backlight_state(BacklightTriggerBlink);
}

void handle_backlight()
{
    if (backlight_state == BacklightOff)
        return;

    if (millis() < backlight_next_event_ms)
        return;

    // Enter next backlight state
    enter_backlight_state(backlight_state);
}
#endif
