/*
Library for King-Meter displays

Copyright © 2015 Michael Fabry (Michael@Fabry.de)

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


// Includes
#include "config.h"
#include "display_kingmeter.h"

#if (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)



// Definitions
#define RXSTATE_STARTCODE   0
#define RXSTATE_SENDTXMSG   1
#define RXSTATE_MSGBODY     2
#define RXSTATE_DONE        3


// Hashtable used for handshaking in 901U protocol
#if (DISPLAY_TYPE == DISPLAY_TYPE_KINGMETER_901U)
const uint8_t KM_901U_HANDSHAKE[64] =
{
    201, 107,  13, 229, 241, 198, 108, 230, 186,  67,  39,  92, 217, 140, 177,  36,
     22,  71, 174,  39, 161, 151,   7, 140, 107, 155, 189, 195, 209, 106,  63, 191,
    218,  47, 221,  46, 135, 145,  98,  82,  35,  42,  85,  99,  35,  43, 180,  12,
      3, 126,  94, 103, 198,  10, 182, 249, 253,  86, 105, 196, 217, 183, 195, 115
};
#endif


// Local function prototypes
#if (DISPLAY_TYPE == DISPLAY_TYPE_KINGMETER_618U)
static void KM_618U_Service(KINGMETER_t* KM_ctx);
#endif

#if (DISPLAY_TYPE == DISPLAY_TYPE_KINGMETER_901U)
static void KM_901U_Service(KINGMETER_t* KM_ctx);
#endif



/* Public functions (Prototypes declared by display_kingmeter.h) */

/****************************************************************************************************
 * KingMeter_Init() - Initializes the display object
 *
 ****************************************************************************************************/
#if HARDWARE_REV < 20
void KingMeter_Init (KINGMETER_t* KM_ctx, SoftwareSerial* DisplaySerial)
#else
void KingMeter_Init (KINGMETER_t* KM_ctx, HardwareSerial* DisplaySerial)
#endif
{
    uint8_t i;


    KM_ctx->SerialPort                      = DisplaySerial;            // Store serial port to use

    KM_ctx->RxState                         = RXSTATE_STARTCODE;
    KM_ctx->LastRx                          = millis();

    for(i=0; i<KM_MAX_RXBUFF; i++)
    {
        KM_ctx->RxBuff[i]                   = 0x00;
    }

    KM_ctx->RxCnt                           = 0;

    // Settings received from display:
    KM_ctx->Settings.PAS_RUN_Direction      = KM_PASDIR_FORWARD;
    KM_ctx->Settings.PAS_SCN_Tolerance      = (uint8_t) pas_tolerance;
    KM_ctx->Settings.PAS_N_Ratio            = 255;
    KM_ctx->Settings.HND_HL_ThrParam        = KM_HND_HL_NO;
    KM_ctx->Settings.HND_HF_ThrParam        = KM_HND_HF_NO;
    KM_ctx->Settings.SYS_SSP_SlowStart      = 1;
    KM_ctx->Settings.SPS_SpdMagnets         = (uint8_t) wheel_magnets;
    KM_ctx->Settings.VOL_1_UnderVolt_x10    = (uint16_t) (vcutoff * 10);
    KM_ctx->Settings.WheelSize_mm           = (uint16_t) (wheel_circumference * 1000);

    // Parameters received from display in operation mode:
    KM_ctx->Rx.AssistLevel                  = 0;
    KM_ctx->Rx.Headlight                    = KM_HEADLIGHT_OFF;
    KM_ctx->Rx.Battery                      = KM_BATTERY_NORMAL;
    KM_ctx->Rx.PushAssist                   = KM_PUSHASSIST_OFF;
    KM_ctx->Rx.PowerAssist                  = KM_POWERASSIST_ON;
    KM_ctx->Rx.Throttle                     = KM_THROTTLE_ON;
    KM_ctx->Rx.CruiseControl                = KM_CRUISE_OFF;
    KM_ctx->Rx.OverSpeed                    = KM_OVERSPEED_NO;
    KM_ctx->Rx.SPEEDMAX_Limit_x10           = (uint16_t) (spd_max1 * 10);
    KM_ctx->Rx.CUR_Limit_x10                = 150;

    // Parameters to be send to display in operation mode:
    KM_ctx->Tx.Battery                      = KM_BATTERY_NORMAL;
    KM_ctx->Tx.Wheeltime_ms                 = KM_MAX_WHEELTIME;
    KM_ctx->Tx.Error                        = KM_ERROR_NONE;
    KM_ctx->Tx.Current_x10                  = 0;


    DisplaySerial->begin(9600);
}



/****************************************************************************************************
 * KingMeter_Service() - Communicates data from and to the display
 *
 ***************************************************************************************************/
void KingMeter_Service(KINGMETER_t* KM_ctx)
{
    #if (DISPLAY_TYPE == DISPLAY_TYPE_KINGMETER_618U)
    KM_618U_Service(KM_ctx);
    #endif

    #if (DISPLAY_TYPE == DISPLAY_TYPE_KINGMETER_901U)
    KM_901U_Service(KM_ctx);
    #endif
}




/* Local functions */

#if (DISPLAY_TYPE == DISPLAY_TYPE_KINGMETER_618U)
/****************************************************************************************************
 * KM_618U_Service() - Communication protocol of 618U firmware (J-LCD compatible)
 *
 ***************************************************************************************************/
static void KM_618U_Service(KINGMETER_t* KM_ctx)
{
    uint8_t  i;
    uint8_t TxBuff[KM_MAX_TXBUFF];


    // Search for Start Code
    if(KM_ctx->RxState == RXSTATE_STARTCODE)
    {
        if(KM_ctx->SerialPort->available())
        {
            KM_ctx->LastRx = millis();

            if(KM_ctx->SerialPort->read() == 0x46)
            {
                KM_ctx->RxBuff[0] = 0x46;
                KM_ctx->RxCnt = 1;
                KM_ctx->RxState = RXSTATE_SENDTXMSG;
            }
            else
            {
                return;                                                 // No need to continue
            }
        }
    }


    if(KM_ctx->RxState == RXSTATE_SENDTXMSG)
    {
        KM_ctx->RxState = RXSTATE_MSGBODY;

        // Prepare Tx message
        TxBuff[0] = 0X46;                                               // StartCode

        if(KM_ctx->Tx.Battery == KM_BATTERY_LOW)
        {
            TxBuff[1] = 0x00;                                           // If none of Bit[0..2] is set, display blinks
        }
        else
        {
            TxBuff[1] = 0x01;
        }

        TxBuff[2] = (uint8_t) ((KM_ctx->Tx.Current_x10 * 3) / 10);      // Current unit: 1/3A
        TxBuff[3] = highByte(KM_ctx->Tx.Wheeltime_ms);
        TxBuff[4] = lowByte (KM_ctx->Tx.Wheeltime_ms);
        TxBuff[5] = 0x7A;                                               // Reply with WheelSize 26" / Maxspeed 25km/h (no influence on display)
        TxBuff[6] = KM_ctx->Tx.Error;

        
        // Send prepared message
        TxBuff[7] = 0x00;

        KM_ctx->SerialPort->write(TxBuff[0]);                           // Send StartCode

        for(i=1; i<7; i++)
        {
            KM_ctx->SerialPort->write(TxBuff[i]);                       // Send TxBuff[1..6]
            TxBuff[7] = TxBuff[7] ^ TxBuff[i];                          // Calculate XOR CheckSum
        }

        KM_ctx->SerialPort->write(TxBuff[7]);                           // Send XOR CheckSum
    }


    // Receive Message body
    if(KM_ctx->RxState == RXSTATE_MSGBODY)
    {
        while(KM_ctx->SerialPort->available())
        {
            KM_ctx->RxBuff[KM_ctx->RxCnt] = KM_ctx->SerialPort->read();
            KM_ctx->RxCnt++;

            if(KM_ctx->RxCnt >= 6)                                      // Check for reception of complete message
            {
                // Verify XOR CheckSum
                if(KM_ctx->RxBuff[4] == (KM_ctx->RxBuff[1] ^ KM_ctx->RxBuff[2] ^ KM_ctx->RxBuff[3]))
                {
                    KM_ctx->RxState = RXSTATE_DONE;
                }
                else
                {
                    KM_ctx->RxState = RXSTATE_STARTCODE;
                }
                break;
            }
        }
    }

    // Message received completely
    if(KM_ctx->RxState == RXSTATE_DONE)
    {
        KM_ctx->RxState = RXSTATE_STARTCODE;

        // Decode PAS level - Display sets PAS-level to 0 when overspeed detected!
        KM_ctx->Rx.AssistLevel = map(KM_ctx->RxBuff[1] & 0x07, 0, 5, 0, 255);

        // Decode Headlight status
        KM_ctx->Rx.Headlight = (KM_ctx->RxBuff[1] & 0x80) >> 7;         // KM_HEADLIGHT_OFF / KM_HEADLIGHT_ON

//      KM_ctx->Rx.Battery;

        // Decode PushAssist status
        KM_ctx->Rx.PushAssist = (KM_ctx->RxBuff[1] & 0x10) >> 4;        // KM_PUSHASSIST_OFF / KM_PUSHASSIST_ON

//      KM_ctx->Rx.PowerAssist;
//      KM_ctx->Rx.Throttle;
//      KM_ctx->Rx.CruiseControl;
//      KM_ctx->Rx.OverSpeed;

        // Decode Speedlimit
        KM_ctx->Rx.SPEEDMAX_Limit_x10 = (((KM_ctx->RxBuff[2] & 0xF8) >> 3) + 10) * 10;

        // Decode Wheelsize by hashtable
        KM_ctx->Settings.WheelSize_mm = KM_WHEELSIZE[KM_ctx->RxBuff[2] & 0x07];

//      KM_ctx->Rx.CUR_Limit_x10;
    }
}
#endif




#if (DISPLAY_TYPE == DISPLAY_TYPE_KINGMETER_901U)
/****************************************************************************************************
 * KM_901U_Service() - Communication protocol of 901U firmware
 *
 ***************************************************************************************************/
static void KM_901U_Service(KINGMETER_t* KM_ctx)
{
    uint8_t  i;
    uint16_t CheckSum;
    uint8_t  TxBuff[KM_MAX_TXBUFF];
    uint8_t  TxCnt;


    // Search for Start Code
    if(KM_ctx->RxState == RXSTATE_STARTCODE)
    {
        if(KM_ctx->SerialPort->available())
        {
            KM_ctx->LastRx = millis();

            if(KM_ctx->SerialPort->read() == 0x3A)
            {
                KM_ctx->RxBuff[0] = 0x3A;
                KM_ctx->RxCnt = 1;
                KM_ctx->RxState = RXSTATE_MSGBODY;
            }
            else
            {
                return;                                                 // No need to continue
            }
        }
    }

    // Receive Message body
    if(KM_ctx->RxState == RXSTATE_MSGBODY)
    {
        while(KM_ctx->SerialPort->available())
        {
            KM_ctx->RxBuff[KM_ctx->RxCnt] = KM_ctx->SerialPort->read();
            KM_ctx->RxCnt++;

            if(KM_ctx->RxCnt == 5)                                      // Range check of Data size
            {
                if(KM_ctx->RxBuff[4] > (KM_MAX_RXBUFF-5-4))
                {
                    KM_ctx->RxState = RXSTATE_STARTCODE;                // Invalid Data size, cancel reception
                    break;
                }
            }


            if(KM_ctx->RxCnt == (5 + KM_ctx->RxBuff[4] + 4))            // Check for reception of complete message
            {
                // Verify CheckSum
                CheckSum = 0x0000;
                for(i=1; i<(5+KM_ctx->RxBuff[4]); i++)
                {
                    CheckSum = CheckSum + KM_ctx->RxBuff[i];            // Calculate CheckSum
                }

                if((lowByte(CheckSum) == KM_ctx->RxBuff[i]) && (highByte(CheckSum) == KM_ctx->RxBuff[i+1]))
                {
                    KM_ctx->RxState = RXSTATE_DONE;
                }
                else
                {
                    KM_ctx->RxState = RXSTATE_STARTCODE;                // Invalid CheckSum, ignore message
                }

                break;
            }
        }
    }


    // Message received completely
    if(KM_ctx->RxState == RXSTATE_DONE)
    {
        KM_ctx->RxState = RXSTATE_STARTCODE;

        switch(KM_ctx->RxBuff[3])
        {
            case 0x52:      // Operation mode

                // Decode Rx message
                KM_ctx->Rx.AssistLevel        =  KM_ctx->RxBuff[5];                 // 0..255
                KM_ctx->Rx.Headlight          = (KM_ctx->RxBuff[6] & 0xC0) >> 6;    // KM_HEADLIGHT_OFF / KM_HEADLIGHT_ON / KM_HEADLIGHT_LOW / KM_HEADLIGHT_HIGH
                KM_ctx->Rx.Battery            = (KM_ctx->RxBuff[6] & 0x20) >> 5;    // KM_BATTERY_NORMAL / KM_BATTERY_LOW
                KM_ctx->Rx.PushAssist         = (KM_ctx->RxBuff[6] & 0x10) >> 4;    // KM_PUSHASSIST_OFF / KM_PUSHASSIST_ON
                KM_ctx->Rx.PowerAssist        = (KM_ctx->RxBuff[6] & 0x08) >> 3;    // KM_POWERASSIST_OFF / KM_POWERASSIST_ON
                KM_ctx->Rx.Throttle           = (KM_ctx->RxBuff[6] & 0x04) >> 2;    // KM_THROTTLE_OFF / KM_THROTTLE_ON
                KM_ctx->Rx.CruiseControl      = (KM_ctx->RxBuff[6] & 0x02) >> 1;    // KM_CRUISE_OFF / KM_CRUISE_ON
                KM_ctx->Rx.OverSpeed          = (KM_ctx->RxBuff[6] & 0x01);         // KM_OVERSPEED_NO / KM_OVERSPEED_YES
                KM_ctx->Rx.SPEEDMAX_Limit_x10 = (((uint16_t) KM_ctx->RxBuff[8])<<8)  | KM_ctx->RxBuff[7];
                KM_ctx->Rx.CUR_Limit_x10      = (((uint16_t) KM_ctx->RxBuff[10])<<8) | KM_ctx->RxBuff[9];


                // Prepare Tx message
                TxBuff[0]  = 0X3A;                                      // StartCode
                TxBuff[1]  = 0x1A;                                      // SrcAdd:  Controller
                TxBuff[2]  = 0X28;                                      // DestAdd: LCD
                TxBuff[3]  = 0x52;                                      // CmdCode
                TxBuff[4]  = 0x06;                                      // DataSize

                if(KM_ctx->Tx.Battery == KM_BATTERY_LOW)
                {
                    TxBuff[5]  = 0x40;                                  // State data (only UnderVoltage bit has influence on display)
                }
                else
                {
                    TxBuff[5]  = 0x00;                                  // State data (only UnderVoltage bit has influence on display)
                }

                TxBuff[6]  = lowByte (KM_ctx->Tx.Current_x10);          // Current low
                TxBuff[7]  = highByte(KM_ctx->Tx.Current_x10);          // Current high
                TxBuff[8]  = lowByte (KM_ctx->Tx.Wheeltime_ms);         // WheelSpeed low
                TxBuff[9]  = highByte(KM_ctx->Tx.Wheeltime_ms);         // WheelSpeed high
                TxBuff[10] = KM_ctx->Tx.Error;                          // Error

                TxCnt = 11;
                break;


            case 0x53:      // Settings mode

                // Decode Rx message
                KM_ctx->Settings.PAS_RUN_Direction   = (KM_ctx->RxBuff[5] & 0x80) >> 7; // KM_PASDIR_FORWARD / KM_PASDIR_BACKWARD
                KM_ctx->Settings.PAS_SCN_Tolerance   =  KM_ctx->RxBuff[6];              // 2..9 
                KM_ctx->Settings.PAS_N_Ratio         =  KM_ctx->RxBuff[7];              // 0..255
                KM_ctx->Settings.HND_HL_ThrParam     = (KM_ctx->RxBuff[8] & 0x80) >> 7; // KM_HND_HL_NO / KM_HND_HL_YES
                KM_ctx->Settings.HND_HF_ThrParam     = (KM_ctx->RxBuff[8] & 0x40) >> 6; // KM_HND_HF_NO / KM_HND_HF_YES
                KM_ctx->Settings.SYS_SSP_SlowStart   =  KM_ctx->RxBuff[9];              // 1..9
                KM_ctx->Settings.SPS_SpdMagnets      =  KM_ctx->RxBuff[10];             // 1..4
                KM_ctx->Settings.VOL_1_UnderVolt_x10 = (((uint16_t) KM_ctx->RxBuff[12])<<8) | KM_ctx->RxBuff[11];
                KM_ctx->Settings.WheelSize_mm        = (((uint16_t) KM_ctx->RxBuff[14])<<8) | KM_ctx->RxBuff[13];


                // Prepare Tx message with handshake code
                TxBuff[0] = 0X3A;                                       // StartCode
                TxBuff[1] = 0x1A;                                       // SrcAdd:  Controller
                TxBuff[2] = 0X28;                                       // DestAdd: LCD
                TxBuff[3] = 0x53;                                       // CmdCode
                TxBuff[4] = 0x01;                                       // DataSize
                TxBuff[5] = KM_901U_HANDSHAKE[KM_ctx->RxBuff[15]];      // Handshake answer
                TxCnt = 6;
                break;

            default:
                TxCnt = 0;
        }


        // Send prepared message
        if(TxCnt != 0)
        {
            CheckSum = 0x0000;

            KM_ctx->SerialPort->write(TxBuff[0]);                       // Send StartCode

            for(i=1; i<TxCnt; i++)
            {
                KM_ctx->SerialPort->write(TxBuff[i]);                   // Send TxBuff[1..x]
                CheckSum = CheckSum + TxBuff[i];                        // Calculate CheckSum 
            }

            KM_ctx->SerialPort->write(lowByte(CheckSum));               // Send CheckSum low
            KM_ctx->SerialPort->write(highByte(CheckSum));              // Send CheckSum high

            KM_ctx->SerialPort->write(0x0D);                            // Send CR
            KM_ctx->SerialPort->write(0x0A);                            // Send LF
        }
    }
}
#endif

#endif // (DISPLAY_TYPE & DISPLAY_TYPE_KINGMETER)
