//==========================================================================
//
//      devs/watchdog/cortexm/stm32/watchdog_stm32.cxx
//
//      Watchdog implementation for STM32 CPU
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2012 Free Software Foundation, Inc.
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
// Author(s):    Ilija Stanislevik
// Contributors:
// Date:         2012-12-28
// Purpose:      Watchdog class implementation
// Description:  Contains an implementation of the Watchdog class for use
//               with the STM32 Independent Watchdog.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/kernel.h>
#include <pkgconf/infra.h>
#include <pkgconf/kernel.h>
#include <pkgconf/watchdog.h>
#include <pkgconf/devs_watchdog_cortexm_stm32.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_diag.h>

#include <cyg/io/watchdog.hxx>

//#include <cyg/hal/hal_var_ints.h>

//==========================================================================

#define VAL_PRESCALER_BITS(_value_) CYGHWR_HAL_STM32_IWDG_PR_##_value_
#define PRESCALER_BITS(_prescaler_) VAL_PRESCALER_BITS(_prescaler_)

//#define RESOLUTION ( CYGNUM_DEVS_WATCHDOG_CORTEXM_STM32_WD_TIMEOUT_US * 1000 )

//==========================================================================

void
Cyg_Watchdog::init_hw(void)
{
  CYG_REPORT_FUNCTION();
  CYG_REPORT_FUNCARGVOID();

  // Get write access to Prescaler and Reload registers
  HAL_WRITE_UINT32(CYGHWR_HAL_STM32_IWDG + CYGHWR_HAL_STM32_IWDG_KR, CYGHWR_HAL_STM32_IWDG_KR_ACCESS);
  // Write prescaler value
  HAL_WRITE_UINT32(CYGHWR_HAL_STM32_IWDG + CYGHWR_HAL_STM32_IWDG_PR, PRESCALER_BITS(CYGNUM_DEVS_WATCHDOG_CORTEXM_STM32_PRESCALER));
  // Write reload value
  HAL_WRITE_UINT32(CYGHWR_HAL_STM32_IWDG + CYGHWR_HAL_STM32_IWDG_RLR, CYGNUM_DEVS_WATCHDOG_CORTEXM_STM32_RELOAD);

  resolution = CYGNUM_DEVS_WATCHDOG_CORTEXM_STM32_WD_TIMEOUT_US  * 1000LL ;

  CYG_REPORT_RETURN();
}

//==========================================================================
/*
 * Reset watchdog timer. This needs to be called regularly to prevent
 * the watchdog from firing.
 */

void
Cyg_Watchdog::reset(void)
{
  CYG_REPORT_FUNCTION();
  CYG_REPORT_FUNCARGVOID();

  HAL_WRITE_UINT32(CYGHWR_HAL_STM32_IWDG + CYGHWR_HAL_STM32_IWDG_KR, CYGHWR_HAL_STM32_IWDG_KR_RESET);

  CYG_REPORT_RETURN();
}

//==========================================================================
/*
 * Start watchdog to generate a hardware reset
 */

void
Cyg_Watchdog::start(void)
{
  CYG_REPORT_FUNCTION();
  CYG_REPORT_FUNCARGVOID();

  HAL_WRITE_UINT32(CYGHWR_HAL_STM32_IWDG + CYGHWR_HAL_STM32_IWDG_KR, CYGHWR_HAL_STM32_IWDG_KR_START);

  CYG_REPORT_RETURN();
}

//==========================================================================
// End of watchdog_stm32.cxx
