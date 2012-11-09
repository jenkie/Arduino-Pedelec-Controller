/*
Switch handling functions: short and long press detection
(c) 2012 jenkie / pedelecforum.de

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


unsigned long switch_disp_pressed,switch_disp2_pressed,switch_thr_pressed;  //time when switch was pressed down (to decide if long or short press)
boolean switch_disp_last, switch_disp2_last,switch_thr_last;                //was switch already pressed since last loop run?
boolean switch_disp_enable,switch_disp2_enable,switch_thr_enable;         //disable switch if long-press action has been done until switch is released

void handle_switch_thr(boolean switch_thr) //throttle switch detection -----------------------------------------------------
{
  if (switch_thr==0)                        
    {
        if (switch_thr_last==1)
        {
            switch_thr_pressed=millis();
        }
        else if ((millis()-switch_thr_pressed)>1000)  //long press detected
        {
          switch_thr_pressed=millis();
          switch_thr_enable=0;
          //long-press action of throttle switch: begin >>>>>>>>>>>>>>>>>>>>>>>
          
          //long-press action of throttle switch: end   <<<<<<<<<<<<<<<<<<<<<<<
        }
    }
    else if ((switch_thr_last==0)&&((millis()-switch_thr_pressed)<1000)&&(switch_thr_enable))
        {
          //short-press action of throttle switch: begin >>>>>>>>>>>>>>>>>>>>>>>
#ifdef SUPPORT_SOFT_POTI
            // Set soft poti if throttle value changed
            if (poti_stat != throttle_stat)
            {
#ifdef SUPPORT_DISPLAY_BACKLIGHT
                enable_custom_backlight(5000);  //switch backlight on for one minute
#endif
                poti_stat = throttle_stat;
                if (poti_stat == 0)
                    display_show_important_info("Tempomat reset", 0);
                else
                    display_show_important_info("Tempomat set", 0);
            }
#endif
            //long-press action of throttle switch: end  <<<<<<<<<<<<<<<<<<<<<<<
        }
        else switch_thr_enable=1;
switch_thr_last=switch_thr;
}


        
void handle_switch_disp(boolean switch_disp) //display switch 1 detection  -----------------------------------------------------
{
    if (switch_disp==0)                  
    {
        if (switch_disp_last==1)
        {
            switch_disp_pressed=millis();
        }
        else if ((millis()-switch_disp_pressed)>1000)
        {
          switch_disp_pressed=millis();
          switch_disp_enable=0;
          //long-press action of display switch: begin >>>>>>>>>>>>>>>>>>>>>>>
#if HARDWARE_REV >=2
          display_show_important_info(msg_shutdown, 60);
          digitalWrite(fet_out,HIGH);
#endif
          //long-press action of display switch: end   <<<<<<<<<<<<<<<<<<<<<<<
        }
    }
    else if ((switch_disp_last==0)&&((millis()-switch_disp_pressed)<1000)&&(switch_disp_enable))
        {
          //short-press action of display switch: begin >>>>>>>>>>>>>>>>>>>>>>>
#if HARDWARE_REV >=2
            digitalWrite(bluetooth_pin, !digitalRead(bluetooth_pin));   //not available in 1.1!
#endif
#ifdef SUPPORT_DISPLAY_BACKLIGHT
            enable_custom_backlight(60000);  //switch backlight on for one minute
#endif
            //short-press action of display switch: end <<<<<<<<<<<<<<<<<<<<<<<
        }
        else switch_disp_enable=1;         //display switch detection end -----------------------------------------------------
switch_disp_last=switch_disp;
}

void handle_switch_disp2(boolean switch_disp2) //display switch 2 detection -----------------------------------------------------
{
#if (DISPLAY_TYPE & DISPLAY_TYPE_NOKIA_4PIN)
#ifndef SUPPORT_DISPLAY_BACKLIGHT
    if (switch_disp2==0)                    
    {
        if (switch_disp2_last==1)
        {
            switch_disp2_pressed=millis();
        }
        else if ((millis()-switch_disp2_pressed)>1000)
        {
          switch_disp2_pressed=millis();
          switch_disp2_enable=0;
          //long-press action of display switch 2: begin >>>>>>>>>>>>>>>>>>>>>>>

          //long-press action of display switch: end     <<<<<<<<<<<<<<<<<<<<<<<
        }
    }
    else if ((switch_disp2_last==0)&&((millis()-switch_disp2_pressed)<1000)&&(switch_disp2_enable))
        {
          //short-press action of display switch 2: begin >>>>>>>>>>>>>>>>>>>>>>>

          //short-press action of display switch 2: end   <<<<<<<<<<<<<<<<<<<<<<<
        }
        else switch_disp2_enable=1;       //display switch 2 detection end -----------------------------------------------------
#endif
#endif
switch_disp2_last=switch_disp2;
}

