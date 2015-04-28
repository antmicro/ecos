/*==========================================================================
//
//      cortexm_fpu.c
//
//      Cortex-M exception vectors
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
*/

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

#ifdef CYGHWR_HAL_CORTEXM_FPU_SWITCH_LAZY
//============================================================================
// FPU Usage Fault VSR handler
// Execution of Floating Point instruction when FPU is disabled
// generates usage fault exception. In LAZY context switching scheme
// it is used for detection of FPU usage by threads.
//
cyg_uint32 hal_deliver_usagefault_fpu_exception(void) {
    cyg_uint32 regval;

    HAL_READ_UINT32(CYGARC_REG_FPU_CPACR, regval);
    if(!((regval & CYGARC_REG_FPU_CPACR_ENABLE) ^ CYGARC_REG_FPU_CPACR_ENABLE)){
        CYG_FAIL("Usage fault exception other than FPU!!!");
    } else {
        HAL_READ_UINT32(CYGARC_REG_UFSR, regval);
        if(regval & CYGARC_REG_UFSR_NOCP){
            // Floating point instruction has occured
            // Enable FPU
            HAL_CORTEXM_FPU_ENABLE();
            CYGARC_VMSR(fpscr, 0);
            HAL_MEMORY_BARRIER();
            HAL_WRITE_UINT32(CYGARC_REG_UFSR, CYGARC_REG_UFSR_NOCP);
        } else {
            CYG_FAIL("Usage fault exception other than FPU/NOCP!!!");
        }
    }
    HAL_READ_UINT32(CYGARC_REG_UFSR, regval);
    return regval;
}
#endif // CYGHWR_HAL_CORTEXM_FPU_SWITCH_LAZY

//==========================================================================
// EOF cortexm_fpu.c
