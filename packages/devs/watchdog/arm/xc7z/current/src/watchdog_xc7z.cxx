//==========================================================================
//
//      watchdog_xc7z.cxx
//
//      watchdog driver for the Xilinx Zynq
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    ITR GmbH
// Date:         2012-06-26
// Purpose:
// Description:  Watchdog driver for ARM Cortex-A9 watchdog
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>             // system configuration file
#include <pkgconf/kernel.h>
#include <pkgconf/infra.h>
#include <pkgconf/watchdog.h>
#include <pkgconf/devs_watchdog_xc7z.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/xc7z020.h>
#include <cyg/hal/var_io.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/io.h>

#include <cyg/io/watchdog.hxx>

/* The watchdog uses the slow clock divided by 128. The slow clock is the
 * clock from the 32.768 kHz crystal.
 */
#define CYGNUM_DEVS_WATCHDOG_MAX_LOAD_VALUE             0xFFFFFFFFul

#define CYGNUM_DEVS_WATCHDOG_DIVIDER                    1ul

#define CYGNUM_DEVS_WATCHDOG_CLOCK                 \
   (CYGNUM_HAL_RTC_PERIOD / CYGNUM_DEVS_WATCHDOG_DIVIDER)

/** helper constant for conversion between seconds and milliseconds */
#define CYGNUM_DEVS_WATCHDOG_S_MS                  1000ull

/** calculate the reload value with ceiling, so that the timeout is longer
 * than the configured value, when it is not possible to match the value
 * exactly with the watchdog clock. */
#define CYGNUM_DEVS_WATCHDOG_RELOAD                \
   ((cyg_uint32) (((cyg_uint64) CYGNUM_DEVS_WATCHDOG_ARM_XC7Z_TIMEOUT_MS * CYGNUM_DEVS_WATCHDOG_CLOCK + \
   (CYGNUM_DEVS_WATCHDOG_S_MS - 1ull)) / CYGNUM_DEVS_WATCHDOG_S_MS))

/****************************************************************************/
/**
*
* Watchdog initialize: Initialize WDT timer [A9 WDT timer]
*
* @return   none
*
*****************************************************************************/
void Cyg_Watchdog::init_hw(void)
{
   CYG_REPORT_FUNCTION();
   CYG_REPORT_FUNCARGVOID();

   // Turn off counter
   HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_CONTROL_OFFSET, 0x00);
   HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_ISR_OFFSET,     XSCUWDTIMER_ISR_EVENT_FLAG_MASK);
   HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_RESET_OFFSET,   XSCUWDTIMER_RESET_FLAG_MASK);
   
   resolution = CYGNUM_DEVS_WATCHDOG_ARM_CORTEXA9_TIMEOUT_MS * 1000000ull; // ms -> ns

   CYG_REPORT_RETURN();
}

/****************************************************************************/
/**
*
* Reset watchdog timer. This needs to be called regularly to prevent
*    the watchdog from firing.
*
* @return   none
*
*****************************************************************************/
void Cyg_Watchdog::reset(void)
{
   CYG_REPORT_FUNCTION();
   CYG_REPORT_FUNCARGVOID();

   HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_LOAD_OFFSET, CYGNUM_DEVS_WATCHDOG_RELOAD);

   CYG_REPORT_RETURN();
}

/****************************************************************************/
/**
*
* Start watchdog to generate a hardware reset when expiring.
*
* @return   none
*
*****************************************************************************/
void Cyg_Watchdog::start(void)
{
   CYG_REPORT_FUNCTION();
   CYG_REPORT_FUNCARGVOID();
   
   //
   // Load the WDT counter register.
   //
   HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_LOAD_OFFSET, CYGNUM_DEVS_WATCHDOG_RELOAD);
   
   //
   // Enable WDT counter register.
   //
   HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_CONTROL_OFFSET, 
        (1 << XSCUWDTIMER_CONTROL_PRESCALER_SHIFT) |
        XSCUWDTIMER_CONTROL_WD_MODE_MASK |
        XSCUWDTIMER_CONTROL_AUTO_RELOAD_MASK |
        XSCUWDTIMER_CONTROL_ENABLE_MASK);
        
   CYG_REPORT_RETURN();
}


