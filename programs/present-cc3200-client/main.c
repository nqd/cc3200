//*****************************************************************************
//
// Application Name     - IOT present client
// Application Overview - Connect to cloud (name?), push trigger data, and
//                        receive command from cloud.
//                        Client use smartRF to configure wifi network, connect
//                        to proximity sensor via I2C protocol.
//                        Client sleep most of the time.
//                        Client connection via REST and MQTT (todo)
//
//*****************************************************************************

// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// simplelink includes
#include "device.h"

// driverlib includes
#include "hw_memmap.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "timer.h"
#include "utils.h"
#include "pinmux.h"

//Free_rtos/ti-rtos includes
#include "osi.h"

// common interface includes
#ifndef NOTERM
#include "uart_if.h"
#endif
#include "gpio_if.h"
#include "timer_if.h"
#include "network_if.h"
#include "udma_if.h"
#include "common.h"

// sensor
#include "tmp006drv.h"
#include "bma222drv.h"
//****************************************************************************
//                          LOCAL DEFINES                                   
//****************************************************************************
#define APPLICATION_VERSION     "1.1.0"
#define APP_NAME                "Conlii Client"

#define SERVER_RESPONSE_TIMEOUT 10
#define SLEEP_TIME              8000000
#define SUCCESS                 0
#define OSI_STACK_SIZE          3000

#define SENSOR_HEADER_BUFF "%s /home/%s/proximity HTTP/1.1\r\nHost: test.nqdinh.koding.io\r\nAccept: */\r\n\r\n"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
// Application specific status/error codes
typedef enum{
    // Choosing -0x7D0 to avoid overlap w/ host-driver's error codes
    SERVER_GET_WEATHER_FAILED = -0x7D0,
    WRONG_CITY_NAME = SERVER_GET_WEATHER_FAILED - 1,
    NO_WEATHER_DATA = WRONG_CITY_NAME - 1,
    DNS_LOOPUP_FAILED = WRONG_CITY_NAME  -1,

    STATUS_CODE_MAX = -0xBB8
}e_AppStatusCodes;

unsigned long g_ulTimerInts;   //  Variable used in Timer Interrupt Handler
SlSecParams_t SecurityParams = {0};  // AP Security Parameters
const char g_ServerAddress[30] = "test.nqdinh.koding.io"; // Weather info provider server
#define g_ServerPort	8080

unsigned char g_macAddressVal[SL_MAC_ADDR_LEN];
unsigned char g_macAddressLen = SL_MAC_ADDR_LEN;

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES                           
//****************************************************************************
static int CreateConnection(unsigned long ulDestinationIP);
static long UpdateStatus(unsigned long ulDestinationIP);
void UpdateStatusTask(void *pvParameters);
static void BoardInit();
static void DisplayBanner(char * AppName);


//*****************************************************************************
//
//! CreateConnection
//!
//! \brief  Creating an endpoint for TCP communication and initiating 
//!         connection on socket
//!
//! \param  The server hostname
//!
//! \return SocketID on Success or < 0 on Failure.        
//!
//
//*****************************************************************************
static int CreateConnection(unsigned long ulDestinationIP)
{
    int iLenorError;
    SlSockAddrIn_t  sAddr;
    int iAddrSize;
    int iSockIDorError = 0;

    sAddr.sin_family = SL_AF_INET;
    sAddr.sin_port = sl_Htons(g_ServerPort);

    //Change the DestinationIP endianity , to big endian
    sAddr.sin_addr.s_addr = sl_Htonl(ulDestinationIP);

    iAddrSize = sizeof(SlSockAddrIn_t);

    iSockIDorError = sl_Socket(SL_AF_INET,SL_SOCK_STREAM, 0);
    ASSERT_ON_ERROR(iSockIDorError);

    iLenorError = sl_Connect(iSockIDorError, ( SlSockAddr_t *)&sAddr, iAddrSize);
    ASSERT_ON_ERROR(iLenorError);

    DBG_PRINT("Socket Id: %d was created\n\r",iSockIDorError);

    return iSockIDorError;//success, connection created
}

//*****************************************************************************
//
//! Periodic Timer Interrupt Handler
//!
//! \param None
//!
//! \return None
//
//*****************************************************************************
void
TimerPeriodicIntHandler(void)
{
    unsigned long ulInts;

    //
    // Clear all pending interrupts from the timer we are
    // currently using.
    //
    ulInts = MAP_TimerIntStatus(TIMERA0_BASE, true);
    MAP_TimerIntClear(TIMERA0_BASE, ulInts);

    //
    // Increment our interrupt counter.
    //
    g_ulTimerInts++;
    if(!(g_ulTimerInts & 0x1))
    {
        //
        // Off Led
        //
        GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    }
    else
    {
        //
        // On Led
        //
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
    }
}

//****************************************************************************
//
//! Function to configure and start timer to blink the LED while device is
//! trying to connect to an AP
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void LedTimerConfigNStart()
{
    //
    // Configure Timer for blinking the LED for IP acquisition
    //
    Timer_IF_Init(PRCM_TIMERA0,TIMERA0_BASE,TIMER_CFG_PERIODIC,TIMER_A,0);
    Timer_IF_IntSetup(TIMERA0_BASE,TIMER_A,TimerPeriodicIntHandler);
    Timer_IF_Start(TIMERA0_BASE,TIMER_A,PERIODIC_TEST_CYCLES / 10);
}

//****************************************************************************
//
//! Disable the LED blinking Timer as Device is connected to AP
//!
//! \param none
//!
//! return none
//
//****************************************************************************
void LedTimerDeinitStop()
{
    //
    // Disable the LED blinking Timer as Device is connected to AP
    //
    Timer_IF_Stop(TIMERA0_BASE,TIMER_A);
    Timer_IF_DeInit(TIMERA0_BASE,TIMER_A);
}


//*****************************************************************************
//
//! Post sensor value
//!
//! \brief  Report proximity sensor to server
//!
//! \param  none
//!
//! \return none.
//!
//
//*****************************************************************************
static long UpdateStatus(unsigned long ulDestinationIP)
{
    int iTXStatus;
    int iRXDataStatus;
    char acSendBuff[512];
    char acRecvbuff[1460];
    int iSocketDesc;
    long lRetVal = -1;

    //
    // Create a TCP connection to the server
    //
    iSocketDesc = CreateConnection(ulDestinationIP);
    if(iSocketDesc < 0)
    {
        DBG_PRINT("Socket creation failed.\n\r");
        return -1;
    }

    struct SlTimeval_t timeVal;
    timeVal.tv_sec =  SERVER_RESPONSE_TIMEOUT;    // Seconds
    timeVal.tv_usec = 0;     // Microseconds. 10000 microseconds resolution
    lRetVal = sl_SetSockOpt(iSocketDesc,SL_SOL_SOCKET,SL_SO_RCVTIMEO,\
                    (unsigned char*)&timeVal, sizeof(timeVal));
    if(lRetVal < 0)
    {
       ERR_PRINT(lRetVal);
       LOOP_FOREVER();
    }

    memset(acRecvbuff, 0, sizeof(acRecvbuff));

    // prepare MAC address
    char uuid[2*g_macAddressLen+1];

    char i;
    for (i = 0; i < g_macAddressLen; i++) {
        snprintf(uuid+i*2, 3, "%02x", g_macAddressVal[i]);  // need 2 characters for a single hex value
    }

    DBG_PRINT("uuid: %s\n\r", uuid);
    //
    // Puts together the HTTP GET string.
    //
    int len;
    len = snprintf(acSendBuff, sizeof(acSendBuff), SENSOR_HEADER_BUFF, "POST", uuid);
    snprintf(&acSendBuff[len], sizeof(acSendBuff), "{\"temperature\":25}");

    DBG_PRINT("request: %s\n\r", acSendBuff);

    //
    // Send the HTTP GET string to the open TCP/IP socket.
    //
    iTXStatus = sl_Send(iSocketDesc, acSendBuff, strlen(acSendBuff), 0);
    if(iTXStatus < 0)
    {
        ASSERT_ON_ERROR(SERVER_GET_WEATHER_FAILED);
    }
    else
    {
        DBG_PRINT("Sent HTTP GET request. \n\r");
    }

    DBG_PRINT("Return value: %d \n\r", iTXStatus);

    //
    // Store the reply from the server in buffer.
    //
    iRXDataStatus = sl_Recv(iSocketDesc, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(iRXDataStatus < 0)
    {
        ASSERT_ON_ERROR(SERVER_GET_WEATHER_FAILED);
    }
    else
    {
        DBG_PRINT("Received HTTP GET response data. \n\r");
    }

    DBG_PRINT("Return value: %d \n\r", iRXDataStatus);
    DBG_PRINT("Data value: %s \n\r", acRecvbuff);

    //
    // Close the socket
    //
    lRetVal = close(iSocketDesc);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    DBG_PRINT("Socket closed\n\r");

    return SUCCESS;
}

//****************************************************************************
//
//! Task function implementing the getweather functionality
//!
//! \param none
//! 
//! This function  
//!    1. Initializes the required peripherals
//!    2. Initializes network driver and connects to the default AP
//!    3. Creates a TCP socket, gets the server IP address using DNS
//!    4. Gets the weather info for the city specified
//!
//! \return None.
//
//****************************************************************************
void UpdateStatusTask(void *pvParameters)
{
	long lRetVal = -1;
    unsigned long ulDestinationIP;

    DBG_PRINT("Conlii: Test Begin\n\r");

    //
    // Configure LED
    //
    GPIO_IF_LedConfigure(LED1|LED3);

    GPIO_IF_LedOff(MCU_RED_LED_GPIO);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);    


    //
    // Reset The state of the machine
    //
    Network_IF_ResetMCUStateMachine();

    //
    // Start the driver
    //
    lRetVal = Network_IF_InitDriver(ROLE_STA);
    if(lRetVal < 0)
    {
       UART_PRINT("Failed to start SimpleLink Device\n\r");
       return;
    }

    // Switch on Green LED to indicate Simplelink is properly UP
    GPIO_IF_LedOn(MCU_GREEN_LED_GPIO);

    //
    // Configure Timer for blinking the LED for IP acquisition
    //
    LedTimerConfigNStart();

    // Initialize AP security params
    SecurityParams.Key = (signed char *)SECURITY_KEY;
    SecurityParams.KeyLen = strlen(SECURITY_KEY);
    SecurityParams.Type = SECURITY_TYPE;

    //
    // Connect to the Access Point
    //
    lRetVal = Network_IF_ConnectAP(SSID_NAME,SecurityParams);
    if(lRetVal < 0)
    {
       UART_PRINT("Connection to an AP failed\n\r");
       LOOP_FOREVER();
    }

    //
    // Disable the LED blinking Timer as Device is connected to AP
    //
    LedTimerDeinitStop();

    //
    // Switch ON RED LED to indicate that Device acquired an IP
    //
    GPIO_IF_LedOn(MCU_IP_ALLOC_IND);

    //
    // Get MAC address
    //
	sl_NetCfgGet(SL_MAC_ADDRESS_GET,NULL,&g_macAddressLen,(unsigned char *)g_macAddressVal);

    //
    // Get the serverhost IP address using the DNS lookup
    //
    lRetVal = Network_IF_GetHostIP((char*)g_ServerAddress, &ulDestinationIP);
    if(lRetVal < 0)
    {
        UART_PRINT("DNS lookup failed. \n\r",lRetVal);
        goto end;
    }

    UpdateStatus(ulDestinationIP);

end:
    //
    // Stop the driver
    //
    lRetVal = Network_IF_DeInitDriver();
    if(lRetVal < 0)
    {
       UART_PRINT("Failed to stop SimpleLink Device\n\r");
       LOOP_FOREVER();
    }

    //
    // Switch Off RED & Green LEDs to indicate that Device is
    // disconnected from AP and Simplelink is shutdown
    //
    GPIO_IF_LedOff(MCU_IP_ALLOC_IND);
    GPIO_IF_LedOff(MCU_GREEN_LED_GPIO);

    DBG_PRINT("Conlii: Test Complete\n\r");

    //
    // Loop here
    //
    LOOP_FOREVER();
}
//*****************************************************************************
//
// Globals used by the timer interrupt handler.
//
//*****************************************************************************
//static volatile unsigned long g_ulSysTickValue;
static volatile unsigned long g_ulBase;
//static volatile unsigned long g_ulRefTimerInts = 0;
//static volatile unsigned long g_ulIntClearVector;
unsigned long g_ulTimerCount;

//*****************************************************************************
//
//! The interrupt handler for the first timer interrupt.
//!
//! \param  None
//!
//! \return none
//
//*****************************************************************************
//#define RET_IF_ERR(Func)          {int iRetValTmp = (Func); \
//                                   if (0 != iRetValTmp) { \
//                                	 DBG_PRINT("Error\n\r"); \
//                                     return iRetValTmp ;}
void
TimerBaseIntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    Timer_IF_InterruptClear(g_ulBase);

    g_ulTimerCount ++;

    float fCurrentTemp;
	TMP006DrvGetTemp(&fCurrentTemp);
	DBG_PRINT("Temp: %f\n\r", fCurrentTemp);
}
//****************************************************************************
//
//! Task function implementing the polling functionality
//!
//! \param none
//!
//! This function
//!    1. Setup timer
//!    2. Report sensor value to server
//!
//! \return None.
//
//****************************************************************************

void PollingTask(void *pvParameters)
{

    DBG_PRINT("Polling task begin\n\r");
    //
    // Base address for first timer
    //
    g_ulBase = TIMERA1_BASE;
    //
    // Configuring the timers
    //
    Timer_IF_Init(PRCM_TIMERA1, g_ulBase, TIMER_CFG_PERIODIC, TIMER_A, 0);
    //
    // Setup the interrupts for the timer timeouts.
    //
    Timer_IF_IntSetup(g_ulBase, TIMER_A, TimerBaseIntHandler);
    //
    // Turn on the timers
    //
    Timer_IF_Start(g_ulBase, TIMER_A,
                  PERIODIC_TEST_CYCLES * PERIODIC_TEST_LOOPS);
    //
    // Loop here
    //
    LOOP_FOREVER();
}

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t      CC3200 %s Application       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
  //
  // Enable Processor
  //
  MAP_IntMasterEnable();
  MAP_IntEnable(FAULT_SYSTICK);

  PRCMCC3200MCUInit();
}

//****************************************************************************
//
//! Main function
//!
//! \param none
//! 
//! This function  
//!    1. Invokes the SLHost task
//!    2. Invokes the GetWeather Task
//!
//! \return None.
//
//****************************************************************************
void main()
{
    long lRetVal = -1;

    //
    // Initialize Board configurations
    //
    BoardInit();

    //
    // Pinmux for UART
    //
    PinMuxConfig();

    //
    // Initializing DMA
    //
    UDMAInit();
#ifndef NOTERM
    //
    // Configuring UART
    //
    InitTerm();
#endif
    //
    // Display Application Banner
    //
    DisplayBanner(APP_NAME);

	// Init Temprature Sensor
	lRetVal = TMP006DrvOpen();
	if(lRetVal < 0)
	{
		ERR_PRINT(lRetVal);
		LOOP_FOREVER();
	}
    // Start the SimpleLink Host
    //
    lRetVal = VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the GetWeather task
    //
    lRetVal = osi_TaskCreate(UpdateStatusTask,
                    (const signed char *)"Update status",
                    OSI_STACK_SIZE, 
                    NULL, 
                    1, 
                    NULL );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }

    //
    // Start the Polling
    //
    lRetVal = osi_TaskCreate(PollingTask,
                    (const signed char *)"Polling status",
                    OSI_STACK_SIZE,
                    NULL,
                    1,
                    NULL );
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    //
    // Start the task scheduler
    //
    osi_start();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
