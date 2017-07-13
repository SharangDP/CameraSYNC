/*********************************************************************
 * Name of the Program : O3M_Simulator
 * 
 * 
 * Function:             Generates CAN Mesasges from time stamps and
 *                       sends it over CAN bus @250KBPS impersonating the 
 *                       CAN ID from SYNC_TIME and GLOB_TIME messages of 
 *                       IFM Camera.
 **********************************************************************/

#include <SPI.h>
#include "mcp_can_2.h"
const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);

/**************************Global Variables******************/

unsigned long sync_time,glob_time;
unsigned char buf1[4];
unsigned char buf2[4];

/*********************Initial Setup of CAN @ 250KBPS*******/

void setup() 

{
      Serial.begin(115200);
    
      while (CAN_OK != CAN.begin(CAN_250KBPS))              
          {
            Serial.println("CAN BUS Shield init fail");
            Serial.println(" Init CAN BUS Shield again");
            delay(100);
          }
      Serial.println("CAN BUS Shield init ok!");
}

/********************MAIN LOOP***************************/

void loop()
{
      
      sync_time=micros();
      delay(0.01);                  // To differentiate the GLOB and SYNC Time Stamps
      glob_time=micros();
      int i=0;
      while(sync_time)              // Converting to HEX and storing SYNC in Buffer 1 to Send to CAN TX Register
              {
                buf1[i++]=sync_time%256;
                sync_time/=256;
              }
      i=0;
      while(glob_time)              // Converting to HEX and storing GLOB in Buffer 2 to Send to CAN TX Register
              {
                buf2[i++]=glob_time%256;
                glob_time/=256;
              }
    
      Serial.println("Sending!");
      CAN.sendMsgBuf(0x4FF00EF,1,4,buf1);               //Sending SYNC with CAN ID = 83820783
      CAN.sendMsgBuf(0x4FF01EF,1,4,buf2);               //Sending GLOB with CAN ID = 83821039
      delay(500);                                        // Cylic delay equal to the frequency of IFM Camera
  
}

/************************END OF PROGRAM*********************/