# Default configuration with Nokia 5510 Display (PCD8544) 

## Arduino Pinout
in config.h check ```HARDWARE_REV 21```
All options are left to default values, else you have to change the connections

### A14: Voltage read-Pin
Leave it open, it will read some random values...

### 38: FET: Pull high to switch off
Add an LED with Resistor (~470Ω) to GND

### A15: Current read-Pin
Leave it open, it will read some random values...

### 11: Buzzer
Connect buzzer with resistor (~200Ω) to GND

### 37: Switch
Connect a push switch to GND

### 12: Switch 2
Connect a push switch to GND

## Nokia 5510 Display (PCD8544)
Connect Display pin -> To arduino Pin
The Display only supports 3.3V, use a level shifter for I/O Pins!
```
LED+  -> 3.3V
VCC   -> 3.3V
GND   -> GND

sclk  ->  7 [PH4] (clock)
sdin  -> 17 [PH0] (data-in)
dc    -> 16 [PH1] (data select)
reset ->  6 [PH3]
sce   ->  8 [PH5] (enable)**
```

SCE is connected to PH2, but PH2 is not connected on the Arduino, so change the line
```cpp
unsigned char sce   = 2);  /* enable      (display pin 5) */
```
to
```cpp
unsigned char sce   = 5);  /* enable      (display pin 5) */
```

### Serial Port
You get all information on the Serial port to Debug




