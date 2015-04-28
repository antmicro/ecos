/*==========================================================================
//
//      stm32f4discovery_misc.c
//
//      Cortex-M4 STM32F4-Discovery HAL functions
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2008, 2011, 2012, 2013 Free Software Foundation, Inc.
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
// Author(s):    jld
// Based on:     stm32x0g_eval misc setup by jlarmour
// Date:         2013-06-05
// Description:
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm.h>
#include <pkgconf/hal_cortexm_stm32.h>
#include <pkgconf/hal_cortexm_stm32_stm32f4discovery.h>

#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_if.h>

//==========================================================================

#if defined(CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT)
#if (CYGNUM_CALL_IF_TABLE_SIZE > 64)
// We force a compilation error for this fatal condition since run-time asserts
// may not be enabled for the build.
#error "The CALL_IF_TABLE_SIZE pre-allocation in the linker scripts for this platform need to be updated"
#endif
#endif // CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT

//==========================================================================
// System init
//
// This is run to set up the basic system, including GPIO setting,
// clock feeds, power supply, and memory initialization. This code
// runs before the DATA is copied from ROM and the BSS cleared, hence
// it cannot make use of static variables or data tables.

__externC void hal_system_init( void )
{
    CYG_ADDRESS base;

    // Enable CCM clock and any required GPIO ports in RCC
    base = CYGHWR_HAL_STM32_RCC;
    HAL_WRITE_UINT32(base+CYGHWR_HAL_STM32_RCC_AHB1ENR,
                     BIT_(CYGHWR_HAL_STM32_RCC_AHB1ENR_CCMDATARAMEN) |
                     BIT_(CYGHWR_HAL_STM32_RCC_AHB1ENR_GPIOA) |
                     BIT_(CYGHWR_HAL_STM32_RCC_AHB1ENR_GPIOC) |
                     BIT_(CYGHWR_HAL_STM32_RCC_AHB1ENR_GPIOD) );

    // Set unused lines on enabled GPIO ports to input with pull down

    // GPIO Port A - setup PA0 for button, PA9 for LED, PA13,14 for SWD
    base = CYGHWR_HAL_STM32_GPIOA;
    HAL_WRITE_UINT32( base+CYGHWR_HAL_STM32_GPIO_PUPDR, 0x82A8AAA8 );
    HAL_WRITE_UINT32( base+CYGHWR_HAL_STM32_GPIO_MODER, 0x28040000 );

    // GPIO Port C - setup PC10,11 for RS232 (UART4)
    base = CYGHWR_HAL_STM32_GPIOC;
    HAL_WRITE_UINT32( base+CYGHWR_HAL_STM32_GPIO_PUPDR, 0xAA0AAAAA );
    HAL_WRITE_UINT32( base+CYGHWR_HAL_STM32_GPIO_MODER, 0x00A00000 );

    // GPIO Port D - setup PD5,12,13,14,15 for LEDs
    base = CYGHWR_HAL_STM32_GPIOD;
    HAL_WRITE_UINT32( base+CYGHWR_HAL_STM32_GPIO_PUPDR, 0x00AAA2AA );
    HAL_WRITE_UINT32( base+CYGHWR_HAL_STM32_GPIO_MODER, 0x55000400 );

    // Enable flash prefetch buffer, cacheability and set latency to 2 wait states
    // Latency has to be set before clock is switched to a higher speed
    {
        cyg_uint32 acr;

        base = CYGHWR_HAL_STM32_FLASH;

        HAL_READ_UINT32( base+CYGHWR_HAL_STM32_FLASH_ACR, acr );
        acr |= CYGHWR_HAL_STM32_FLASH_ACR_PRFTEN;
        acr |= CYGHWR_HAL_STM32_FLASH_ACR_DCEN|CYGHWR_HAL_STM32_FLASH_ACR_ICEN;
        acr |= CYGHWR_HAL_STM32_FLASH_ACR_LATENCY(CYGNUM_HAL_CORTEXM_STM32_FLASH_WAIT_STATES);
        HAL_WRITE_UINT32( base+CYGHWR_HAL_STM32_FLASH_ACR, acr );
    }
}

//==========================================================================

__externC void hal_platform_init( void )
{
#ifdef CYGDBG_USE_ASSERTS
    __externC char __sram_data_start[];
#endif

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    // Check the number of VSRs matches the linker script. We can do this
    // because we intend the VV table to follow the VSR table with no gaps.
    CYG_ASSERT( (char*)&hal_virtual_vector_table[0] - (char*)&hal_vsr_table >= CYGNUM_HAL_VSR_COUNT*4,
                "VSR table size does not match" );
    // Now check the declared start of SRAM data follows the VV table end
    CYG_ASSERT( (__sram_data_start - (char*)&hal_virtual_vector_table[0]) >= CYGNUM_CALL_IF_TABLE_SIZE*4,
                "VV table size does not match sram space" );
#else
    // Check the VSR table fits below declared start of SRAM data
    CYG_ASSERT( (__sram_data_start - (char*)&hal_vsr_table[0]) >= CYGNUM_HAL_VSR_COUNT*4,
                "VSR table size does not match" );
#endif
}

//==========================================================================
// EOF stm32f4discovery_misc.c
