//==========================================================================
//
//      kinetis_ddram.c
//
//      Cortex-M Kinetis HAL functions
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2012, 2013 Free Software Foundation, Inc.                        
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
// Author(s):    Ilija kocho <ilijak@siva.com.mk>
// Date:         2012-03-08
// Description:
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm.h>
#include <pkgconf/hal_cortexm_kinetis.h>
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

// DDRAM Controller
#ifdef CYGPKG_HAL_CORTEXM_KINETIS_DDRMC

// DDRAM controller register indices.
const cyg_uint8 const kinetis_ddr_reg_ix[] = {
    0,
    2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    20, 21, 22, 23,
    25, 26, 27, 28, 29, 30,
    34,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
    52, 53, 54, 55, 56, 57
};

// Initialize DDRAM controller.
// inidat[] is an array of ordered pairs: (register-index, register-value).
void CYGOPT_HAL_KINETIS_MISC_FLASH_SECTION_ATTR
hal_cortexm_kinetis_ddrmc_init(const cyg_uint32 inidat[])
{
    cyghwr_hal_kinetis_ddrmc_t* ddrmc_p = CYGHWR_HAL_KINETIS_DDRMC_P;
    cyghwr_hal_kinetis_sim_t *sim_p = CYGHWR_HAL_KINETIS_SIM_P;
    volatile cyg_uint32* cr_p;
    cyg_uint32 cr_ix;
    cyg_uint32 cr_i;
    cyg_uint32 regval;
    
    CYGHWR_IO_CLOCK_ENABLE(CYGHWR_HAL_KINETIS_SIM_SCGC_DDR);

    regval = sim_p->mcr & ~CYGHWR_HAL_KINETIS_SIM_MCR_DDR_SETUP_M;
    sim_p->mcr   = regval | CYGHWR_HAL_KINETIS_SIM_MCR_DDR_SETUP;

    ddrmc_p->rcr |= CYGHWR_HAL_KINETIS_DDRMC_RCR_RST_M;
    ddrmc_p->pad_ctrl = CYGHWR_HAL_KINETIS_DDRMC_PAD_CTRL;
    cr_p = ddrmc_p->cr;
    for(cr_ix = 0; cr_ix < sizeof(kinetis_ddr_reg_ix); cr_ix++) {
        cr_i = kinetis_ddr_reg_ix[cr_ix];
        cr_p[cr_i] = *inidat++;
    };
    __asm__ volatile ("nop\n");
    ddrmc_p->cr[0] |= CYGHWR_HAL_KINETIS_DDRMC_CR00_START;
    while(!(ddrmc_p->cr[30] & CYGHWR_HAL_KINETIS_DDRMC_CR30_DRAM_INIT_CPL));
}

void
hal_cortexm_kinetis_ddrmc_diag(void)
{
    cyghwr_hal_kinetis_ddrmc_t* ddrmc_p = CYGHWR_HAL_KINETIS_DDRMC_P;
    cyghwr_hal_kinetis_sim_t *sim_p = CYGHWR_HAL_KINETIS_SIM_P;
    volatile cyg_uint32* cr_p;
    cyg_uint32 cr_i;

    diag_printf("SCGC3 = 0x%08x, MCR = 0x%08x\n", sim_p->scgc3, sim_p->mcr);
    diag_printf("RCR= 0x%08x, PAD_CTRL= 0x%08x\n", ddrmc_p->rcr, ddrmc_p->pad_ctrl);
    cr_p = ddrmc_p->cr;
    for(cr_i=0; cr_i < 64; cr_i++){
        diag_printf("  CR%02d = 0x%08x\n", cr_i, cr_p[cr_i]);
    }
}

#endif // CYGPKG_HAL_CORTEXM_KINETIS_DDRMC

//==========================================================================
// EOF kinetis_ddram.c
