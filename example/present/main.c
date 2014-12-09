//*****************************************************************************
//
// Application Name     - Free-RTOS Demo
// Application Overview - The objective of this application is to showcasing the 
//                        FreeRTOS feature like Multiple task creation, Inter 
//                        task communication using queues. Two tasks and one 
//                        queue is created. one of the task sends a constant 
//                        message into the queue and the other task receives the 
//                        same from the queue. after receiving every message, it 
//                        displays that message over UART.
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_FreeRTOS_Application
// or
// docs\examples\CC32xx_FreeRTOS_Application.pdf
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup freertos_demo
//! @{
//
//*****************************************************************************

// Standard includes. 
#include <stdio.h>
#include <stdlib.h>

// Free-RTOS includes
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "portmacro.h"
#include "osi.h"

// Driverlib includes
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "hw_types.h"
#include "hw_ints.h"
#include "interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "uart.h"
#include "gpio.h"
#include "prcm.h"
#include "utils.h"

// Common interface includes
#include "uart_if.h"
#include "gpio_if.h"

#include "pinmux.h"

//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define APPLICATION_VERSION     "0.0.1"
#define UART_PRINT              Report
#define SPAWN_TASK_PRIORITY     9
#define OSI_STACK_SIZE          1024
#define APP_NAME                "Conlii Client"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
// The queue used to send strings to the task1.
QueueHandle_t xPrintQueue;

#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************


//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS
//*****************************************************************************
static void vTestTask1( void *pvParameters );
static void vTestTask2( void *pvParameters );
static void BoardInit();
static void LEDInit(void);


#ifdef USE_FREERTOS
//*****************************************************************************
// FreeRTOS User Hook Functions enabled in FreeRTOSConfig.h
//*****************************************************************************

//*****************************************************************************
//
//! \brief Application defined hook (or callback) function - assert
//!
//! \param[in]  pcFile - Pointer to the File Name
//! \param[in]  ulLine - Line Number
//!
//! \return none
//!
//*****************************************************************************
void
vAssertCalled( const char *pcFile, unsigned long ulLine )
{
    //Handle Assert here
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined idle task hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationIdleHook( void)
{
    //Handle Idle Hook for Profiling, Power Management etc
}

//*****************************************************************************
//
//! \brief Application defined malloc failed hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationMallocFailedHook()
{
    //Handle Memory Allocation Errors
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined stack overflow hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationStackOverflowHook( OsiTaskHandle *pxTask,
                                   signed char *pcTaskName)
{
    //Handle FreeRTOS Stack Overflow
    while(1)
    {
    }
}
#endif //USE_FREERTOS

/**
 * initialize led
 * @return  none
 */
static void  LEDInit(void) 
{
  GPIO_IF_LedConfigure(LED1|LED2|LED3);
  GPIO_IF_LedOff(MCU_ALL_LED_IND);
}
//******************************************************************************
//
//! First test task
//!
//! \param pvParameters is the parameter passed to the task while creating it.
//!
//!    This Function
//!        1. Receive message from the Queue and display it on the terminal.
//!
//! \return none
//
//******************************************************************************
void vTestTask1( void *pvParameters )
{
  portCHAR *pcMessage;
  UART_PRINT("Task 1\n\r");

    for( ;; )
    {
      /* Wait for a message to arrive. */
      xQueueReceive( xPrintQueue, &pcMessage, portMAX_DELAY );

      UART_PRINT("message = ");
      UART_PRINT(pcMessage);
      UART_PRINT("\n\r");
      MAP_UtilsDelay(2000000);
    }
}

//******************************************************************************
//
//! Second test task
//!
//! \param pvParameters is the parameter passed to the task while creating it.
//!
//!    This Function
//!        1. Creates a message and send it to the queue.
//!
//! \return none
//
//******************************************************************************
void vTestTask2( void *pvParameters )
{
  unsigned long ul_2;
  const portCHAR *pcInterruptMessage[4] = {"Welcome","to","CC32xx"
          ,"development !\n"};
  UART_PRINT("Task 2\n\r");
  ul_2 =0;
      
  for( ;; )
  {
    /* Queue a message for the print task to display on the UART CONSOLE. */
    xQueueSend( xPrintQueue, &pcInterruptMessage[ul_2 % 4], portMAX_DELAY );
    ul_2++;
    MAP_UtilsDelay(2000000);

    GPIO_IF_LedToggle(MCU_RED_LED_GPIO);
  }
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
  Report("\n\n\n\r");
  Report("\t\t *************************************************\n\r");
  Report("\t\t    CC3200 %s Application       \n\r", AppName);
  Report("\t\t *************************************************\n\r");
  Report("\n\n\n\r");
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
#if defined(ccs) || defined(gcc)
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

//*****************************************************************************
//
//!  main function handling the freertos_demo.
//!
//! \param  None
//!
//! \return none
//
//*****************************************************************************
int main( void )
{
    //
    // Initialize the board
    //
    BoardInit();

    PinMuxConfig();

    //
    // LED init
    //
    LEDInit();

    //
    // Initializing the terminal
    //
    InitTerm();

    //
    // Clearing the terminal
    //
    ClearTerm();

    //
    // Diasplay Banner
    //
    DisplayBanner(APP_NAME);

    //
    // Creating a queue for 10 elements.
    //
    xPrintQueue =xQueueCreate( 10, sizeof( unsigned portLONG ) );

    if( xPrintQueue == 0 )
    {
      // Queue was not created and must not be used.
      return 0;
    }
    VStartSimpleLinkSpawnTask(SPAWN_TASK_PRIORITY);

    //
    // Create the Queue Receive task
    //
    osi_TaskCreate( vTestTask1, ( signed portCHAR * ) "TASK1",
                OSI_STACK_SIZE, NULL, 1, NULL );

    //
    // Create the Queue Send task
    //
    osi_TaskCreate( vTestTask2, ( signed portCHAR * ) "TASK2",
                OSI_STACK_SIZE,NULL, 1, NULL );

    //
    // Start the task scheduler
    //
    osi_start();

    return 0;
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
