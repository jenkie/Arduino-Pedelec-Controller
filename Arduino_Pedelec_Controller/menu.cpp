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
    │   ├── Graf. an/aus
    │   └── Zurück
    ├── BT an/aus
    ├── Profil 1<>2
    ├── Sonstiges
    │   ├── Nothilfe
    │   │    ├── Ign. Bremse
    │   │    ├── Ign. Treten
    │   │    ├── Ign. Tacho
    │   │    └── Zurück
    │   └── Zurück
    └── Zurück
*/
MenuSystem menu_system;
static Menu menu_main("");
static Menu menu_display("Anzeige ->");
static MenuItem m_display_reset_wh("Reset Wh");
static MenuItem m_display_reset_km("Reset KM");
static MenuItem m_display_graphical_onoff("Graf. an/aus");
static MenuItem m_main_bt_onoff("BT an/aus");
static MenuItem m_main_shutdown("Ausschalten");
static MenuItem m_main_profile("Profil 1<>2");
#ifdef SUPPORT_FIRST_AID_MENU
static Menu menu_misc("Sonstiges ->");
static Menu menu_first_aid("Nothilfe ->");
static MenuItem m_first_aid_ignore_break("Ign. Bremse");
static MenuItem m_first_aid_ignore_pas("Ign. Treten");
static MenuItem m_first_aid_ignore_speed("Ign. Tacho");
#endif

// Universally used "go back" menu entry
static MenuItem m_go_back("Zurueck ->");

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

static void handle_graphical_onoff(MenuItem* p_menu_item)
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA)
    display_force_text = !display_force_text;
#endif

    menu_active = false;
}

static void handle_bluetooth_onoff(MenuItem* p_menu_item)
{
    // Toggle bluetooth
#if HARDWARE_REV >=2
    digitalWrite(bluetooth_pin, !digitalRead(bluetooth_pin));   //not available in 1.1
#endif

    menu_active = false;
}

static void handle_shutdown(MenuItem* p_menu_item)
{
    // Shut down system
#if HARDWARE_REV >=2
    display_show_important_info(msg_shutdown, 60);
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
static void show_new_state(const boolean is_on)
{
    if (is_on)
        display_show_important_info("Aktiviert", 1);
    else
        display_show_important_info("Deaktiviert", 1);

    menu_active = false;
}

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

// TODO: Remove this for final version
static void show_not_implemented(MenuItem* p_menu_item)
{
    display_show_important_info("Not implemented :(", 2);
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
    menu_display.add_item(&m_go_back, &handle_go_back);

#if HARDWARE_REV >=2
    // Not available in 1.1
    menu_main.add_item(&m_main_bt_onoff, &handle_bluetooth_onoff);
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
