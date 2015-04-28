/*==========================================================================
//
//      hal_arch.inc
//
//      Cortex-M exception vector macros
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
// Author(s):       ilijak
// Contributors(s):
// Date:            2012-07-08
// Description:     This file defines some GAS macros exception VSRs.
//
//####DESCRIPTIONEND####
//
//========================================================================
*/

#ifdef CYGHWR_HAL_CORTEXM_FPU

# ifdef CYGINT_HAL_FPV4_SP_D16

//============================================================================
// LAZY context switching scheme keeps FPU disabled for the threads that
// don't use floating point. We need to enable it before we save FPU context
// in order to avoid Usage Fault exception.

        .macro hal_fpu_enable
        ldr    r1,=CYGARC_REG_FPU_CPACR
        ldr    r2,[r1]
        stmdb  r0!,{r2}           // Save thread's CPACR state
        orr    r2,#CYGARC_REG_FPU_CPACR_ENABLE
        str    r2,[r1]
        .endm

//============================================================================
// Restore thread's FPU usage state.
// undo hal_fpu_enable

        .macro hal_fpu_undo_enable
        ldmia  r0!,{r2}           // Retrieve previous thread's CPACR state
        ldr    r1,=CYGARC_REG_FPU_CPACR
        str    r2,[r1]
        .endm

//============================================================================
// Store FPU context during exception if FPU was disabled then enamble it.

        .macro hal_fpu_exc_push
        hal_fpu_enable
        vmrs         r1,fpscr
        stmdb        r0!,{r1}
        vstmdb.f64   r0!,{d0-d15}
        .endm

//============================================================================
// Restore FPU context during exception and undo FPU enable.

        .macro hal_fpu_exc_pop
        vldmia.f64   r0!,{d0-d15}
        ldmia        r0!,{r1}
        vmsr         fpscr,r1
        hal_fpu_undo_enable
        .endm

//============================================================================
// Make fake fpu frame for hal_pendable_svc_vsr

        .macro hal_fpu_isr_fake_frame_push
        sub        r12,#4
        vmrs       r1,fpscr
        stmdb      r12!,{r1}
        vstmdb.f32 r12!,{s0-s15}
        .endm

# else // CYGINT_HAL_FPV4_SP_D16
#  error Unknown Floating Point Unit!
# endif // CYGINT_HAL_FPV4_SP_D16

#endif //CYGHWR_HAL_CORTEXM_FPU

// end of hal_arch.inc
