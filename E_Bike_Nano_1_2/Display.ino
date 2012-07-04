//Display-Functions
//written by jenkie / pedelecforum.de
//Copyright (C) 2012

//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 3 of the License, or
//(at your option) any later version.


void display_nokia_setup()    //first time setup of nokia display------------------------------------------------------------------------------------------------------------------
{
    lcd.begin(84, 48);
    lcd.createChar(0, glyph1);
    lcd.createChar(1, glyph2);
    lcd.createChar(2, glyph3);
    lcd.setCursor(4,0);
    lcd.print("V");
    lcd.setCursor(13,0);
    lcd.print("%");
    lcd.setCursor(3,1);
    lcd.print("W");
    lcd.setCursor(12,1);
    lcd.print("Wh");
    lcd.setCursor(0,2);
    lcd.print(" SPD   KM  CAD");
    lcd.setCursor(12,4);
    lcd.write(0);
    lcd.write(1);
}

void display_4bit_update()  //update 4bit display------------------------------------------------------------------------------------------------------------------
{
    lcd.setCursor(0,0);
    lcd.print(voltage_display,1);
    lcd.print(" ");
    lcd.print(battery_percent,0);
    lcd.print("%  ");
    lcd.setCursor(0,1);
    lcd.print(power,0);
    lcd.print("/");
    lcd.print(power_set);
    lcd.print("W      ");
}

void display_nokia_update()  //update nokia display------------------------------------------------------------------------------------------------------------------
{
    lcd.setCursor(0,0);
    lcd.print(voltage_display,1);

    lcd.setCursor(6,0);
    if (current_display<9.5)
        {lcd.print(" ");}
    lcd.print(current_display,1);

    lcd.setCursor(10,0);
    if (battery_percent<99.5)
        {lcd.print(" ");}
    if (battery_percent<9.5)
        {lcd.print(" ");}
    lcd.print(battery_percent,0);

    lcd.setCursor(0,1);
    if (power<99.5)
        {lcd.print(" ");}
    if (power<9.5)
        {lcd.print(" ");}
    lcd.print(power,0);

    lcd.setCursor(9,1);
    if (wh<99.5)
        {lcd.print(" ");}
    if (wh<9.5)
        {lcd.print(" ");}
    lcd.print(wh,0);


    lcd.setCursor(0,3);
    if (spd<9.5)
        {lcd.print(" ");}
    lcd.print(spd,1);

    lcd.setCursor(5,3);
    if (km<99.5)
        {lcd.print(" ");}
    if (km<9.5)
        {lcd.print(" ");}
    lcd.print(km,1);

    lcd.setCursor(11,3);
    if (cad<100)
        {lcd.print(" ");}
    if (cad<10)
        {lcd.print(" ");}
    lcd.print(cad,10);

    lcd.setCursor(0,4);
    if ( spd > 5.0)
        lcd.print(power/spd,1);
    else
        lcd.print("---");
    lcd.print("/");
    if ( km > 0.1)
        lcd.print(wh/km,1);
    else
        lcd.print("---");
    lcd.print(" ");

    lcd.setCursor(0,5);
//lcd.print(millis()/60000.0,1);
//lcd.print(" Minuten");
    lcd.print(temperature,1);
    lcd.print(" ");
    lcd.print((int)altitude);
    lcd.print(" ");
    lcd.print(analogRead(option_in));
    lcd.print("    ");
    lcd.setCursor(13,5);
    if (digitalRead(bluetooth_pin)==1)
        {lcd.write(2);}
    else
        {lcd.print(" ");}
}
