//==========================================================================
//
//      vybrid_misc.c
//
//      Cortex-M Vybrid HAL functions
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2010, 2013 Free Software Foundation, Inc.                        
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
// Author(s):    Antmicro Ltd <contact@antmicro.com>
// Based on:	 {...}/hal/packages/cortexm/kinetis/var/current/include/kinetis_misc.h
// Date:         2014-03-28
// Description:
//
//####DESCRIPTIONEND####
//
//========================================================================
 

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm.h>
#include <pkgconf/hal_cortexm_vybrid.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/cortexm_endian.h>
#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL header
#include <cyg/hal/hal_if.h>             // HAL header

#include <cyg/hal/hal_cache.h>

#include <cyg/hal/var_io_gpio.h>
#include <assert.h>

void sst25xx_freescale_dspi_reg(void);

//==========================================================================
// Setup variant specific hardware
//=========================================================================


const char vf61_pads[] ={
// port A
NONE, NONE, NONE, NONE, NONE, NONE, PTA6, PTA7,
PTA8, PTA9, PTA10,PTA11,PTA12,NONE, NONE, NONE,
PTA16,PTA17,PTA18,PTA19,PTA20,PTA21,PTA22,PTA23,
PTA24,PTA25,PTA26,PTA27,PTA28,PTA29,PTA30,PTA31,
// port B
PTB0, PTB1, PTB2, PTB3, PTB4, PTB5, PTB6, PTB7,
PTB8, PTB9, PTB10,PTB11,PTB12,PTB13,PTB14,PTB15,
PTB16,PTB17,PTB18,PTB19,PTB20,PTB21,PTB22,PTB23,
PTB24,PTB25,PTB26,PTB27,PTB28,NONE, NONE, NONE, 
// port C
PTC0, PTC1, PTC2, PTC3, PTC4, PTC5, PTC6, PTC7,
PTC8, PTC9, PTC10,PTC11,PTC12,PTC13,PTC14,PTC15,
PTC16,PTC17,NONE, NONE, NONE, NONE, NONE, NONE,
NONE, NONE, PTC26,PTC27,PTC28,PTC29,PTC30,PTC31,
// port D
PTD0, PTD1, PTD2, PTD3, PTD4, PTD5, PTD6, PTD7,
PTD8, PTD9, PTD10,PTD11,PTD12,PTD13,NONE, NONE,
PTD16,PTD17,PTD18,PTD19,PTD20,PTD21,PTD22,PTD23,
PTD24,PTD25,PTD26,PTD27,PTD28,PTD29,PTD30,PTD31,
// port E
PTE0, PTE1, PTE2, PTE3, PTE4, PTE5, PTE6, PTE7, 
PTE8, PTE9, PTE10,PTE11,PTE12,PTE13,PTE14,PTE15, 
PTE16,PTE17,PTE18,PTE19,PTE20,PTE21,PTE22,PTE23,
PTE24,PTE25,PTE26,PTE27,PTE28,NONE, NONE, NONE
};

void hal_variant_init( void )
{
#if defined CYGPKG_HAL_VYBRID_CACHE
# if defined CYG_HAL_STARTUP_RAM
    register CYG_INTERRUPT_STATE oldints;
# endif
#endif

    hal_update_clock_var(); //VYBRID

#if defined CYGPKG_HAL_VYBRID_CACHE
# if defined CYG_HAL_STARTUP_RAM
    HAL_DISABLE_INTERRUPTS(oldints);
    HAL_DCACHE_SYNC();
    HAL_DCACHE_DISABLE();
    HAL_DCACHE_PURGE_ALL();
    HAL_ICACHE_DISABLE();
    HAL_ICACHE_INVALIDATE_ALL();
# endif // defined CYG_HAL_STARTUP_RAM
# if defined CYG_HAL_STARTUP_RAM
    HAL_RESTORE_INTERRUPTS(oldints);
# endif
# ifdef CYGSEM_HAL_ENABLE_ICACHE_ON_STARTUP
    HAL_ICACHE_ENABLE();
# endif
# ifdef CYGSEM_HAL_ENABLE_DCACHE_ON_STARTUP
    HAL_DCACHE_ENABLE();
# endif
#endif // defined CYGPKG_HAL_VYBRID_CACHE

#ifdef CYGSEM_HAL_VIRTUAL_VECTOR_SUPPORT
    hal_if_init();
#endif
}

//==========================================================================
// Pin configuration functions
//==========================================================================

static cyghwr_hal_vybrid_port_t * const Ports[] = {
    CYGHWR_HAL_VYBRID_PORTA_P, CYGHWR_HAL_VYBRID_PORTB_P,
    CYGHWR_HAL_VYBRID_PORTC_P, CYGHWR_HAL_VYBRID_PORTD_P,
    CYGHWR_HAL_VYBRID_PORTE_P, CYGHWR_HAL_VYBRID_PORTF_P
};


void
hal_set_pin_function(cyg_uint32 pin)
{	
	if (pin==CYGHWR_HAL_VYBRID_PORT_PIN_NONE) return;
    
	cyg_uint32 func = CYGHWR_HAL_VYBRID_PIN_FUNC(pin);	
	cyg_uint32 pad = vf61_pads[CYGHWR_HAL_VYBRID_PIN_PORT(pin)*32+CYGHWR_HAL_VYBRID_PIN_BIT(pin)];
	
	// pad must exist in the device
	assert(pad != NONE);
	
	cyg_uint32 mux_val, mux_cnf;
	
	HAL_READ_UINT32(CYGHWR_HAL_VYBRID_GET_IOMUXC(pad),mux_val);
	
	mux_val &= 0xff8fffff; // clear the MUX_MODE
	
	mux_val |= ((func & 0xf00) << 12); //assign new MUX_MODE

	mux_cnf = pin & 0x0000003f; // extract cnf from pin definition
	
	if (mux_cnf) // non zero cnf assigned
	{		
		mux_val &= 0xffffffc0; 
		mux_val |= mux_cnf; 
	}
	
	HAL_WRITE_UINT32(CYGHWR_HAL_VYBRID_GET_IOMUXC(pad),mux_val);    
}

void
hal_dump_pin_function(cyg_uint32 pin)
{
	if(pin == CYGHWR_HAL_VYBRID_PIN_NONE) return;
	
	cyg_uint32 port = CYGHWR_HAL_VYBRID_PIN_PORT(pin);
    cyg_uint32 bit =  CYGHWR_HAL_VYBRID_PIN_BIT(pin);

	cyg_uint32 pad = vf61_pads[CYGHWR_HAL_VYBRID_PIN_PORT(pin)*32+CYGHWR_HAL_VYBRID_PIN_BIT(pin)];	
	cyg_uint32 mux_val;	
	HAL_READ_UINT32(CYGHWR_HAL_VYBRID_GET_IOMUXC(pad),mux_val);	
	diag_printf("Pin PT%c%d: IOMUX=0x08%x\n",0x41+port,bit,mux_val);
}

inline void
hal_gpio_set_pin(cyg_uint32 pin)
{
	CYGHWR_HAL_VYBRID_GPIO_SET_PIN(vf61_pads[CYGHWR_HAL_VYBRID_PIN_PORT(pin)*32+CYGHWR_HAL_VYBRID_PIN_BIT(pin)]);
}

inline void
hal_gpio_clear_pin(cyg_uint32 pin)
{
	CYGHWR_HAL_VYBRID_GPIO_CLEAR_PIN(vf61_pads[CYGHWR_HAL_VYBRID_PIN_PORT(pin)*32+CYGHWR_HAL_VYBRID_PIN_BIT(pin)]);
}

inline void
hal_gpio_toggle_pin(cyg_uint32 pin)
{
	CYGHWR_HAL_VYBRID_GPIO_TOGGLE_PIN(vf61_pads[CYGHWR_HAL_VYBRID_PIN_PORT(pin)*32+CYGHWR_HAL_VYBRID_PIN_BIT(pin)]);
}

inline cyg_uint32
hal_gpio_get_pin(cyg_uint32 pin)
{
	return CYGHWR_HAL_VYBRID_GPIO_GET_PIN(vf61_pads[CYGHWR_HAL_VYBRID_PIN_PORT(pin)*32+CYGHWR_HAL_VYBRID_PIN_BIT(pin)])?1:0;
}
//==========================================================================
// VYBRID Clock distribution
//==========================================================================

void
hal_clock_enable(cyg_uint32 ccgr)
{
    volatile cyg_uint32 *ccm_p;

    if(ccgr != CYGHWR_HAL_CCGR_NONE) {
        ccm_p = &CYGHWR_HAL_VYBRID_CCM_P->ccgr0 +
                 CYGHWR_HAL_VYBRID_CCM_CCGR_REG(ccgr);
        *ccm_p |= 3 << CYGHWR_HAL_VYBRID_CCM_CCGR_BIT(ccgr);
    }
}

void
hal_clock_disable(cyg_uint32 ccgr)
{
    volatile cyg_uint32 *ccm_p;

    if(ccgr != CYGHWR_HAL_CCGR_NONE) {
        ccm_p = &CYGHWR_HAL_VYBRID_CCM_P->ccgr0 +
                 CYGHWR_HAL_VYBRID_CCM_CCGR_REG(ccgr);
        *ccm_p &= ~(3 << CYGHWR_HAL_VYBRID_CCM_CCGR_BIT(ccgr));
    }
}

//==========================================================================
// VYBRID Misc functions
//==========================================================================

int
hal_get_core_num(void)
{
    cyg_uint32 reg;
    HAL_READ_UINT32(CYGHWR_HAL_VYBRID_MSCM_CPxNUM, reg);

    return (reg & CYGHWR_HAL_VYBRID_MSCM_CPxNUM_CPN_M);
}

//==========================================================================
// EOF vybrid_misc.c
