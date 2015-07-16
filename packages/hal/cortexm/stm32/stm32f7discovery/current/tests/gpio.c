/*=============================================================================
//
//      gpio.c
//
//      Test for STM32F4-Discovery GPIO
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2013 Free Software Foundation, Inc.                        
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    jld
// Date:         2013-06-07
//              
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/infra/testcase.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/var_io.h>

externC void cyg_start(void) {
    int b, n;
    CYG_TEST_INIT();
    CYG_TEST_INFO( "Starting STM32F4-Discovery GPIO test" );
    CYG_TEST_INFO( "Press and hold user button for slow LED count" );
    for ( n = 0; n < 0x100; n++ ) {
    	// display least significant 4 bits of count on user LEDs
        CYGHWR_HAL_STM32_GPIO_OUT( CYGHWR_HAL_STM32F4DISCOVERY_LED1, 0 != (n & 0x1) );
        CYGHWR_HAL_STM32_GPIO_OUT( CYGHWR_HAL_STM32F4DISCOVERY_LED2, 0 != (n & 0x2) );
        CYGHWR_HAL_STM32_GPIO_OUT( CYGHWR_HAL_STM32F4DISCOVERY_LED3, 0 != (n & 0x4) );
        CYGHWR_HAL_STM32_GPIO_OUT( CYGHWR_HAL_STM32F4DISCOVERY_LED4, 0 != (n & 0x8) );
        // extend delay from 125ms to 500ms when user button pressed
        CYGHWR_HAL_STM32_GPIO_IN( CYGHWR_HAL_STM32F4DISCOVERY_BTN1, &b );
        HAL_DELAY_US( 125000 * (1 + ( (b & 1) * 3) ) );
    }
    CYG_TEST_PASS_FAIL( 1, "STM32F4-Discovery GPIO test" );
    CYG_TEST_FINISH( "STM32F4-Discovery GPIO test" );
}

//=============================================================================
// EOF gpio.c
