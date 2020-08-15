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
#define RXSTATE_WAITGAP     0 //waiting for gap between messages to reset rx buffer
#define RXSTATE_STARTCODE   1 //waiting for startcode
#define RXSTATE_REQUEST     2 //request startcode received, waiting for request code
#define RXSTATE_INFO        3 //info startcode received, waiting for info code
#define RXSTATE_INFOMESSAGE 4 //info code received, waiting for info message
#define RXSTATE_DONE        5 //command received

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

    BF_ctx->RxState                         = RXSTATE_WAITGAP;
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
    //wait for gap
    if(BF_ctx->RxState == RXSTATE_WAITGAP) //waiting for start code
    {
      while(BF_ctx->SerialPort->available())
      {
        BF_ctx->SerialPort->read();
        BF_ctx->LastRx = millis();
      }
      
      if (millis()-BF_ctx->LastRx>BF_DISPLAYTIMEOUT) //gap detected
      {
        BF_ctx->RxState++; ///go to next state
        BF_ctx->RxCnt=0;
      }
    }
    
    // Search for Start Code
    if(BF_ctx->RxState == RXSTATE_STARTCODE) //waiting for start code
    {
      if(BF_ctx->SerialPort->available())
      {
          BF_ctx->LastRx = millis();
          BF_ctx->RxBuff[0]=BF_ctx->SerialPort->read();
          if(BF_ctx->RxBuff[0]==BF_CMD_STARTREQUEST) //valid request startcode detected
          {
              BF_ctx->RxCnt = 1;
              BF_ctx->RxState = RXSTATE_REQUEST;
          }
          else if(BF_ctx->RxBuff[0]==BF_CMD_STARTINFO) //valid info startcode detected
          {
              BF_ctx->RxCnt = 1;
              BF_ctx->RxState = RXSTATE_INFO;
          }
          else
          {
              BF_ctx->RxState == RXSTATE_WAITGAP;
          }
      }
    }
    
    if(BF_ctx->RxState == RXSTATE_REQUEST) //we are waiting for request code
    {
        if(BF_ctx->SerialPort->available()) //request code received
        {
            BF_ctx->RxBuff[BF_ctx->RxCnt] = BF_ctx->SerialPort->read();
            BF_ctx->RxCnt++;            
            BF_ctx->LastRx = millis();
            switch (BF_ctx->RxBuff[1]) // analyze and send correct answer
            {
              case BF_CMD_GETSPEED:
#if (DISPLAY_TYPE==DISPLAY_TYPE_BAFANG_C961)
              spd_tmp=BF_ctx->Rx.Wheeldiameter*0.02*spd;
#elif (DISPLAY_TYPE==DISPLAY_TYPE_BAFANG_C965 || DISPLAY_TYPE==DISPLAY_TYPE_BAFANG_SW102)
              spd_tmp=BF_ctx->Rx.Wheeldiameter*0.03887*spd;
#endif
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
#if (DISPLAY_TYPE==DISPLAY_TYPE_BAFANG_SW102)
              if(battery_percent_fromcapacity>90)
              {
                TxBuff[0]=75;
                TxBuff[1]=75;
              }
              else if (battery_percent_fromcapacity>70)
              {
                TxBuff[0]=50;
                TxBuff[1]=50;
              }
              else if (battery_percent_fromcapacity>50)
              {
                TxBuff[0]=30;
                TxBuff[1]=30;
              }
              else if (battery_percent_fromcapacity>30)
              {
                TxBuff[0]=10;
                TxBuff[1]=10;
              }
              else if (battery_percent_fromcapacity>10)
              {
                TxBuff[0]=6;
                TxBuff[1]=6;
              }
              else
              {
                TxBuff[0]=0;
                TxBuff[1]=0;
              }
#else
              TxBuff[0]=battery_percent_fromcapacity;
              TxBuff[1]=battery_percent_fromcapacity;
#endif    
              BF_sendmessage(BF_ctx,2);
              break;
              
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
          BF_ctx->RxState = RXSTATE_WAITGAP;  //reset state machine
         }
            
    }
    
    if(BF_ctx->RxState == RXSTATE_INFO) //we are waiting for info code
    {
      if(BF_ctx->SerialPort->available()) //info code received
      {
        BF_ctx->RxBuff[BF_ctx->RxCnt] = BF_ctx->SerialPort->read();
        BF_ctx->RxCnt++;            
        BF_ctx->LastRx = millis();
        switch (BF_ctx->RxBuff[1])   //analyze info code and set correct bytes to receive
        {
          case BF_CMD_LEVEL:
          BF_ctx->InfoLength=4;  //level message has length of 4 bytes
          BF_ctx->RxState++;
          break;
          
          case BF_CMD_LIGHT:
          BF_ctx->InfoLength=3;  //light message has length of 3 bytes
          BF_ctx->RxState++;
          break;
          
          case BF_CMD_WHEELDIAM: //wheeldiameter message has length of 5 bytes
          BF_ctx->InfoLength=5;
          BF_ctx->RxState++;
          break;
          
          default:
          BF_ctx->RxState=RXSTATE_WAITGAP; //not a valid message -> reset state machine
          break;
        }
      }        
    }
    
    if (BF_ctx->RxState == RXSTATE_INFOMESSAGE) //we are waiting for info message
    {
      while(BF_ctx->SerialPort->available()&&BF_ctx->RxCnt<BF_ctx->InfoLength) //read info message
      {
        BF_ctx->RxBuff[BF_ctx->RxCnt] = BF_ctx->SerialPort->read();
        BF_ctx->RxCnt++;            
        BF_ctx->LastRx = millis();
      }
      
      if (BF_ctx->RxCnt==BF_ctx->InfoLength) //info message complete --> analyze
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
       BF_ctx->RxState = RXSTATE_STARTCODE; 
      }
    }
}

void BF_sendmessage(BAFANG_t* BF_ctx,uint8_t count)
{
  for(int i=0; i<count; i++)
    BF_ctx->SerialPort->write(TxBuff[i]);
}



#endif // (DISPLAY_TYPE & DISPLAY_TYPE_BAFANG)
