// HRMI-Breakout: http://www.sparkfun.com/products/8661
// Code mainly taken from:  http://bildr.org/2011/08/heartrate-arduino/

#ifndef HRMI_H
#define HRMI_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

void hrmi_open();
int getHeartRate();

#endif
