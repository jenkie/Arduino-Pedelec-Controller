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

#include "menu.h"
#include "MenuSystem.h"

boolean menu_active=false;

/*
Layout:
    ├── Anzeige
    │   ├── Reset Wh
    │   ├── Reset KM
    │   ├── Grafisch an/aus
    │   └── Zurück
    ├── BT an/aus
    └── Zurück
*/
MenuSystem menu_system;
static Menu menu_main("");
static Menu menu_display("Anzeige");
static MenuItem m_display_reset_wh("Reset Wh");
static MenuItem m_display_reset_km("Reset KM");
static MenuItem m_display_graphical_onoff("Grafisch an/aus");
static MenuItem m_main_bt_onoff("BT an/aus");

// Universally used "go back" menu entry
static MenuItem m_go_back("Zurück");

void init_menu()
{
    menu_main.add_menu(&menu_display);

    menu_display.add_item(&m_display_reset_wh, NULL);
    menu_display.add_item(&m_display_reset_km, NULL);
    menu_display.add_item(&m_display_graphical_onoff, NULL);
    menu_display.add_item(&m_go_back, NULL);

    menu_main.add_item(&m_main_bt_onoff, NULL);
    menu_main.add_item(&m_go_back, NULL);

    menu_system.set_root_menu(&menu_main);
}

// Switches are handles in switches.cpp:_handle_menu_switch()
// Menu display is done in display.cpp:_display_menu()
