/**************************************************************************/
/**
*
* @file     xc7z_ostimer.c
*
* @brief    XC7Z Cortex-A9 OS timer [Private Timer] functions
*
***************************************************************************/
/*==========================================================================
//
//      xc7z_ostimer.c
//
//      HAL timer code using the Private Timer Counter
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    ITR-GmbH
// Date:         2012-06-25
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================
*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_intr.h>           // necessary?
#include <cyg/hal/var_io.h>

/* Internal tick units */
static cyg_uint32 _period;

/****************************************************************************/
/**
*
* HAL clock initialize: Initialize OS timer [A9 Private timer]
*
* @param    period - value for load to private timer.
*
* @return   none
*
*****************************************************************************/

void hal_clock_initialize(cyg_uint32 period)
{
    unsigned long dwVal;
    /*
     * Load the timer counter register.
     */
    HAL_WRITE_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_LOAD_OFFSET, period);
    /*
      If main CPU clock is 667MHz, prescaler 1:2, so TIMERCLK = 167 MHz
    */

    /*
     * Start the A9Timer device.
     */
    HAL_READ_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_CONTROL_OFFSET, dwVal);
    /* Enable Auto reload mode.  */
    dwVal |= XSCUTIMER_CONTROL_AUTO_RELOAD_MASK;
    /* Clear prescaler control bits */
    dwVal &= ~XSCUTIMER_CONTROL_PRESCALER_MASK;
    /* Set prescaler value /2 */
    dwVal |= (1 << XSCUTIMER_CONTROL_PRESCALER_SHIFT);
    /* Enable the decrementer */
    dwVal |= XSCUTIMER_CONTROL_ENABLE_MASK;
    /* Enable the interrupt */
    dwVal |= XSCUTIMER_CONTROL_IRQ_ENABLE_MASK;
    
    HAL_WRITE_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_CONTROL_OFFSET, dwVal);
    
    /* Clear INT bit */
    HAL_WRITE_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_ISR_OFFSET, 
                     XSCUTIMER_ISR_EVENT_FLAG_MASK);
    
    /* Interrupt for private timer always enabled in GIC */
    
    _period = period;
}

/****************************************************************************/
/**
*
* HAL clock reset handler: Reset OS timer [A9 Private timer]
*
* @param    vector - interrupt number of private timer.
* @param    period - value for load to private timer.
*
* @return   none
*
*****************************************************************************/
void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
    HAL_WRITE_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_ISR_OFFSET, XSCUTIMER_ISR_EVENT_FLAG_MASK);
	//cyg_uint32 period = 100000;	
    
    // Clear pending interrupt bit
    HAL_INTERRUPT_ACKNOWLEDGE(vector);
    
    if (period != _period)
        {
            HAL_WRITE_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_LOAD_OFFSET, period);
            
            _period = period;
        }
}

/****************************************************************************/
/**
*
* HAL clock read
*
* @param    pvalue - pointer to cyg_uint32 variable for filling current timer value.
*
* @return   none
*
*****************************************************************************/
void hal_clock_read(cyg_uint32 *pvalue)
{
    cyg_uint32 i;
    //TODO: check this fcn, 
    //	if _period is ok ? right now it's hardcoded to 76667 in hal_hardware_init 
    //
    // Private timer counts down. So need to convert value to eCOS requirements.
    //
    HAL_READ_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_COUNTER_OFFSET, i);
    //diag_printf("%s: period=%d, ret=%d\n",__func__,_period, _period-i);
    *pvalue = _period - i;
}

/****************************************************************************/
/**
*
* HAL us delay
* (CPUCLK = 667MHz)
*
* @param    usecs - number of usecs for delay.
*
* @return   none
*
*****************************************************************************/
#define US_LIMIT 75 // TODO: should depend on clock settings

void hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 stat0, stat, i;

    // Wait for the compare
    for (i = 0; i < usecs; i++)
    {
        hal_clock_read(&stat0);

        do {
            hal_clock_read(&stat);
        } while ((stat - stat0) < US_LIMIT);
    }
}

// xc7z_ostimer.c
