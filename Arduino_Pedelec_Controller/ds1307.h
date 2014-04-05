/*
Code by JeeLabs http://news.jeelabs.org/code/
Released to the public domain! Enjoy!
modified by Jens Kießling / jenkie

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

#ifndef _ds1307_H_
#define _ds1307_H_

struct Time   
{
    uint8_t hh; //hours  
    uint8_t mm; //minutes
    uint8_t ss; //seconds
};

class RTC_DS1307 {
public:
    static void adjust_time(uint8_t hh, uint8_t mm, uint8_t ss);
    Time get_time(void);
};



#endif
