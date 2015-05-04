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
// Date:         2015-04-28
// Description:  Based on hwz7zc702 platform HAL
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <string.h>

#include <sys/reent.h>
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

#define errno (*__errno())


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

// -------------------------------------------------------------------------
// Helper functions

#if (__GNUC__ >= 3)

// Versions of gcc/g++ after 3.0 (approx.), when configured for Linux
// native development (specifically, --with-__cxa_enable), have
// additional dependencies related to the destructors for static
// objects. When compiling C++ code with static objects the compiler
// inserts a call to __cxa_atexit() with __dso_handle as one of the
// arguments. __cxa_atexit() would normally be provided by glibc, and
// __dso_handle is part of crtstuff.c. Synthetic target applications
// are linked rather differently, so either a differently-configured
// compiler is needed or dummy versions of these symbols should be
// provided. If these symbols are not actually used then providing
// them is still harmless, linker garbage collection will remove them.

// gcc 3.2.2 (approx). The libsupc++ version of the new operator pulls
// in exception handling code, even when using the nothrow version and
// building with -fno-exceptions. libgcc_eh.a provides the necessary
// functions, but requires a dl_iterate_phdr() function. That is related
// to handling dynamically loaded code so is not applicable to eCos.
int
dl_iterate_phdr(void* arg1, void* arg2)
{
    return -1;
}

struct _reent impure_data = { 0, 0, "", 0, "C"}; // stub for gcc 4.6.1

struct _reent* _impure_ptr = &impure_data;

int* __errno ()
{
    return &_impure_ptr->_errno;
}

#endif

#ifndef CYGPKG_LIBC_STDIO
//-------------------------------------------------------------------------
//sprintf function stub (library libsupc++ needs it)
int
sprintf(const char * format, ...)
{
    return 0;
}
#endif

//-------------------------------------------------------------------------
//reset platfrom
void hal_reset(void)
{
    /* Unlock SLCR regs */
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCR_UNLOCK_OFFSET, XSLCR_UNLOCK_KEY);
    
    /* Tickle soft reset bit */
    HAL_WRITE_UINT32(0xF8000200, 1);

    while (1);
}

//--------------------------------------------------------------------------
// EOF mars_zx3_misc.c
