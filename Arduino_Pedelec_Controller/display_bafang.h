/*
Library for Bafang BBS01/BBS02 Displays (C965)

Copyright © 2016 Jens Kießling (jenskiessling@gmail.com)
inspired by Kingmeter Library (Michael Fabry)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


#ifndef BAFANG_H
#define BAFANG_H


// Includes
#include "config.h"

#if (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)

#if HARDWARE_REV < 20
#include <SoftwareSerial.h>
#endif


// Definitions
#define BF_CMD_STARTREQUEST 17
#define BF_CMD_STARTINFO 22
#define BF_CMD_LEVEL 11
#define BF_CMD_LIGHT 26
#define BF_CMD_WHEELDIAM 31
#define BF_CMD_GETSPEED 32
#define BF_CMD_GETERROR 8
#define BF_CMD_GETBAT 17
#define BF_CMD_GETPOWER 10
#define BF_CMD_GET2 49

#define BF_LIGHTON 241

#define BF_MAX_RXBUFF 6
#define BF_MAX_TXBUFF 3

#define BF_LEVEL0 0
#define BF_LEVEL1 1
#define BF_LEVEL2 11
#define BF_LEVEL3 12
#define BF_LEVEL4 13
#define BF_LEVEL5 2
#define BF_LEVEL6 21
#define BF_LEVEL7 22
#define BF_LEVEL8 23
#define BF_LEVEL9 3
#define BF_PUSHASSIST 6

#define BF_DISPLAYTIMEOUT 10

typedef struct
{
    // Parameters received from display in operation mode:
    uint8_t  AssistLevel;               // 0..9 Power Assist Level
    uint8_t  Headlight;                 // BF_HEADLIGHT_OFF / BF_HEADLIGHT_ON 
    uint8_t  PushAssist;                // BF_PUSHASSIST_OFF / BF_PUSHASSIST_ON
    uint16_t Wheeldiameter;             // Wheel Diameter
}RX_PARAM_t;


typedef struct
{
#if HARDWARE_REV < 20
    SoftwareSerial* SerialPort;
#else
    HardwareSerial* SerialPort;
#endif
    
    uint8_t         RxState;
    uint32_t        LastRx;

    uint8_t         RxBuff[BF_MAX_RXBUFF];
    uint8_t         RxCnt;
    uint8_t         InfoLength;

    RX_PARAM_t      Rx;

}BAFANG_t;




// Public function prototypes
#if HARDWARE_REV < 20
void Bafang_Init (BAFANG_t* BF_ctx, SoftwareSerial* DisplaySerial);
#else
void Bafang_Init (BAFANG_t* BF_ctx, HardwareSerial* DisplaySerial);
#endif

void Bafang_Service(BAFANG_t* BF_ctx);

void BF_sendmessage(BAFANG_t* BF_ctx,uint8_t count);


#endif // BAFANG
#endif // (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)
