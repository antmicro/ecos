//*==========================================================================
//
//      hwz7zc702_redboot.c
//
//      Redboot support code for Xilinx HWZ7ZC702 board.
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2006 Free Software Foundation, Inc.
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
// Date:         2012-06-27
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/
#include <pkgconf/system.h>
#ifdef CYGPKG_REDBOOT

#include <pkgconf/hal.h>
#include <cyg/hal/hal_io.h>             // IO macros
#include <redboot.h>
#include CYGHWR_MEMORY_LAYOUT_H

#define CYGNUM_DEVS_WATCHDOG_RELOAD                \
   ((cyg_uint32) (((cyg_uint64) CYGNUM_DEVS_WATCHDOG_ARM_CORTEXA9_TIMEOUT_MS * CYGNUM_DEVS_WATCHDOG_CLOCK + \
   (CYGNUM_DEVS_WATCHDOG_S_MS - 1ull)) / CYGNUM_DEVS_WATCHDOG_S_MS))

//--------------------------------------
// Memory section for loading applications
//--------------------------------------
EXTERN void
cyg_plf_memory_segment(int seg, unsigned char **start, unsigned char **end)
{
    *start = CYGMEM_REGION_sdram;
    *end   = CYGMEM_REGION_sdram + CYGMEM_REGION_sdram_SIZE - BOOTLOADER_RAM_SIZE - 1;
}

//--------------------------------------
// Watchdog
//--------------------------------------
/* Kick the watchdog. */
static void
hwz7zc702_kick_watchdog(bool is_idle)
{
    /* Write magic code to reset the watchdog. */
    HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_LOAD_OFFSET, CYGNUM_DEVS_WATCHDOG_RELOAD);
}

// Add an idle function to be run by RedBoot when idle
RedBoot_idle(hwz7zc702_kick_watchdog, RedBoot_IDLE_BEFORE_NETIO);

/* Watchdog command for Redboot. */

/** calculate the reload value with ceiling, so that the timeout is longer
 * than the configured value, when it is not possible to match the value
 * exactly with the watchdog clock. */
static void
hwz7zc702_cmd_watchdog(int argc, char *argv[])
{
    cyg_uint32 reg_val;

    if (argc == 2 && strcasecmp(argv[1], "off") == 0) {
        HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_CONTROL_OFFSET, 0x00);
        HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_DISABLE_OFFSET, XSCUWDTIMER_WD_DISABLE_SEQ1); 
        HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_DISABLE_OFFSET, XSCUWDTIMER_WD_DISABLE_SEQ2);
        HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_ISR_OFFSET,     XSCUWDTIMER_ISR_EVENT_FLAG_MASK);
        HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_RESET_OFFSET,   XSCUWDTIMER_RESET_FLAG_MASK);
        argc--;
    }

    if (argc == 1) {
        HAL_READ_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_CONTROL_OFFSET, reg_val);
        diag_printf("Watchdog is: %s\n",
                    (reg_val & XSCUWDTIMER_CONTROL_ENABLE_MASK) ? "off" : "on");
    }
    else
        diag_printf("Invalid watchdog command option\n");
}

// Add CLI command 'watchdog' as defined in this file
RedBoot_cmd("watchdog",
            "Display watchdog status or disable watchdog",
            "[off]",
            hwz7zc702_cmd_watchdog
    );
#endif
//--------------------------------------------------------------------------
// EOF hwz7zc702_redboot.c
