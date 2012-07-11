//EEPROMAnything is taken from here: http://www.arduino.cc/playground/Code/EEPROMWriteAnything
#ifndef EEPROM_ANYTHING_H
#define EEPROM_ANYTHING_H

#include "EEPROM.h"
#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    int i;
    for (i = 0; i < sizeof(value); i++)
        EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    int i;
    for (i = 0; i < sizeof(value); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}

#endif
