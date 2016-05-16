/*
Library for Bafang BBS01/BBS02 Displays (C965...)

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


// Includes
#include "config.h"
#include "display_bafang.h"
#include "globals.h"

#if (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)



// Definitions
#define RXSTATE_STARTCODE   0
#define RXSTATE_MSGBODY     1
#define RXSTATE_DONE        2


// Local function prototypes
static void BAFANG_Service(BAFANG_t* BF_ctx);

uint8_t TxBuff[BF_MAX_TXBUFF];
uint16_t spd_tmp;



/* Public functions (Prototypes declared by display_bafang.h) */

/****************************************************************************************************
 * Bafang_Init() - Initializes the display object
 *
 ****************************************************************************************************/
#if HARDWARE_REV < 20
void Bafang_Init (BAFANG_t* BF_ctx, SoftwareSerial* DisplaySerial)
#else
void Bafang_Init (BAFANG_t* BF_ctx, HardwareSerial* DisplaySerial)
#endif
{
    uint8_t i;
    BF_ctx->SerialPort                      = DisplaySerial;            // Store serial port to use

    BF_ctx->RxState                         = RXSTATE_STARTCODE;
    BF_ctx->LastRx                          = millis();

    for(i=0; i<BF_MAX_RXBUFF; i++)
    {
        BF_ctx->RxBuff[i]                   = 0x00;
    }

    DisplaySerial->begin(1200);
}



/****************************************************************************************************
 * Bafang_Service() - Communicates data from and to the display
 *
 ***************************************************************************************************/
void Bafang_Service(BAFANG_t* BF_ctx)
{
    uint8_t  i;  
    // Search for Start Code
    if(BF_ctx->RxState == RXSTATE_STARTCODE) //waiting for start code
    {
      if (millis()-BF_ctx->LastRx>BF_DISPLAYTIMEOUT) //new transmission frame will come
        if(BF_ctx->SerialPort->available())
        {
            BF_ctx->LastRx = millis();
            BF_ctx->RxBuff[0]=BF_ctx->SerialPort->read();
            if(BF_ctx->RxBuff[0]==BF_CMD_STARTREQUEST||BF_ctx->RxBuff[0]==BF_CMD_STARTINFO) //valid startcode detected
            {
                BF_ctx->RxCnt = 1;
                BF_ctx->RxState = RXSTATE_MSGBODY;
            }
            else
            {
                return;                                                 // No need to continue
            }
        }
    }
    
        // Receive Message body
    if(BF_ctx->RxState == RXSTATE_MSGBODY)
    {
        while(BF_ctx->SerialPort->available())
        {
            BF_ctx->RxBuff[BF_ctx->RxCnt] = BF_ctx->SerialPort->read();
            BF_ctx->RxCnt++;            
            if(BF_ctx->RxCnt > 5)   // something is wrong, reset
            {
                BF_ctx->RxState = RXSTATE_STARTCODE;
                BF_ctx->LastRx = millis();
                break;
            }
            BF_ctx->LastRx = millis();
        }
    }
    
    if ((millis()-BF_ctx->LastRx)>BF_DISPLAYTIMEOUT&&(BF_ctx->RxState == RXSTATE_MSGBODY)) //new message has been received -> analyze
    {
      BF_ctx->RxState = RXSTATE_DONE;
    }
    
    // Message received completely, analyze
    if(BF_ctx->RxState == RXSTATE_DONE)
    {
      BF_ctx->RxState = RXSTATE_STARTCODE;
      if (BF_ctx->RxBuff[0]==BF_CMD_STARTREQUEST) //display wants an answer, send it!
      {
        switch (BF_ctx->RxBuff[1])
        {
          case BF_CMD_GETSPEED:
          spd_tmp=BF_ctx->Rx.Wheeldiameter*0.03887*spd;
          TxBuff[0]=(spd_tmp>>8);
          TxBuff[1]=(spd_tmp&0xff);
          TxBuff[2]=TxBuff[0]+TxBuff[1]+32;
          BF_sendmessage(BF_ctx,3);
          break;
          
          case BF_CMD_GETERROR:
          TxBuff[0]=1;
          BF_sendmessage(BF_ctx,1);
          break;
          
          case BF_CMD_GETBAT:
          TxBuff[0]=battery_percent_fromcapacity;
          TxBuff[1]=battery_percent_fromcapacity;
          BF_sendmessage(BF_ctx,2);
          
          case BF_CMD_GETPOWER:
          TxBuff[0]=power/10;
          TxBuff[1]=power/10;
          BF_sendmessage(BF_ctx,2);
          break;
          
          case BF_CMD_GET2:
          TxBuff[0]=48;
          TxBuff[1]=48;
          BF_sendmessage(BF_ctx,2);
          break;
        }
        
      }
      else if(BF_ctx->RxBuff[0]==BF_CMD_STARTINFO)
      {
        switch (BF_ctx->RxBuff[1])
        {
          case BF_CMD_LEVEL:
          if (BF_ctx->RxBuff[3]==BF_ctx->RxBuff[0]+BF_ctx->RxBuff[1]+BF_ctx->RxBuff[2]) //checksum is correct, set poti_stat
          {
            BF_ctx->Rx.PushAssist=0;
            switch(BF_ctx->RxBuff[2])
            {
              case BF_LEVEL0:
              BF_ctx->Rx.AssistLevel=0;
              break;
              case BF_LEVEL1:
              BF_ctx->Rx.AssistLevel=1;
              break;
              case BF_LEVEL2:
              BF_ctx->Rx.AssistLevel=2;
              break;
              case BF_LEVEL3:
              BF_ctx->Rx.AssistLevel=3;
              break;             
              case BF_LEVEL4:
              BF_ctx->Rx.AssistLevel=4;
              break;                
              case BF_LEVEL5:
              BF_ctx->Rx.AssistLevel=5;
              break;                
              case BF_LEVEL6:
              BF_ctx->Rx.AssistLevel=6;
              break;               
              case BF_LEVEL7:
              BF_ctx->Rx.AssistLevel=7;
              break;  
              case BF_LEVEL8:
              BF_ctx->Rx.AssistLevel=8;
              break;  
              case BF_LEVEL9:
              BF_ctx->Rx.AssistLevel=9;
              break;
              case BF_PUSHASSIST:
              BF_ctx->Rx.PushAssist=1;
              break;
            }
          }
          break;
          
          case BF_CMD_LIGHT:
          BF_ctx->Rx.Headlight=(BF_ctx->RxBuff[2]==BF_LIGHTON);
          break;
          
          case BF_CMD_WHEELDIAM:
          BF_ctx->Rx.Wheeldiameter=BF_ctx->RxBuff[2]*256+BF_ctx->RxBuff[3];
          break;
        }
      }       
    }
}

void BF_sendmessage(BAFANG_t* BF_ctx,uint8_t count)
{
  for(int i=0; i<count; i++)
    BF_ctx->SerialPort->write(TxBuff[i]);
}



#endif // (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)
