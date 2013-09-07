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

#define COMMUNI_SUCCEED             1  
#define COMMUNI_FAILED              0  
  
#define noACK 0  
#define ACK 1  
  
#define           COMPASS_CALIBRATION_COMMAND       0xe0  
#define           PRESS_WRITE_COMMAND               0xB0  
#define           TEMPERATURE_WRITE_COMMAND         0x80  
#define           ALTITUDE_WRITE_COMMAND            0xA0  
#define           COMPASS_WRITE_COMMAND             0xC0  

class DSPC01
{
public:
    DSPC01();
    void request_pressure(void);
    void request_altitude(void);
    void request_temperature(void);
    void request_compass(void);
    void calibrate_compass(void);
    unsigned long int pressure(void);
    long int altitude(void);
    long int temperature(void);
    unsigned int compass(void);
    void begin(int SCK_pin,int DATA_pin);

private:
   int SCK_PIN;
   int DATA_PIN;
   unsigned char communicate_status; 
   void send_SPC01_write_command(unsigned char command);
   void IIC_SCL_HIGH(void);
   void IIC_SCL_LOW(void);
   void IIC_Start(void);
   void IIC_Stop(void);
   void IIC_ACK(void);
   void IIC_NoAck(void);
   unsigned char IIC_ReadByte(void);
   unsigned char IIC_WriteByte( unsigned char ucData );
};
