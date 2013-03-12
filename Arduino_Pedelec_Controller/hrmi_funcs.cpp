// HRMI-Breakout: http://www.sparkfun.com/products/8661
// Code mainly taken from:  http://bildr.org/2011/08/heartrate-arduino/

#include <Wire.h>
#include "hrmi_funcs.h"

#define HRMI_I2C_ADDR      127

void hrmi_open()
{
    Wire.begin();
    writeRegister(HRMI_I2C_ADDR, 0x53, 1); // Configure the HRMI with the requested algorithm mode
}


int getHeartRate()
{
    //get and return heart rate
    //returns 0 if we could not determine the heart rate
    byte i2cRspArray[3]; // I2C response array
    i2cRspArray[2] = 0;

    writeRegister(HRMI_I2C_ADDR,  0x47, 0x1); // Request a set of heart rate values

    if (hrmiGetData(127, 3, i2cRspArray))
    {
        return i2cRspArray[2];
    }
    else
    {
        return 0;
    }
}

void writeRegister(int deviceAddress, byte address, byte val)
{
    //I2C command to send data to a specific address on the device
    Wire.beginTransmission(deviceAddress); // start transmission to device
    Wire.write(address);       // send register address
    Wire.write(val);         // send value to write
    Wire.endTransmission();     // end transmission
}

boolean hrmiGetData(byte addr, byte numBytes, byte* dataArray)
{
    //Get data from heart rate monitor and fill dataArray byte with responce
    //Returns true if it was able to get it, false if not
    Wire.requestFrom(addr, numBytes);
    if (Wire.available())
    {

        for (int i=0; i<numBytes; i++)
        {
            dataArray[i] = Wire.read();
        }

        return true;
    }
    else
    {
        return false;
    }
}
