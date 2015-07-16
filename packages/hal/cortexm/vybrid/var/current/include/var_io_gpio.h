#ifndef CYGONCE_HAL_VAR_IO_GPIO_H
#define CYGONCE_HAL_VAR_IO_GPIO_H
//===========================================================================
//
//      var_io_gpio.h
//
//      Vybrid GPIO
//
//===========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2011 Free Software Foundation, Inc.                        
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Antmicro Ltd <contact@antmicro.com>
// Based on:	 {...}/hal/packages/cortexm/kinetis/var/current/include/var_io_devs.h
// Date:         2014-03-28
// Purpose:       Vybrid variant specific registers
// Description:
// Usage:         #include <cyg/hal/var_io.h>  // var_io.h includes this file
//
//####DESCRIPTIONEND####
//
//===========================================================================

//-----------------------------------------------------------------------------
// VYBRID section for GPIO handling per pin operations are covered
// as in VF61GS10 control of subsequent pins from ports e.g A,B,C is distributed between multiple registers
//-----------------------------------------------------------------------------

// the complete maps of available pads arranged with ascending adresses of 
// corresponding IOMUX registers
typedef enum vf61_rgpio_t{
PTA6, PTA8, PTA9, PTA10,PTA11,PTA12,PTA16,PTA17,PTA18,PTA19,PTA20,
PTA21,PTA22,PTA23,PTA24,PTA25,PTA26,PTA27,PTA28,PTA29,PTA30,PTA31,
PTB0, PTB1, PTB2, PTB3, PTB4, PTB5, PTB6, PTB7, PTB8, PTB9, PTB10,
PTB11,PTB12,PTB13,PTB14,PTB15,PTB16,PTB17,PTB18,PTB19,PTB20,PTB21,
PTB22,PTC0, PTC1, PTC2, PTC3, PTC4, PTC5, PTC6, PTC7, PTC8, PTC9,
PTC10,PTC11,PTC12,PTC13,PTC14,PTC15,PTC16,PTC17,PTD31,PTD30,PTD29,
PTD28,PTD27,PTD26,PTD25,PTD24,PTD23,PTD22,PTD21,PTD20,PTD19,PTD18,
PTD17,PTD16,PTD0, PTD1, PTD2, PTD3, PTD4, PTD5, PTD6, PTD7, PTD8,
PTD9, PTD10,PTD11,PTD12,PTD13,PTB23,PTB24,PTB25,PTB26,PTB27,PTB28,
PTC26,PTC27,PTC28,PTC29,PTC30,PTC31,PTE0, PTE1, PTE2, PTE3, PTE4,
PTE5, PTE6, PTE7, PTE8, PTE9, PTE10,PTE11,PTE12,PTE13,PTE14,PTE15,
PTE16,PTE17,PTE18,PTE19,PTE20,PTE21,PTE22,PTE23,PTE24,PTE25,PTE26,
PTE27,PTE28,PTA7,NONE
} vf61_rgpio;


#define CYGHWR_HAL_VYBRID_RGPIO_BASE 0x40048000 //base for IOMUXC
#define CYGHWR_HAL_VYBRID_GPIO_BASE 0x400ff000 //base for GPIO ports

// GPIO pin control registers offsets
#define CYGHWR_HAL_VYBRID_GPIO_PDOR 0x00
#define CYGHWR_HAL_VYBRID_GPIO_PSOR 0x04
#define CYGHWR_HAL_VYBRID_GPIO_PCOR 0x08
#define CYGHWR_HAL_VYBRID_GPIO_PTOR 0x0c
#define CYGHWR_HAL_VYBRID_GPIO_PDIR 0x10

// gets port numer (0,1,2,3) from pin name 
#define CYGHWR_HAL_VYBRID_GET_PORT(__pin) (vf61_rgpio)__pin >> 5

// gets GPIO address from pin name
#define CYGHWR_HAL_VYBRID_GET_GPIO(__pin) (CYGHWR_HAL_VYBRID_GPIO_BASE + 0x40*((vf61_rgpio)__pin >> 5))

// gets bit location from pin name 
#define CYGHWR_HAL_VYBRID_GET_PIN_LOC(__pin) (__pin-32*(__pin>>5))

// gets IOMUXC address from pin name 
#define CYGHWR_HAL_VYBRID_GET_IOMUXC(__pin) CYGHWR_HAL_VYBRID_RGPIO_BASE+(__pin<<2)

// get pin status based onto pin name (returns non zero if pin is '1')
#define CYGHWR_HAL_VYBRID_GPIO_GET_PIN(__pin) \
	(*((volatile cyg_uint32 *)(CYGHWR_HAL_VYBRID_GET_GPIO(__pin) + CYGHWR_HAL_VYBRID_GPIO_PDIR)) & (1 << CYGHWR_HAL_VYBRID_GET_PIN_LOC(__pin)))

// set pin defined by pad name	
#define CYGHWR_HAL_VYBRID_GPIO_SET_PIN(__pin) \
	HAL_WRITE_UINT32(CYGHWR_HAL_VYBRID_GET_GPIO(__pin) + CYGHWR_HAL_VYBRID_GPIO_PSOR, (1 << (CYGHWR_HAL_VYBRID_GET_PIN_LOC(__pin))))

// clear pin defined by pad name	
#define CYGHWR_HAL_VYBRID_GPIO_CLEAR_PIN(__pin) \
	HAL_WRITE_UINT32(CYGHWR_HAL_VYBRID_GET_GPIO(__pin) + CYGHWR_HAL_VYBRID_GPIO_PCOR, (1 << (CYGHWR_HAL_VYBRID_GET_PIN_LOC(__pin))))

// toggle pin defined by pad name	
#define CYGHWR_HAL_VYBRID_GPIO_TOGGLE_PIN(__pin) \
	HAL_WRITE_UINT32(CYGHWR_HAL_VYBRID_GET_GPIO(__pin) + CYGHWR_HAL_VYBRID_GPIO_PTOR, (1 << (CYGHWR_HAL_VYBRID_GET_PIN_LOC(__pin))))

// setup pin definition (mux, direction and so on_
#define CYGHWR_HAL_VYBRID_PIN_SET_FUNCTION(__pin,__settings)\
	HAL_WRITE_UINT32(CYGHWR_HAL_VYBRID_GET_IOMUXC(__pin), __settings)

//
//-----------------------------------------------------------------------------
// end of var_io_gpio.h
#endif // CYGONCE_HAL_VAR_IO_GPIO_H
