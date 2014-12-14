//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - Out of Box
//
//*****************************************************************************

// Standard includes
#include <stdlib.h>
#include <string.h>

// Simplelink includes
#include "simplelink.h"
#include "netcfg.h"

// Driverlib includes
#include "hw_ints.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "utils.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "pin.h"

// OS includes
#include "osi.h"

// Common interface includes
#include "gpio_if.h"
#include "uart_if.h"
#include "i2c_if.h"
#include "common.h"

// App Includes
#include "smartconfig.h"
#include "tmp006drv.h"
#include "bma222drv.h"
#include "pinmux.h"
#include "wlan_if.h"
#include "setup.h"

#define OOB_TASK_PRIORITY                1
#define SPAWN_TASK_PRIORITY              9
#define OSI_STACK_SIZE                   2048
#define AP_SSID_LEN_MAX                 32
#define SL_STOP_TIMEOUT                 200

typedef enum
{
  LED_OFF = 0,
  LED_ON,
  LED_BLINK
} eLEDStatus;

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static const char pcDigits[] = "0123456789";
static unsigned char POST_token[] = "__SL_P_ULD";
static unsigned char GET_token_TEMP[]  = "__SL_G_UTP";
static unsigned char GET_token_ACC[]  = "__SL_G_UAC";
static unsigned char GET_token_UIC[]  = "__SL_G_UIC";
static unsigned char g_ucDryerRunning = 0;
static unsigned char g_ucLEDStatus = LED_OFF;


//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//
//! itoa
//!
//!    @brief  Convert integer to ASCII in decimal base
//!
//!     @param  cNum is input integer number to convert
//!     @param  cString is output string
//!
//!     @return number of ASCII parameters
//!
//!
//
//*****************************************************************************
static unsigned short itoa(char cNum, char *cString)
{
    char* ptr;
    char uTemp = cNum;
    unsigned short length;

    // value 0 is a special case
    if (cNum == 0)
    {
        length = 1;
        *cString = '0';

        return length;
    }

    // Find out the length of the number, in decimal base
    length = 0;
    while (uTemp > 0)
    {
        uTemp /= 10;
        length++;
    }

    // Do the actual formatting, right to left
    uTemp = cNum;
    ptr = cString + length;
    while (uTemp > 0)
    {
        --ptr;
        *ptr = pcDigits[uTemp % 10];
        uTemp /= 10;
    }

    return length;
}


//*****************************************************************************
//
//! ReadAccSensor
//!
//!    @brief  Read Accelerometer Data from Sensor
//!
//!
//!     @return none
//!
//!
//
//*****************************************************************************
void ReadAccSensor()
{
    //Define Accelerometer Threshold to Detect Movement
    const short csAccThreshold    = 5;

    signed char cAccXT1,cAccYT1,cAccZT1;
    signed char cAccXT2,cAccYT2,cAccZT2;
    signed short sDelAccX, sDelAccY, sDelAccZ;
    int iRet = -1;
    int iCount = 0;
      
    iRet = BMA222ReadNew(&cAccXT1, &cAccYT1, &cAccZT1);
    if(iRet)
    {
        //In case of error/ No New Data return
        return;
    }
    for(iCount=0;iCount<2;iCount++)
    {
        MAP_UtilsDelay((90*80*1000)); //30msec
        iRet = BMA222ReadNew(&cAccXT2, &cAccYT2, &cAccZT2);
        if(iRet)
        {
            //In case of error/ No New Data continue
            iRet = 0;
            continue;
        }

        else
        {                       
            sDelAccX = abs((signed short)cAccXT2 - (signed short)cAccXT1);
            sDelAccY = abs((signed short)cAccYT2 - (signed short)cAccYT1);
            sDelAccZ = abs((signed short)cAccZT2 - (signed short)cAccZT1);

            //Compare with Pre defined Threshold
            if(sDelAccX > csAccThreshold || sDelAccY > csAccThreshold ||
               sDelAccZ > csAccThreshold)
            {
                //Device Movement Detected, Break and Return
                g_ucDryerRunning = 1;
                break;
            }
            else
            {
                //Device Movement Static
                g_ucDryerRunning = 0;
            }
        }
    }
       
}


//*****************************************************************************
//
//! \brief This function initializes the application variables
//!
//! \param    None
//!
//! \return None
//!
//*****************************************************************************
static void InitializeAppVariables()
{
    // g_ulStatus = 0;
    // memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
    // memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
    // g_iInternetAccess = -1;
    g_ucDryerRunning = 0;
    // g_uiDeviceModeConfig = ROLE_STA; //default is STA mode
    g_ucLEDStatus = LED_OFF;    
}




//****************************************************************************
//
//!    \brief OOB Application Main Task - Initializes SimpleLink Driver and
//!                                              Handles HTTP Requests
//! \param[in]                  pvParameters is the data passed to the Task
//!
//! \return                        None
//
//****************************************************************************
static void OOBTask(void *pvParameters)
{
    long   lRetVal = -1;

    //Read Device Mode Configuration
    UART_PRINT("\tReading device configuration...\n\r");
    ReadDeviceConfiguration();
    UART_PRINT("\tDone!\n\r");

    //Connect to Network
    UART_PRINT("\tConnecting to network...\n\r");
    lRetVal = ConnectToNetwork();
    UART_PRINT("\tDone!\n\r");
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //Handle Async Events
    while(1)
    {
        //LED Actions
        if(g_ucLEDStatus == LED_ON)
        {
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            osi_Sleep(500);
        }
        if(g_ucLEDStatus == LED_OFF)
        {
            GPIO_IF_LedOff(MCU_RED_LED_GPIO);
            osi_Sleep(500);
        }
        if(g_ucLEDStatus==LED_BLINK)
        {
            GPIO_IF_LedOn(MCU_RED_LED_GPIO);
            osi_Sleep(500);
            GPIO_IF_LedOff(MCU_RED_LED_GPIO);
            osi_Sleep(500);
        }
    }
}


//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************
void main()
{
    long   lRetVal = -1;

    doSetup();

    //
    // I2C Init
    //
    lRetVal = I2C_IF_Open(I2C_MASTER_MODE_FST);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    

    //Init Temprature Sensor
    lRetVal = TMP006DrvOpen();
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    

    //Init Accelerometer Sensor
    lRetVal = BMA222Open();
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    
    
    //
    // Simplelinkspawntask
    //
    UART_PRINT("Spawning tasks...\n\r");
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    
    
    //
    // Create OOB Task
    //
    UART_PRINT("Creating OOB task...\n\r");
    lRetVal = osi_TaskCreate(OOBTask, (signed char*)"OOBTask", \
                                OSI_STACK_SIZE, NULL, \
                                OOB_TASK_PRIORITY, NULL );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    

    //
    // Start OS Scheduler
    //
    UART_PRINT("Starting OS scheduler...\n\r");
    osi_start();

    while (1)
    {
    }

}
