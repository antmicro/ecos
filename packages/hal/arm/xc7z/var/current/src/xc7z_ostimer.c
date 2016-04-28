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
    /* Set prescaler value */
    dwVal |= ((CYGHWR_HAL_ARM_SOC_PRIVATE_TIMER_PRESCALER - 1) << XSCUTIMER_CONTROL_PRESCALER_SHIFT);
    /* Enable the decrementer */
    dwVal |= XSCUTIMER_CONTROL_ENABLE_MASK;
    /* Enable the interrupt */
    dwVal |= XSCUTIMER_CONTROL_IRQ_ENABLE_MASK;

    HAL_WRITE_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_CONTROL_OFFSET, dwVal);

    /* Clear INT bit */
    HAL_WRITE_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_ISR_OFFSET,
                     XSCUTIMER_ISR_EVENT_FLAG_MASK);

    /* Interrupt for private timer always enabled in GIC */

    /* Initialize TTC Counter1 to be used for hal_delay_us */
    hal_ttc_counter_init();

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
    //
    // Private timer counts down. So need to convert value to eCOS requirements.
    //
    HAL_READ_UINT32(XC7Z_SCU_TIMER_BASEADDR + XSCUTIMER_COUNTER_OFFSET, i);
    *pvalue = _period - i;
}


/****************************************************************************/
/**
*
* HAL Triple Timer Counter init
*
* @param    none
*
* @return   none
*
* *****************************************************************************/
void hal_ttc_counter_init()
{
    cyg_uint32 reg_val;
    cyg_uint32 ticks_per_us;

    /* Determine CPU Clock Ratio Mode */
    HAL_READ_UINT32(XC7Z_SYS_CTRL_BASEADDR + 0x1C4, reg_val);
    if (reg_val & 0x01)
    {
        ticks_per_us = CYGHWR_HAL_ARM_SOC_PROCESSOR_CLOCK/6000000;
    } else {
        ticks_per_us = CYGHWR_HAL_ARM_SOC_PROCESSOR_CLOCK/4000000;
    }

    /* Stop TTC counter before initialization */
    HAL_READ_UINT32(XC7Z_TTC0_BASEADDR + XTTC_COUNTER_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);
    reg_val |= (1 << XTTC_CNTCONTROL_DIS_SHIFT);
    HAL_WRITE_UINT32(XC7Z_TTC0_BASEADDR + XTTC_COUNTER_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);

    /* Setup Counter Control register */
    HAL_READ_UINT32(XC7Z_TTC0_BASEADDR + XTTC_COUNTER_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);
    /* Set DECR to 1 - enable decrementing */
    reg_val |= (1 << XTTC_CNTCONTROL_DECR_SHIFT);
    /* Set INT to 1 - enable interval mode */
    reg_val |= (1 << XTTC_CNTCONTROL_INT_SHIFT);
    HAL_WRITE_UINT32(XC7Z_TTC0_BASEADDR + XTTC_COUNTER_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);

    /* Setup Clock Control register */
    /* Clear PS_EN bit - disable prescaler to get clock equal to CPU_1x */
    HAL_READ_UINT32(XC7Z_TTC0_BASEADDR + XTTC_CLOCK_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);
    reg_val &= XTTC_CLKCONTROL_PRE_EN_CLR_MASK;
    HAL_WRITE_UINT32(XC7Z_TTC0_BASEADDR + XTTC_CLOCK_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);

    /* Setup Interrupt Enable register */
    /*
        Set interrupt enable for interval so the interval interrupt bit is set
        each time the TTC counter reach zero
    */
    HAL_READ_UINT32(XC7Z_TTC0_BASEADDR + XTTC_INTERRUPT_EN_OFFSET + XTTC_COUNTER_1, reg_val);
    reg_val |= (1 << XTTC_INTEN_INTERVAL_SHIFT);
    HAL_WRITE_UINT32(XC7Z_TTC0_BASEADDR + XTTC_INTERRUPT_EN_OFFSET + XTTC_COUNTER_1, reg_val);

    /* Setup Interval Counter register */
    /* Set interval to value equal to 1 microsecond */
    reg_val = ticks_per_us & XTTC_INTERVAL_VALUE_MASK;
    HAL_WRITE_UINT32(XC7Z_TTC0_BASEADDR + XTTC_INTERVAL_COUNTER_OFFSET + XTTC_COUNTER_1, reg_val);
}

/****************************************************************************/
/**
*
* HAL us delay
*
* @param    usecs - number of usecs for delay.
*
* @return   none
*
* *****************************************************************************/
void hal_delay_us(cyg_int32 usecs)
{
    // start counting
    cyg_uint32 reg_val;
    HAL_READ_UINT32(XC7Z_TTC0_BASEADDR + XTTC_COUNTER_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);
    reg_val &= XTTC_CNTCONTROL_DIS_CLR_MASK;
    HAL_WRITE_UINT32(XC7Z_TTC0_BASEADDR + XTTC_COUNTER_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);

    do {
        // wait until interval interrupt bit is set
        do {
            HAL_READ_UINT32(XC7Z_TTC0_BASEADDR + XTTC_INTERRUPT_OFFSET + XTTC_COUNTER_1, reg_val);
        } while (!(reg_val & XTTC_INT_INTERVAL_MASK));

        usecs--;
    } while(usecs);

    // stop counting
    HAL_READ_UINT32(XC7Z_TTC0_BASEADDR + XTTC_COUNTER_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);
    reg_val |= XTTC_CNTCONTROL_DIS_SET_MASK;
    HAL_WRITE_UINT32(XC7Z_TTC0_BASEADDR + XTTC_COUNTER_CONTROL_OFFSET + XTTC_COUNTER_1, reg_val);
}

// xc7z_ostimer.c
