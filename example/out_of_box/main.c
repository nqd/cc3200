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
// #include "bma222drv.h"
#include "pinmux.h"
#include "wlan_if.h"
#include "setup.h"
// #include "mqtt.h"

#define OOB_TASK_PRIORITY                1
#define SPAWN_TASK_PRIORITY              9
#define OSI_STACK_SIZE                   2048
#define AP_SSID_LEN_MAX                 32
#define SL_STOP_TIMEOUT                 200


//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
static const char pcDigits[] = "0123456789";
static unsigned char POST_token[] = "__SL_P_ULD";
static unsigned char GET_token_TEMP[]  = "__SL_G_UTP";
static unsigned char GET_token_ACC[]  = "__SL_G_UAC";
static unsigned char GET_token_UIC[]  = "__SL_G_UIC";


static int g_iNetworkConnection = -1;
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



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
    g_iNetworkConnection = lRetVal;

    //Handle Async Events
    while(1)
    {
        osi_Sleep(1000);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        osi_Sleep(1000);
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);

        // read temp
        float fCurrentTemp;
        TMP006DrvGetTemp(&fCurrentTemp);
        char cTemp = (char)fCurrentTemp;
        UART_PRINT("TMP006 Temp = %dF\n\r", cTemp);

        // report with mqtt
        // int rc = buttonNotify();
        // if (rc < 0)
        //     UART_PRINT("error in reporting with mqtt\n\r");
    }
}

#include "mqtt/MQTTClient.h"

#define MQTT_TOPIC_LEDS "ticc3200/leds/#"
#define MQTT_TOPIC_BUTTON1 "ticc3200/buttons/1"
#define MQTT_TOPIC_BUTTON2 "ticc3200/buttons/2"
#define DEVICE_STRING_SUB "ticc3200-sub"
#define DEVICE_STRING_PUB "ticc3200-pub"

static void ButtonNotifyTask(void *pvParameters)
{
    static unsigned int msg_id = 0;
    Network n;
    Client hMQTTClient;
    int rc = 0;
    unsigned char buf[100];
    unsigned char readbuf[100];

    unsigned char sw2_pinNum = 22;
    unsigned char sw2_ucPin;
    unsigned int sw2_uiGPIOPort;
    unsigned char sw2_ucGPIOPin;
    unsigned char sw2_ucGPIOValue;
    unsigned char sw2_ucSent;
    
    unsigned char sw3_pinNum = 13;
    unsigned char sw3_ucPin;
    unsigned int sw3_uiGPIOPort;
    unsigned char sw3_ucGPIOPin;
    unsigned char sw3_ucGPIOValue;
    unsigned char sw3_ucSent;

    while (g_iNetworkConnection != 0) {};

    // Wait for a connection
    while (1) {

        NewNetwork(&n);
        UART_PRINT("Connecting Socket\n\r");

        //Works - Unsecured
        rc = ConnectNetwork(&n, "test.mosquitto.org", 1883);

        UART_PRINT("Opened TCP Port to mqtt broker with return code %d\n\r", rc);
        UART_PRINT("Connecting to MQTT broker\n\r");

        MQTTClient(&hMQTTClient, &n, 1000, buf, 100, readbuf, 100);
        MQTTPacket_connectData cdata = MQTTPacket_connectData_initializer;
        cdata.MQTTVersion = 3;
        cdata.keepAliveInterval = 10;
        cdata.clientID.cstring = (char*) DEVICE_STRING_PUB;
        rc = MQTTConnect(&hMQTTClient, &cdata);
        if (rc != 0) {
            UART_PRINT("rc from MQTT server is %d\n\r", rc);
        } else {
            UART_PRINT("connected to MQTT broker\n\r");
        }
        UART_PRINT("\tsocket=%d\n\r", n.my_socket);
    
        while (1) {
            UART_PRINT("tick\n\r");
            sw3_ucGPIOValue = 1;
            sw3_ucSent = 0;

                    UART_PRINT("SW3\n\r");
                    sw3_ucSent = 1;

                    MQTTMessage msg;
                    msg.dup = 0;
                    msg.id = msg_id++;
                    msg.payload = "pushed";
                    msg.payloadlen = 7;
                    msg.qos = QOS0;
                    msg.retained = 0;
                    int rc = MQTTPublish(&hMQTTClient, MQTT_TOPIC_BUTTON2, &msg);
                    UART_PRINT("SW3, rc=%d\n\r", rc);
                    if (rc != 0)
                        break;
                    
            rc = MQTTYield(&hMQTTClient, 10);
            if (rc != 0) {
                UART_PRINT("rc = %d\n\r", rc);
                break;
            }
            UART_PRINT("done\n\r");
            osi_Sleep(5000);
        }
    }
}
//****************************************************************************
//                            MAIN FUNCTION
//****************************************************************************
void main()
{
    long lRetVal = -1;

    doSetup();

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
    // Create OOB Task
    //
    UART_PRINT("Creating ButtonNotifyTask task...\n\r");
    lRetVal = osi_TaskCreate(ButtonNotifyTask, (signed char*)"ButtonNotifyTask", \
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
