/*==========================================================================
//
//      mcount.S
//
//      PowerPC mcount implementation
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
// Date:            2014-03-20
// Description:     This file provides mcount functions used for
//                  call-graph profiling.
//
//####DESCRIPTIONEND####
//
//========================================================================
*/

#include <cyg/hal/arch.inc>

/*
// GCC inserts mcount code at the start of every function when compiling
// with "-pg". For GCC targeting PowerPC, the following code is inserted:
//
//   mflr r0
//   stw r0, 4(r1)
//   bl _mcount
//
// We provide an implementation of _mcount() to call the eCos
// __profile_mcount() function.
*/

        .section ".text.mcount"
        .globl _mcount
_mcount:
        // create stack frame
        stwu r1, -48(r1)

        // caller assumes r3-r10 will be preserved and we use r14
        stw r3, 8(r1)
        stw r4, 12(r1)
        stw r5, 16(r1)
        stw r6, 20(r1)
        stw r7, 24(r1)
        stw r8, 28(r1)
        stw r9, 32(r1)
        stw r10, 36(r1)
        stw r14, 40(r1)

        // set up parameters for __profile_mcount() and preserve
        // lr across our __profile_mcount() call
        lwz r3, 52(r1)
        subi r3, r3, 2
        mflr r4
        stw r4, 44(r1)

        // disable interrupts
        lwi r0, ~CYGARC_REG_MSR_INTBITS
        mfmsr r14
        and r0, r0, r14
        mtmsr r0

        // call eCos __profile_mcount()
        // r14 is preserved across the call per EABI standard
        bl __profile_mcount

        // restore interrupts
        mtmsr r14

        // restore registers
        lwz r3, 8(r1)
        lwz r4, 12(r1)
        lwz r5, 16(r1)
        lwz r6, 20(r1)
        lwz r7, 24(r1)
        lwz r8, 28(r1)
        lwz r9, 32(r1)
        lwz r10, 36(r1)
        lwz r14, 40(r1)

        // unwind stack frame and return
        lwz r0, 44(r1)
        mtctr r0
        lwz r0, 52(r1)
        mtlr r0
        addi r1, r1, 48
        bctr

//==========================================================================
// end of mcount.S
