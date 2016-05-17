/*==========================================================================
//
//      mercury_zx3_misc.c
//
//      HAL misc board support for Enclustra Mercury ZX1 module
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
// Author(s):    Antmicro Ltd <www.antmicro.com>
// Contributors: Deimos Space <www.deimos-space.com>
// Date:         2015-04-28
// Description:  Based on hwz7zc702 platform HAL
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <string.h>

#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_diag.h>

#include <cyg/hal/hal_mmu.h>
#include <cyg/hal/var_mmu.h>


#ifdef CYGHWR_DEVS_QSPI_ARM_XC7Z
#ifdef CYGPKG_DEVS_FLASH_SPI_M25PXX
#include <cyg/io/m25pxx.h>
#include <cyg/io/qspi_xc7z.h>

cyg_qspi_xc7z_device_t m25pxx_spi_device = {
    .qspi_device.spi_bus = &cyg_qspi_xc7z_bus0.qspi_bus,
    .dev_num = 0,
    .cl_pol = 1,
    .cl_pha = 1,
    .cl_brate = 25000000,                // Nominal 25MHz.
    .cs_up_udly = 1,
    .cs_dw_udly = 1,
    .tr_bt_udly = 1,
};

CYG_DEVS_FLASH_SPI_M25PXX_DRIVER (
    m25pxx_flash_device, 0, &m25pxx_spi_device
);

#endif
#endif

// -------------------------------------------------------------------------
// Hardware init

void hal_plf_hardware_init(void) {
#ifndef CYGPKG_IO_WATCHDOG
  /* Disable the watchdog. The eCos philosophy is that the watchdog is
     disabled unless the watchdog driver is used to enable it.
     Whoever if we disable it here we cannot re-enable it in the
     watchdog driver, hence the conditional compilation. */
  HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_DISABLE_OFFSET, XSCUWDTIMER_WD_DISABLE_SEQ1); 
  HAL_WRITE_UINT32(XC7Z_SCU_WDT_BASEADDR + XSCUWDTIMER_DISABLE_OFFSET, XSCUWDTIMER_WD_DISABLE_SEQ2); 
#endif

#ifdef CYGHWR_HAL_ARM_ENABLE_FPU
    // Enable FPU

    // ARM Cortex-A9 NEON Media Processing Engine
    // Revision: r3p0
    // Technical Reference Manual
    //
    // Code:
    //
    // MRC p15,0,r0,c1,c0,2   ; Read CPACR into r0
    // ORR r0,r0,#(3<<20)     ; OR in User and Privileged access for CP10
    // ORR r0,r0,#(3<<22)     ; OR in User and Privileged access for CP11
    // BIC r0, r0, #(3<<30)   ; Clear ASEDIS/D32DIS if set
    // MCR p15,0,r0,c1,c0,2   ; Store new access permissions into CPACR
    // ISB                    ; Ensure side-effect of CPACR is visible
    // MOV r0,#(1<<30)        ; Create value with FPEXC (bit 30) set in r0
    // VMSR FPEXC,r0          ; Enable VFP and SIMD extensions

    asm volatile ("MRC p15,0,r0,c1,c0,2");
    asm volatile ("ORR r0,r0,#(3<<20)");
    asm volatile ("ORR r0,r0,#(3<<22)");
    asm volatile ("BIC r0, r0, #(3<<30)");
    asm volatile ("MCR p15,0,r0,c1,c0,2");
    asm volatile ("ISB");
    asm volatile ("MOV r0,#(1<<30)");
    asm volatile ("VMSR FPEXC,r0");
#endif

}

// -------------------------------------------------------------------------
// MMU init

static cyg_uint32 mmu_table[4096] __attribute__ ((section (".mmu_tables")));

void hal_mmu_init(void) {

    cyg_uint32 reg;
    cyg_uint32 ttb_base = (cyg_uint32)mmu_table;

    // Set the TTB registers
    asm volatile ("mcr  p15,0,%0,c2,c0,0" : : "r"(ttb_base));
    asm volatile ("mcr  p15,0,%0,c2,c0,1" : : "r"(ttb_base));
    reg = 0;
    asm volatile ("mcr  p15,0,%0,c2,c0,2" : : "r"(reg));

    // Set the Domain Access Control to manager mode
    reg = -1;
    asm volatile ("mcr  p15,0,%0,c3,c0,0" : : "r"(reg));

    // First clear all entries
    memset(mmu_table, 0, sizeof(mmu_table));

    //                Actual   Virtual  Size  Attributes
    //                Base     Base     MB    cached?              buffered?             access permissions
    //                xxx00000 xxx00000
    ARC_X_ARM_MMU_SECTION(0x000, 0x000, 4096,   ARC_ARM_CACHEABLE,   ARC_ARM_BUFFERABLE, ARC_ARM_ACCESS_PERM_RW_RW); //RAM
    ARC_X_ARM_MMU_SECTION(0xE00, 0xE00,    3, ARC_ARM_UNCACHEABLE, ARC_ARM_UNBUFFERABLE, ARC_ARM_ACCESS_PERM_RW_RW); //IO
    ARC_X_ARM_MMU_SECTION(0xF80, 0xF80,   16, ARC_ARM_UNCACHEABLE, ARC_ARM_UNBUFFERABLE, ARC_ARM_ACCESS_PERM_RW_RW); //System registers
    ARC_X_ARM_MMU_SECTION(0xFFF, 0xFFF,    1, ARC_ARM_UNCACHEABLE, ARC_ARM_UNBUFFERABLE, ARC_ARM_ACCESS_PERM_RW_RW); //OCM

}

//--------------------------------------------------------------------------
// EOF mars_zx3_misc.c
