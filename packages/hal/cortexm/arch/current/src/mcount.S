/*==========================================================================
//
//      mcount.S
//
//      Cortex-M mcount implementation
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2014 Free Software Foundation, Inc.                        
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
// Author(s):       jld
// Contributor(s):  
// Date:            2014-02-28
// Description:     This file provides mcount functions used for
//                  call-graph profiling.
//
//####DESCRIPTIONEND####
//
//========================================================================
*/

#include <pkgconf/hal_cortexm.h>

/*
// GCC inserts mcount code at the start of every function when compiling
// with "-pg". For GCC prior to version 4.4 targeting Cortex-M,
// the following code is inserted:
// 
//   mov r12, lr
//   bl mcount
//   .word <data pointer>
//
// For GCC version 4.4 and later targeting Cortex-M, the following code is
// inserted:
//
//   push { lr }
//   bl __gnu_mcount_nc
//
// We provide implementations of both mcount() and __gnu_mcount_nc() to
// call the eCos __profile_mcount() function.
*/

        .syntax unified
        .globl mcount
        .section .text.mcount
        .thumb_func
mcount:
        // resume execution beyond the data pointer on return to caller
        add lr, lr, #4

        // caller assumes r0-r3 will be preserved (non-AAPCS), we use
        // r6 and must preserve lr across our __profile_mcount() call
        push { r0, r1, r2, r3, r6, lr }

        // set up parameters for __profile_mcount()
        sub r0, r12, #2
        bic r0, r0, #1
        bic r1, lr, #1

        // disable interrupts
        mov r2, #CYGNUM_HAL_CORTEXM_PRIORITY_MAX
        mrs r6, basepri
        msr basepri, r2

        // call eCos __profile_mcount()
        // r6 is preserved across the call per AAPCS
        bl __profile_mcount

        // restore interrupts
        msr basepri, r6

        // restore registers and return
        pop { r0, r1, r2, r3, r6, pc }


        .globl __gnu_mcount_nc
        .section .text.__gnu_mcount_nc
        .thumb_func
__gnu_mcount_nc:
        // caller assumes r0-r3 will be preserved (non-AAPCS), we use
        // r6 and must preserve lr across our __profile_mcount() call
        push { r0, r1, r2, r3, r6, lr }

        // set up parameters for __profile_mcount()
        ldr r0, [ sp, #24 ]
        sub r0, r0, #2
        bic r0, r0, #1
        bic r1, lr, #1

        // disable interrupts
        mov r2, #CYGNUM_HAL_CORTEXM_PRIORITY_MAX
        mrs r6, basepri
        msr basepri, r2

        // call eCos __profile_mcount()
        // r6 is preserved across the call per AAPCS
        bl __profile_mcount

        // restore interrupts
        msr basepri, r6

        // restore registers and return
        pop { r0, r1, r2, r3, r6, r12, lr }
        bx r12

//==========================================================================
// end of mcount.S
