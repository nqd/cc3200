// Standard includes
#include <stdlib.h>
// OS includes
#include "osi.h"

#ifdef USE_FREERTOS
//*****************************************************************************
//
//! Application defined hook (or callback) function - the tick hook.
//! The tick interrupt can optionally call this
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationTickHook( void )
{
}

//*****************************************************************************
//
//! Application defined hook (or callback) function - assert
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vAssertCalled( const char *pcFile, unsigned long ulLine )
{
    while(1)
    {

    }
}

//*****************************************************************************
//
//! Application defined idle task hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationIdleHook( void )
{

}

//*****************************************************************************
//
//! Application provided stack overflow hook function.
//!
//! \param  handle of the offending task
//! \param  name  of the offending task
//!
//! \return none
//!
//*****************************************************************************
void
vApplicationStackOverflowHook( OsiTaskHandle *pxTask, signed char *pcTaskName)
{
    ( void ) pxTask;
    ( void ) pcTaskName;

    for( ;; );
}

void vApplicationMallocFailedHook()
{
    while(1)
  {
    // Infinite loop;
  }
}
#endif

