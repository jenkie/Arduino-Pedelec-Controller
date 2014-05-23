/*
On the go menu system
(c) 2013 Thomas Jarosch / pedelecforum.de

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
#include "globals.h"
#include "menu.h"
#include "display.h"
#include "display_backlight.h"
#include "MenuSystem.h"
#include "switches.h"

unsigned long menu_activity_expire=0;
boolean menu_active=false;
boolean menu_changed=false;

/*
Layout:
    ├── [Ausschalten]              (only shown when no button is set to ACTION_SHUTDOWN_SYSTEM)
    ├── Anzeige
    │   ├── Reset Wh
    │   ├── Reset KM
    │   ├── Graf. an/              (only for Nokia displays)
    |   ├── Beleuchtung an         (only with SUPPORT_DISPLAY_BACKLIGHT)
    │   └── Zurück
    ├── Licht an/aus
    ├── BT an/aus
    ├── Profil 1<>2
    ├── Sonstiges
    │   ├── Nothilfe
    │   │    ├── Ign. Bremse
    │   │    ├── Ign. Treten
    │   │    ├── Ign. Tacho
    │   │    ├── Ign. Gasgr.
    │   │    ├── Ign. Poti             (only with SUPPORT_POTI)
    │   │    ├── Poti +
    │   │    ├── Poti -
    │   │    └── Zurück
    │   └── Zurück
    └── Zurück
*/
MenuSystem menu_system;

static const char desc_main[] PROGMEM = "";
static Menu menu_main(desc_main);

static const char desc_display[] PROGMEM = "Anzeige ->";
static Menu menu_display(desc_display);

static const char desc_display_reset_wh[] PROGMEM = "Reset Wh";
static MenuItem m_display_reset_wh(desc_display_reset_wh);

static const char desc_display_reset_km[] PROGMEM = "Reset KM";
static MenuItem m_display_reset_km(desc_display_reset_km);

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
static const char desc_display_graphical_onoff[] PROGMEM = "Graf. an/aus";
static MenuItem m_display_graphical_onoff(desc_display_graphical_onoff);
#endif

#ifdef SUPPORT_DISPLAY_BACKLIGHT
static const char desc_display_backlight_on[] PROGMEM = "Beleuchtung an";
static MenuItem m_display_backlight_on(desc_display_backlight_on);
#endif

static const char desc_main_lights_onoff[] PROGMEM = "Licht an/aus";
static MenuItem m_main_lights_onoff(desc_main_lights_onoff);

static const char desc_main_bt_onoff[] PROGMEM = "BT an/aus";
static MenuItem m_main_bt_onoff(desc_main_bt_onoff);

static const char desc_main_shutdown[] PROGMEM = "Ausschalten";
static MenuItem m_main_shutdown(desc_main_shutdown);

static const char desc_main_profile[] PROGMEM = "Profil 1<>2";
static MenuItem m_main_profile(desc_main_profile);

#ifdef SUPPORT_FIRST_AID_MENU
static const char desc_misc[] PROGMEM = "Sonstiges ->";
static Menu menu_misc(desc_misc);

static const char desc_first_aid[] PROGMEM = "Nothilfe ->";
static Menu menu_first_aid(desc_first_aid);

static const char desc_first_aid_ignore_break[] PROGMEM = "Ign. Bremse";
static MenuItem m_first_aid_ignore_break(desc_first_aid_ignore_break);

static const char desc_first_aid_ignore_pas[] PROGMEM = "Ign. Treten";
static MenuItem m_first_aid_ignore_pas(desc_first_aid_ignore_pas);

static const char desc_first_aid_ignore_speed[] PROGMEM = "Ign. Tacho";
static MenuItem m_first_aid_ignore_speed(desc_first_aid_ignore_speed);

static const char desc_first_aid_ignore_throttle[] PROGMEM = "Ign. Gasgr.";
static MenuItem m_first_aid_ignore_throttle(desc_first_aid_ignore_throttle);

static const char desc_first_aid_ignore_poti[] PROGMEM = "Ignore Poti";
static MenuItem m_first_aid_ignore_poti(desc_first_aid_ignore_poti);

static const char desc_first_aid_inc_poti[] PROGMEM = "Poti +";
static MenuItem m_first_aid_inc_poti(desc_first_aid_inc_poti);

static const char desc_first_aid_dec_poti[] PROGMEM = "Poti -";
static MenuItem m_first_aid_dec_poti(desc_first_aid_dec_poti);
#endif

// Universally used "go back" menu entry
static const char desc_go_back[] PROGMEM = "Zurueck ->";
static MenuItem m_go_back(desc_go_back);

static void handle_reset_wh(MenuItem* p_menu_item)
{
    wh = 0;
    mah = 0;

    menu_active = false;
}

static void handle_reset_km(MenuItem* p_menu_item)
{
    km = 0;

    menu_active = false;
}

static void show_new_state(const boolean is_on)
{
    if (is_on)
        display_show_important_info(FROM_FLASH(msg_activated), 1);
    else
        display_show_important_info(FROM_FLASH(msg_deactivated), 1);

    menu_active = false;
}

#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
static void handle_graphical_onoff(MenuItem* p_menu_item)
{
    display_force_text = !display_force_text;
    menu_active = false;
}
#endif

#ifdef SUPPORT_DISPLAY_BACKLIGHT
static void handle_display_backlight_on(MenuItem* p_menu_item)
{
    bool enabled = toggle_force_backlight_on();

    show_new_state(enabled);
}
#endif

#if defined(SUPPORT_LIGHTS_SWITCH) && defined(SUPPORT_LIGHTS_SWITCH_MENU)
static void handle_lights_onoff(MenuItem* p_menu_item)
{
    bool new_state = !digitalRead(lights_pin);

    // Toggle lights
    digitalWrite(lights_pin, new_state);

    show_new_state(new_state);
}
#endif

static void handle_bluetooth_onoff(MenuItem* p_menu_item)
{
    bool new_state = false;

    // Toggle bluetooth
#if HARDWARE_REV >=2
    new_state = !digitalRead(bluetooth_pin);
    digitalWrite(bluetooth_pin, new_state);   //not available in 1.1
#endif

    show_new_state(new_state);
}

static void handle_shutdown(MenuItem* p_menu_item)
{
    // Shut down system
#if HARDWARE_REV >=2
    display_show_important_info(FROM_FLASH(msg_shutdown), 60);
    digitalWrite(fet_out,HIGH);
#endif

    menu_active = false;
}

static void handle_go_back(MenuItem* p_menu_item)
{
    // Go back in the main menu?
    if (menu_system.back() == false)
        menu_active = false;
}

static void handle_profile(MenuItem* p_menu_item)
{
    current_profile=!(current_profile);
    activate_new_profile();

    menu_active = false;
}

#ifdef SUPPORT_FIRST_AID_MENU
static void handle_ignore_break(MenuItem* p_menu_item)
{
    first_aid_ignore_break = !first_aid_ignore_break;
    show_new_state(first_aid_ignore_break);
}

static void handle_ignore_pas(MenuItem* p_menu_item)
{
    first_aid_ignore_pas = !first_aid_ignore_pas;
    show_new_state(first_aid_ignore_pas);
}

static void handle_ignore_speed(MenuItem* p_menu_item)
{
    first_aid_ignore_speed = !first_aid_ignore_speed;
    show_new_state(first_aid_ignore_speed);
}

static void handle_ignore_throttle(MenuItem* p_menu_item)
{
    first_aid_ignore_throttle = !first_aid_ignore_throttle;
    show_new_state(first_aid_ignore_throttle);
}

static void handle_ignore_poti(MenuItem* p_menu_item)
{
    first_aid_ignore_poti = !first_aid_ignore_poti;

    // Just switched it on? Reset poti value
    if (first_aid_ignore_poti)
        poti_stat = 0;

    show_new_state(first_aid_ignore_poti);
}

static void handle_inc_poti(MenuItem* p_menu_item)
{
    first_aid_ignore_poti = true;

    action_increase_poti();

    menu_active = false;
}

static void handle_dec_poti(MenuItem* p_menu_item)
{
    first_aid_ignore_poti = true;

    action_decrease_poti();

    menu_active = false;
}

static void add_first_aid_menu()
{
    menu_main.add_menu(&menu_misc);
    menu_misc.add_menu(&menu_first_aid);
    menu_misc.add_item(&m_go_back, &handle_go_back);

#ifdef SUPPORT_BRAKE
    menu_first_aid.add_item(&m_first_aid_ignore_break, &handle_ignore_break);
#endif
#ifdef SUPPORT_PAS
    menu_first_aid.add_item(&m_first_aid_ignore_pas, &handle_ignore_pas);
#endif
    menu_first_aid.add_item(&m_first_aid_ignore_speed, &handle_ignore_speed);
#ifdef SUPPORT_THROTTLE
    menu_first_aid.add_item(&m_first_aid_ignore_throttle, &handle_ignore_throttle);
#endif

#ifdef SUPPORT_POTI
    menu_first_aid.add_item(&m_first_aid_ignore_poti, &handle_ignore_poti);
#endif
#if defined(SUPPORT_POTI) \
        || defined(SUPPORT_THROTTLE) \
        || defined(SUPPORT_SOFT_POTI) \
        || defined (SUPPORT_POTI_SWITCHES)
    menu_first_aid.add_item(&m_first_aid_inc_poti, &handle_inc_poti);
    menu_first_aid.add_item(&m_first_aid_dec_poti, &handle_dec_poti);
#endif

    menu_first_aid.add_item(&m_go_back, &handle_go_back);
}
#endif

void init_menu()
{
#if HARDWARE_REV >=2
    // Add 'shutdown' entry if no switch is configured as the 'shutdown' action
    if (SW_THROTTLE_SHORT_PRESS != ACTION_SHUTDOWN_SYSTEM &&
            SW_THROTTLE_LONG_PRESS != ACTION_SHUTDOWN_SYSTEM &&
            SW_DISPLAY1_SHORT_PRESS != ACTION_SHUTDOWN_SYSTEM &&
            SW_DISPLAY1_LONG_PRESS != ACTION_SHUTDOWN_SYSTEM &&
            SW_DISPLAY2_SHORT_PRESS != ACTION_SHUTDOWN_SYSTEM &&
            SW_DISPLAY2_LONG_PRESS != ACTION_SHUTDOWN_SYSTEM)
    {
        menu_main.add_item(&m_main_shutdown, &handle_shutdown);
    }
#endif

    menu_main.add_menu(&menu_display);

    menu_display.add_item(&m_display_reset_wh, &handle_reset_wh);
    menu_display.add_item(&m_display_reset_km, &handle_reset_km);
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    menu_display.add_item(&m_display_graphical_onoff, &handle_graphical_onoff);
#endif
#ifdef SUPPORT_DISPLAY_BACKLIGHT
    menu_display.add_item(&m_display_backlight_on, &handle_display_backlight_on);
#endif
    menu_display.add_item(&m_go_back, &handle_go_back);

#if defined(SUPPORT_LIGHTS_SWITCH) && defined(SUPPORT_LIGHTS_SWITCH_MENU)
    menu_main.add_item(&m_main_lights_onoff, &handle_lights_onoff);
#endif

#if HARDWARE_REV >=2
    // Not available in 1.1
#if (SERIAL_MODE & SERIAL_MODE_MMC) || (SERIAL_MODE & SERIAL_MODE_ANDROID)
    menu_main.add_item(&m_main_bt_onoff, &handle_bluetooth_onoff);
#endif
#endif

#ifdef SUPPORT_PROFILE_SWITCH_MENU
    menu_main.add_item(&m_main_profile, &handle_profile);
#endif
#ifdef SUPPORT_FIRST_AID_MENU
    add_first_aid_menu();
#endif
    menu_main.add_item(&m_go_back, &handle_go_back);

    menu_system.set_root_menu(&menu_main);
}

// Switches are handles in switches.cpp:_handle_menu_switch()
// Menu display is done in display.cpp:_display_menu()
