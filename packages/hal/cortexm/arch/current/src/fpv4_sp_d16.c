//==========================================================================
//
//      fpv4_sp_d16.c
//
//      FPv4-SP-D16 support
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
// Author(s):      ilijak
// Contributor(s):
// Date:           2012-05-30
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm.h>

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL header
#include <cyg/hal/cortexm_regs.h>       // Special Cortex-M asm instructions

#include <cyg/hal/cortexm_fpu.h>        // Optional Floating Point Unit


#define CYGARC_REG_NVIC_ACTLR                   (CYGARC_REG_NVIC_BASE + 0x008)
#define CYGARC_REG_NVIC_ACTLR_DISFCA            BIT_(8)
//==========================================================================
// FPU is disbled upon reset. Dependent on FPU context switching model it
// may be enabled.

void hal_init_fpu(void)
{
    cyg_uint32 regval;

    // Initialize FPU according to context switch model.
    // Disable FPU so we could access FPCCR
    HAL_CORTEXM_FPU_DISABLE();

#if defined CYGARC_CORTEXM_FPU_EXC_AUTOSAVE
    // Enable automatic exception FPU context saving.
    HAL_READ_UINT32(CYGARC_REG_FPU_FPCCR, regval);
    regval |= CYGARC_REG_FPU_FPCCR_LSPEN | CYGARC_REG_FPU_FPCCR_ASPEN;
    HAL_WRITE_UINT32(CYGARC_REG_FPU_FPCCR, regval);
    HAL_MEMORY_BARRIER();
#else
    // Disable automatic exception FPU context saving.
    CYGARC_MRS(regval, control);
    regval &= ~CYGARC_REG_CONTROL_FPCA_M;
    CYGARC_MSR(control, regval);
    HAL_READ_UINT32(CYGARC_REG_FPU_FPCCR, regval);
    regval &= ~(CYGARC_REG_FPU_FPCCR_LSPEN | CYGARC_REG_FPU_FPCCR_ASPEN);
    HAL_WRITE_UINT32(CYGARC_REG_FPU_FPCCR, regval);
    HAL_MEMORY_BARRIER();
#endif

#if defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_NONE || \
    defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL
    // Enable FPU
    HAL_CORTEXM_FPU_ENABLE();
    CYGARC_VMSR(fpscr, 0);
#endif
}

//==========================================================================
// EOF fpv4_sp_d16.c
