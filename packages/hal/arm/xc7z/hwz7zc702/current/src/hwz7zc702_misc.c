/**************************************************************************/
/**
*
* @file     hwz7zc702_misc.c
*
* @brief    HAL misc board support code for Xilinx HWZ7ZC702 board
*
***************************************************************************/
/*==========================================================================
//
//      hwz7zc702_misc.c
//
//      HAL misc board support code for Xilinx HWZ7ZC702 board
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
#include <pkgconf/hal.h>

#include <sys/reent.h>

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>

#define errno (*__errno())

//extern int *__errno ( void );

// -------------------------------------------------------------------------
// Hardware init

void
hal_plf_hardware_init (void)
{
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

//--------------------------------------------------------------------------
// EOF hwz7zc702_misc.c
