#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2006 Free Software Foundation, Inc.
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
// Author(s):   ITR-GmbH
// Contributors:Antmicro <www.antmicro.com>
// Date:        2012-06-27
// Purpose:     Zynq-7000 platform specific support routines
// Description:
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/hal/var_io.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_mmu.h>


// Flush and disable the caches
        .macro _disable_cache
__disable_cache:
// Flush the entire dcache (and then both TLBs, just in case)
        mov     r0, #0
        mcr     p15,0,r0,c7,c6,0
        mcr     p15,0,r0,c8,c7,0

// Disable and clear caches
        mrc     p15,0,r0,c1,c0,0
        bic     r0,r0,#0x1000           // disable ICache
        bic     r0,r0,#0x000F           // disable DCache,
                                        // MMU and alignment faults
        mcr     p15,0,r0,c1,c0,0
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        mov     r0, #0
        mcr     p15,0,r0,c7,c6,0        // clear data cache
        mcr     p15,0,r0,c7,c5,0        // clear instruction cache


        .endm

#if defined(CYG_HAL_STARTUP_ROM)
        .macro  _setup
        _flash_init
        _main_clock_init
        _remap_flash
        .endm
#else
        .macro _setup
        nop
        /*
        * Invalidate L1 I/D
        */
        mov r0, #0                  // set up for MCR
        mcr p15, 0, r0, c8, c7, 0   // invalidate TLBs
        mcr p15, 0, r0, c7, c5, 0   // invalidate icache

	/*
	* Set VBAR to zero
	*/
	mov r0, #0
	mcr p15, 0, r0, c12, c0, 0

        /*
        * disable MMU stuff and caches
        */
        mrc p15, 0, r0, c1, c0, 0
        bic r0, r0, #0x00001000     // clear bit 12 (--I-) Instruction cache
        bic r0, r0, #0x00002000     // clear bit 13 (--V-)
        bic r0, r0, #0x00000007     // clear bits 2:0 (-CAM)
        orr r0, r0, #0x00000002     // set bit 1 (--A-) Align
        orr r0, r0, #0x00000800     // set bit 11 (Z---) Branch prediction
        mcr p15, 0, r0, c1, c0, 0
        nop
        nop
        nop
        nop
        nop
        
        // Disable ints and turn Supervisor mode
        mov     r0, #(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr, r0
        // Prepare SPSR_SVC for work
        mov     r0, #(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     spsr_cxsf, r0
        mrs     r0, spsr
       
        // unlock slcr, remap sram, lock slcr
        ldr r0, =0xdf0d
        ldr r1, =0xf8000008
        str r0, [r1]
        ldr r0, =0xf
        ldr r1, =0xf8000910
        str r0, [r1] 
	ldr r0, =0x0
	ldr r1, =0xf8f00040
	str r0, [r1]
        ldr r0, =0x767b
        ldr r1, =0xf8000008
        str r0, [r1]
        
        nop
        nop
        nop
        nop
        nop
        mov r0, #0
        mov r1, #0

	// Create MMU tables
	bl hal_mmu_init

	// Enable MMU
	mrc p15, 0, r1, c1, c0, 0
	orr r1, r1, #1
	orr r1, r1, #(1 << 28)
	mcr p15, 0, r1, c1, c0, 0

        .endm
#endif 


#define PLATFORM_SETUP1 _setup

#ifdef CYGPKG_HAL_SMP_SUPPORT
        // This code adds a core to SMP. It assumes the primary core already
        // has enabled snoop control and has entered SMP.
        .macro _setup2
        nop

        // Disable interupts and set to SVC mode
        mov     r0, #(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     cpsr, r0

        // Set SPSR
        mov     r0, #(CPSR_IRQ_DISABLE|CPSR_FIQ_DISABLE|CPSR_SUPERVISOR_MODE)
        msr     spsr_cxsf, r0
        mrs     r0, spsr

        cpsie A                         @ Enable non-precise data aborts.

        // Join SMP, Enable SCU broadcast.
        mrc     p15, 0, r0, c1, c0, 1   @ Read ACTLR
        orr     r0, r0, #0x042          @ Set bit 6, 2 and 1
        mcr     p15, 0, r0, c1, c0, 1   @ Write ACTLR

        // Disable strict align check
    	mrc     p15, 0, r0, c1, c0, 0
    	bic     r0, r0, #(0x1<<1)       @ Clear A bit of SCTLR
    	mcr     p15, 0, r0, c1, c0, 0

    	// Disable branch prediction
        mrc     p15, 0, r0, c1, c0, 0                 @ Read SCTLR
    	orr     r0, r0, #(1 << 11)                    @ Set the Z bit (bit 11)
    	mcr     p15, 0,r0, c1, c0, 0                  @ Write SCTLR

    	// Invalidate L1 I-cache
    	mov	r1,	#0x0
    	mcr p15, 0, r1, c7, c5, 0       @ Invalidate I-Cache
        mcr p15, 0, r1, c7, c5, 6       @ Invalidate Branch Predictor
        mov  r1, #0x1800
        mcr p15, 0, r1, c1, c0, 0       @ Enable I-Cache and Branch Predictor
        isb

    	// Invalidate L1 D-cache
        mov     r0, #0
        mcr     p15, 2, r0, c0, c0, 0
        mrc     p15, 1, r0, c0, c0, 0

        ldr     r1, =0x7fff
        and     r2, r1, r0, lsr #13

        ldr     r1, =0x3ff

        and     r3, r1, r0, lsr #3  @ NumWays - 1
        add     r2, r2, #1          @ NumSets

        and     r0, r0, #0x7
        add     r0, r0, #4          @ SetShift

        clz     r1, r3              @ WayShift
        add     r4, r3, #1          @ NumWays
10:     sub     r2, r2, #1          @ NumSets--
        mov     r3, r4              @ Temp = NumWays
20:     subs    r3, r3, #1          @ Temp--
        mov     r5, r3, lsl r1
        mov     r6, r2, lsl r0
        orr     r5, r5, r6          @ Reg = (Temp<<WayShift)|(NumSets<<SetShift)
        mcr     p15, 0, r5, c7, c6, 2
        bgt     20b
        cmp     r2, #0
        bgt     10b
        dsb

        // Set MMU table address
        ldr     r0, =__mmu_tables_start
        mcr     p15, 0, r0, c2, c0, 0 @ TTBR0

        // Set Client mode for all Domains
        ldr     r0, =0x55555555
        mcr     p15, 0, r0, c3, c0, 0 @ DACR

        mov     r0, #1
        mcr     p15, 0, r0, c8, c7, 0                 @ TLBIALL - Invalidate entire unified TLB
        dsb

        // Enable MMU
        mrc     p15, 0, r0, c1, c0, 0
        orr     r0, r0, #1
        mcr     p15, 0, r0, c1, c0, 0
        isb
        dsb

        // Enable I and D Caches
        mrc     p15, 0, r0, c1, c0, 0 @ SCTLR
        orr     r0, r0, #0x1000
        orr     r0, r0, #0x0004
        mcr     p15, 0, r0, c1, c0, 0 @ SCTLR
        dsb
        isb

        .endm

#define PLATFORM_SETUP2 _setup2
#endif

//-----------------------------------------------------------------------------
// end of hal_platform_setup.h
#endif // CYGONCE_HAL_PLATFORM_SETUP_H
