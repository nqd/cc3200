// Standard includes
#include <stdlib.h>
#include <string.h>

// Common interface includes
#include "gpio_if.h"
#include "uart_if.h"
#include "i2c_if.h"
#include "common.h"

// Driverlib includes
#include "rom_map.h"
#include "hw_ints.h"
#include "hw_types.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "prcm.h"
#include "pin.h"

// App Includes
#include "setup.h"
#include "pinmux.h"
#include "tmp006drv.h"
#include "bma222drv.h"

#if defined(ccs) || defined(gcc)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif

// TODO: write this function
void ConfigureGPIO(void)
{
    // Initialize the LED pins
    // GPIO_IF_LedConfigure(0xff);
    GPIO_IF_LedConfigure(LED1);
    GPIO_IF_LedOff(MCU_RED_LED_GPIO);

    // Set Interrupt Type for GPIO buttons
    // GPIOIntTypeSet(LEFT_BUTTON_PORT, LEFT_BUTTON, GPIO_BOTH_EDGES);
    // GPIOIntTypeSet(RIGHT_BUTTON_PORT, RIGHT_BUTTON, GPIO_BOTH_EDGES);

    // Register Interrupt handlers
    // IntRegister(17, button_handler);
    // IntPrioritySet(17, 1);
    // IntEnable(17);
    // IntRegister(18, button_handler);
    // IntPrioritySet(18, 1);
    // IntEnable(18);

    // Enable Interrupts
    // GPIOIntClear(GPIOA1_BASE,GPIO_PIN_5);
    // GPIOIntEnable(GPIOA1_BASE,GPIO_INT_PIN_5);

    // GPIOIntClear(GPIOA2_BASE,GPIO_PIN_6);
    // GPIOIntEnable(GPIOA2_BASE,GPIO_INT_PIN_6);
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
void
DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t     CC3200 %s Application       \n\r", AppName);
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
void
BoardInit(void)
{
    /* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
    //
    // Set vector table base
    //
#if defined(ccs) || defined(gcc)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif  //ccs
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif  //ewarm
    
#endif  //USE_TIRTOS
    
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

void doSetup(void)
{
    //
    // Board Initilization
    //
    BoardInit();
    
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();

    PinConfigSet(PIN_58,PIN_STRENGTH_2MA|PIN_STRENGTH_4MA,PIN_TYPE_STD_PD);
    
    //
    // UART Init
    //
    InitTerm();
    
    DisplayBanner(APP_NAME);

    ConfigureGPIO();

    //
    // I2C Init
    //
    if(I2C_IF_Open(I2C_MASTER_MODE_FST) < 0)
    {
        UART_PRINT("I2C cannot be opened\n\r");
    }

    //Init Temprature Sensor
    if(TMP006DrvOpen() < 0)
    {
        UART_PRINT("TMP006 cannot be opened\n\r");
    }

    //Init Accelerometer Sensor
    if(BMA222Open() < 0)
    {
        UART_PRINT("BMA222 cannot be opened\n\r");
    }
}
