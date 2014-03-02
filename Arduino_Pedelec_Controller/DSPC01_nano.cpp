/*
Arduino Library for the Dorji DSPC01 module
written by Jens Kießling
For more Information see
http://www.dorji.com/pro/sensor-module/Compass_pressure_sensor.html
Copyright (C) 2013

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

#include "DSPC01_nano.h"
#include <Arduino.h>
#include "globals.h"

DSPC01::DSPC01()
{
}

void DSPC01::begin(int SCK_pin,int DATA_pin)
{
    DATA_PIN=DATA_pin;
    SCK_PIN=SCK_pin;
    pinMode(SCK_PIN,OUTPUT);
    pinMode(DATA_PIN,OUTPUT);
}


void DSPC01::calibrate_compass()
{
    send_SPC01_write_command(COMPASS_CALIBRATION_COMMAND);
    while ( communicate_status != COMMUNI_SUCCEED)
    {
        send_SPC01_write_command(COMPASS_CALIBRATION_COMMAND);
    }
}

unsigned long int DSPC01::pressure(void)
{
    unsigned long int press_temp=0;
    IIC_Start();
    communicate_status = IIC_WriteByte(0x21);
    if( communicate_status == COMMUNI_SUCCEED)
    {
        press_temp = IIC_ReadByte();
        press_temp*=256;
        press_temp*=256;
        IIC_ACK();
        press_temp+= (unsigned int)(IIC_ReadByte()<<8);
        IIC_ACK();
        press_temp+= IIC_ReadByte();
        IIC_NoAck();
        IIC_Stop();
    }
    return press_temp;
}

long int DSPC01::altitude(void)
{
    long int alt_temp=0;
    IIC_Start();
    communicate_status = IIC_WriteByte(0x21);
    if( communicate_status == COMMUNI_SUCCEED)
    {
        alt_temp = IIC_ReadByte();
        alt_temp*=256;
        alt_temp*=256;
        IIC_ACK();
        alt_temp+= (unsigned int)(IIC_ReadByte()<<8);
        IIC_ACK();
        alt_temp+= IIC_ReadByte();
        IIC_NoAck();
        IIC_Stop();
    }
    if (alt_temp>8388608) //negative!
        alt_temp=8388608-alt_temp;
    return alt_temp;
}

long int DSPC01::temperature(void)
{
    long int temp=0;
    IIC_Start();
    communicate_status = IIC_WriteByte(0x21);
    if( communicate_status == COMMUNI_SUCCEED)
    {
        temp +=(unsigned int)(IIC_ReadByte()<<8);
        IIC_ACK();
        temp +=IIC_ReadByte();
        IIC_NoAck();
        IIC_Stop();
    }
    if (temp>32768)
        temp = 32768-temp;
    return temp;
}

unsigned int DSPC01::compass(void)
{
    unsigned int comp=0;
    IIC_Start();
    communicate_status = IIC_WriteByte(0x21);
    if( communicate_status == COMMUNI_SUCCEED)
    {
        comp+= (unsigned long int)(IIC_ReadByte()<<8);
        IIC_ACK();
        comp+=IIC_ReadByte();
        IIC_NoAck();
        IIC_Stop();
    }
    return comp;
}


void DSPC01::request_pressure()
{
    send_SPC01_write_command(PRESS_WRITE_COMMAND);
    while ( communicate_status != COMMUNI_SUCCEED)
    {
        send_SPC01_write_command(PRESS_WRITE_COMMAND);
    }
}

void DSPC01::request_altitude()
{
    send_SPC01_write_command(ALTITUDE_WRITE_COMMAND);
    while ( communicate_status != COMMUNI_SUCCEED)
    {
        send_SPC01_write_command(ALTITUDE_WRITE_COMMAND);
    }
}

void DSPC01::request_temperature()
{
    send_SPC01_write_command(TEMPERATURE_WRITE_COMMAND);
    while ( communicate_status != COMMUNI_SUCCEED)
    {
        send_SPC01_write_command(TEMPERATURE_WRITE_COMMAND);
    }
}

void DSPC01::request_compass()
{
    send_SPC01_write_command(COMPASS_WRITE_COMMAND);
    while ( communicate_status != COMMUNI_SUCCEED)
    {
        send_SPC01_write_command(COMPASS_WRITE_COMMAND);
    }
}

//HELPER FUNCTIONS==============================================================================
void DSPC01::send_SPC01_write_command(unsigned char command)
{
    IIC_Start();
    communicate_status = IIC_WriteByte(0x20);
    if( communicate_status == COMMUNI_SUCCEED)
    {
        communicate_status = IIC_WriteByte(command);
        if( communicate_status == COMMUNI_SUCCEED)
        {
            IIC_Stop();
        }
    }
}

void DSPC01::IIC_SCL_HIGH(void)
{
    digitalWrite(SCK_PIN,1);
}

void DSPC01::IIC_SCL_LOW(void)
{
    digitalWrite(SCK_PIN,0);
}

void DSPC01::IIC_Start(void)
{
    pinMode(DATA_PIN,OUTPUT);
    digitalWrite(DATA_PIN,1);

    delayMicroseconds(100);

    IIC_SCL_HIGH();
    delayMicroseconds(100);

    digitalWrite(DATA_PIN,0);
    delayMicroseconds(100);

    IIC_SCL_LOW();
    delayMicroseconds(100);

}

void DSPC01::IIC_Stop(void)
{
    pinMode(DATA_PIN,OUTPUT);
    IIC_SCL_LOW();
    delayMicroseconds(100);

    digitalWrite(DATA_PIN,0);
    delayMicroseconds(100);

    IIC_SCL_HIGH();
    delayMicroseconds(100);

    digitalWrite(DATA_PIN,1);
    delayMicroseconds(100);
}

void DSPC01::IIC_ACK(void)
{
    pinMode(DATA_PIN,OUTPUT);
    delayMicroseconds(100);

    digitalWrite(DATA_PIN,0);
    delayMicroseconds(100);

    IIC_SCL_HIGH();
    delayMicroseconds(100);
    IIC_SCL_LOW();
    delayMicroseconds(100);

}

void DSPC01::IIC_NoAck(void)
{
    pinMode(DATA_PIN,OUTPUT);
    delayMicroseconds(100);

    digitalWrite(DATA_PIN,1);
    delayMicroseconds(100);

    IIC_SCL_HIGH();
    delayMicroseconds(100);

    IIC_SCL_LOW();
    delayMicroseconds(100);

}

unsigned char DSPC01::IIC_ReadByte(void)
{
    unsigned char ucValue;
    unsigned char ucIndex;

    ucValue = 0;
    pinMode(DATA_PIN,INPUT);
    delayMicroseconds(100);


    for ( ucIndex = 0; ucIndex < 8; ucIndex++ )
    {
        ucValue <<= 1;

        IIC_SCL_LOW();
        delayMicroseconds(100);

        IIC_SCL_HIGH();
        delayMicroseconds(100);

        if(digitalRead(DATA_PIN))  ucValue |= 1;


        delayMicroseconds(100);
        IIC_SCL_LOW();
        delayMicroseconds(100);

    }
    return ucValue;
}

unsigned char DSPC01::IIC_WriteByte( unsigned char ucData )
{
    unsigned char i;

    pinMode(DATA_PIN,OUTPUT);
    delayMicroseconds(100);
    for( i = 0; i < 8; i++ )
    {
        IIC_SCL_LOW();
        delayMicroseconds(100);

        if((ucData & 0x80) == 0x80)
        {
            digitalWrite(DATA_PIN,1);
            delayMicroseconds(100);
        }
        else

        {
            digitalWrite(DATA_PIN,0);
            delayMicroseconds(100);
        }


        IIC_SCL_HIGH();
        delayMicroseconds(100);
        ucData <<= 1;
        IIC_SCL_LOW();
    }
    pinMode(DATA_PIN,INPUT);
    delayMicroseconds(100);
    IIC_SCL_LOW();
    delayMicroseconds(100);

    IIC_SCL_HIGH();
    delayMicroseconds(100);


    if( digitalRead(DATA_PIN) != 0)
    {
        IIC_SCL_LOW();
        delayMicroseconds(100);
        delay(10);
        return (COMMUNI_FAILED);
    }
    else

    {
        IIC_SCL_LOW();
        delayMicroseconds(100);
        return (COMMUNI_SUCCEED);
    }
}
