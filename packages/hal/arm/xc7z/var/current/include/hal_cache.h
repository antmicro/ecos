#ifndef CYGONCE_HAL_CACHE_H
#define CYGONCE_HAL_CACHE_H

//=============================================================================
//
//      hal_cache.h
//
//      HAL cache control API
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
// ####DESCRITPIONBEGIN####
// Authors: 
// Contributors: Ant Micro
// Date:         12.07.2012
// Purpose:      cache control
//
//
// ####DESCRIPTIONEND####


#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
 

// L2 cache
#define HAL_L2CACHE_SIZE                524288  // Size of L2 cache in bytes
#define HAL_L2CACHE_LINE_SIZE           32	// Size of a L2 cache line //64
#define HAL_L2CACHE_WAYS                8       // Associativity of the cache

// Data cache
#define HAL_DCACHE_SIZE                 32768   // Size of data cache in bytes
#define HAL_DCACHE_LINE_SIZE            64      // Size of a data cache line
#define HAL_DCACHE_WAYS                 4       // Associativity of the cache

// Instruction cache
#define HAL_ICACHE_SIZE                 32768    // Size of cache in bytes
#define HAL_ICACHE_LINE_SIZE            64      // Size of a cache line
#define HAL_ICACHE_WAYS                 4       // Associativity of the cache

#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))


#define HAL_ARM_DSB()                                          \
CYG_MACRO_START                                                \
asm volatile (                                                 \
	     "mov r0,#0;"                                           \
	     "mcr p15, 0, r0, c7, c10, 4;" /* DSB */                \
	     :                                                      \
	     :                                                      \
	     : "r0"                                                 \
	     );                                                     \
CYG_MACRO_END

//-----------------------------------------------------------------------------
// Global control of data cache

// Enable the data cache
#define HAL_DCACHE_ENABLE()	                                        \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc p15, 0, r1, c1, c0, 0;"                                    \
                "orr r1, r1, #0x00000005;" /* enable DCache (also ensures */        \
                               /* the MMU,  and */              	\
        "mcr p15, 0, r1, c1, c0, 0"                                     \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END


// Disable the data cache
#define HAL_DCACHE_DISABLE()	                                        \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc p15, 0, r1, c1, c0, 0;"                                    \
        "bic r1, r1, #0x00000004;" /* disable DCache by clearing C bit */   \
                             /* but not MMU and alignment faults*/      \
        "mcr p15, 0, r1, c1, c0, 0"                                     \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
    );                                                                  \
CYG_MACRO_END

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)                                  \
CYG_MACRO_START                                                         \
    register int reg;                                                   \
    asm volatile (                                                      \
        "nop; "                                                         \
        "nop; "                                                         \
        "nop; "                                                         \
        "nop; "                                                         \
        "nop; "                                                         \
        "mrc p15, 0, %0, c1, c0, 0;"                                    \
                  : "=r"(reg)                                           \
                  :                                                     \
        );                                                              \
    (_state_) = (0 != (4 & reg)); /* Bit 2 is DCache enable */          \
CYG_MACRO_END

// Invalidate the entire cache
#define HAL_DCACHE_INVALIDATE_ALL_DEFINED
#define HAL_DCACHE_INVALIDATE_ALL()                                                     \
    CYG_MACRO_START                                                                     \
    register volatile CYG_ADDRESS addr;                                                   \
    register volatile CYG_WORD irqs;                                                    \
    HAL_DISABLE_INTERRUPTS(irqs);                                                       \
    for (addr = (CYG_ADDRESS)0;                                                          \
         addr < (HAL_L2CACHE_SIZE);                                         \
         addr += HAL_DCACHE_LINE_SIZE )                                                 \
    {                                                                                   \
		 asm volatile (                                                                   \
		     "mcr p15, 0, %0, c7, c6, 1;"  /* Invalidate Data or Unified cache line by MVA to PoU */  \
		     "nop;":                                                                      \
		     : "r"(addr)                                                                  \
		     :                                                                            \
		     );                                                                           \
    }                                                                                   \
    HAL_RESTORE_INTERRUPTS(irqs);                                                       \
    CYG_MACRO_END

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC_DEFINED
#define HAL_DCACHE_SYNC()                                                               \
    CYG_MACRO_START                                                                     \
    register volatile CYG_BYTE *addr;                                                   \
    register volatile CYG_WORD irqs;                                                    \
    HAL_DISABLE_INTERRUPTS(irqs);                                                       \
    for (addr = (CYG_BYTE *)0;                                                          \
         addr < (CYG_BYTE *)(HAL_L2CACHE_SIZE);                                         \
         addr += HAL_L2CACHE_LINE_SIZE )                                                 \
    {                                                                                   \
		 asm volatile (                                                                   \
		     "mcr p15, 0, %0, c7, c10, 1;"  /* Clean Data or Unified cache line by MVA to PoC */  \
		     :                                                                            \
		     : "r"(addr)                                                                  \
		     :                                                                            \
		     );                                                                           \
    }                                                                                   \
	 HAL_ARM_DSB();                                                                    \
    HAL_RESTORE_INTERRUPTS(irqs);                                                       \
    CYG_MACRO_END


// Load the contents of the given address range into the data cache
// and then lock the cache so that it stays there.
#define HAL_DCACHE_LOCK_DEFINED
#define HAL_DCACHE_LOCK(_base_, _asize_) HAL_CACHE_LOCK(_base_,_asize_,0)
#define HAL_DCACHE_UNLOCK_DEFINED
#define HAL_DCACHE_UNLOCK(_base_, _asize_) HAL_CACHE_UNLOCK(_base_,_asize_,0)

#define HAL_CACHE_LOCK(_base_, _asize_, _way_)                                          \
    CYG_MACRO_START                                                                     \
    CYG_ASSERT(((CYG_ADDRESS)(_base_) & (HAL_L2CACHE_LINE_SIZE-1)) == 0,"cache alignment"); \
    register volatile CYG_WORD irqs;                                                    \
    register CYG_WORD old_lockdown;                                                     \
    register CYG_WORD way = 0xff & (~(1<<_way_));                                     \
    register CYG_ADDRESS _endaddr_ = (CYG_ADDRESS)(_base_) + (_asize_);                   \
    register CYG_ADDRESS _addr_ = (CYG_ADDRESS)(_base_);                                \
    HAL_DISABLE_INTERRUPTS(irqs);                                                       \
    HAL_DCACHE_FLUSH(_base_, _asize_);                                                  \
    HAL_ICACHE_INVALIDATE(_base_, _asize_);                                             \
    asm volatile (  " mrc p15, 0, %0, c9, c0, 0;"                                       \
                    " mcr p15, 0, %1, c9, c0, 0;"                                       \
                     : "=r"(old_lockdown) : "r"(way) : );                               \    
    for( ; _addr_ < _endaddr_; _addr_ += HAL_DCACHE_LINE_SIZE )                         \
      asm volatile (" ldr r0, [%0]"                                                     \
                    :                                                                   \
                    : "r"(_addr_) : "r0");                                              \
    way = old_lockdown | (1<<_way_);                                                    \
    asm volatile (  " mcr p15, 0, %0, c9, c0, 0;"                                       \
                     : : "r"(way) : );      						\
    HAL_RESTORE_INTERRUPTS(irqs);                                                       \
    CYG_MACRO_END

// Undo a previous lock operation
#define HAL_CACHE_UNLOCK(_base_, _asize_, _way_)                                        \
    CYG_MACRO_START                                                                     \
    asm volatile (  " mrc p15, 0, r0, c9, c0, 0;"                                       \
                    " bic r0, r0, %0;"                                                  \
                    " mcr p15, 0, r0, c9, c0, 0;"                                       \
                     : : "I"(1<<_way_) : );                                             \
    CYG_MACRO_END

// Unlock entire cache
#define HAL_DCACHE_UNLOCK_ALL_DEFINED
#define HAL_DCACHE_UNLOCK_ALL() HAL_DCACHE_UNLOCK(0,HAL_DCACHE_SIZE)

#define HAL_ICACHE_LOCK_DEFINED
#define HAL_ICACHE_LOCK(_base_, _asize_) HAL_CACHE_LOCK(_base_,_asize_,1)
#define HAL_ICACHE_UNLOCK_DEFINED
#define HAL_ICACHE_UNLOCK(_base_, _asize_) HAL_CACHE_UNLOCK(_base_,_asize_,1)

// Unlock entire cache
#define HAL_ICACHE_UNLOCK_ALL_DEFINED
#define HAL_ICACHE_UNLOCK_ALL() HAL_CACHE_UNLOCK(0,HAL_DCACHE_SIZE,1)


//-----------------------------------------------------------------------------
// Data cache line control

// Allocate cache lines for the given address range without reading its
// contents from memory.
//#define HAL_DCACHE_ALLOCATE( _base_ , _asize_ )

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_DCACHE_FLUSH_DEFINED
#define HAL_DCACHE_FLUSH( _base_ , _asize_ )                                            \
    CYG_MACRO_START                                                                     \
    CYG_ASSERT(((CYG_ADDRESS)(_base_) & (HAL_L2CACHE_LINE_SIZE-1)) == 0,"cache alignment"); \
    register CYG_ADDRESS _endaddr_ = (CYG_ADDRESS)(_base_) + (_asize_);                   \
    register CYG_ADDRESS _addr_ = (CYG_ADDRESS)(_base_);                                \
    register volatile CYG_WORD irqs;                                                    \
    HAL_DISABLE_INTERRUPTS(irqs);                                                       \
    for( ; _addr_ < _endaddr_; _addr_ += HAL_L2CACHE_LINE_SIZE )                        \
		 asm volatile (                                                                   \
		     "mcr p15, 0, %0, c7, c14, 1;"  /* Clean and Invalidate Data or Unified cache line by MVA to PoC */  \
		     :                                                                            \
		     : "r"(_addr_)                                                                \
		     :                                                                            \
		     );                                                                           \
	 HAL_ARM_DSB();                                                                      \
    HAL_RESTORE_INTERRUPTS(irqs);                                                       \
    CYG_MACRO_END

// Write dirty cache lines to memory for the given address range.
#define HAL_DCACHE_STORE_DEFINED
#define HAL_DCACHE_STORE( _base_ , _asize_ )                                            \
    CYG_MACRO_START                                                                     \
    CYG_ASSERT(((CYG_ADDRESS)(_base_) & (HAL_L2CACHE_LINE_SIZE-1)) == 0,"cache alignment"); \
    register CYG_ADDRESS _endaddr_ = (CYG_ADDRESS)(_base_) + (_asize_);                   \
    register CYG_ADDRESS _addr_ = (CYG_ADDRESS)(_base_);                                \
    register volatile CYG_WORD irqs;                                                    \
    HAL_DISABLE_INTERRUPTS(irqs);                                                       \
    for( ; _addr_ < _endaddr_; _addr_ += HAL_L2CACHE_LINE_SIZE )                        \
		 asm volatile (                                                                   \
		     "mcr p15, 0, %0, c7, c10, 1;"  /* Clean Data or Unified cache line by MVA to PoC */  \
		     :                                                                            \
		     : "r"(_addr_)                                                                \
		     :                                                                            \
		     );                                                                           \
	 HAL_ARM_DSB();                                                                      \
    HAL_RESTORE_INTERRUPTS(irqs);                                                       \
    CYG_MACRO_END

// Invalidate cache lines in the given range without writing to memory.
#define HAL_DCACHE_INVALIDATE_DEFINED
#define HAL_DCACHE_INVALIDATE( _base_ , _asize_ )                                       \
    CYG_MACRO_START                                                                     \
    CYG_ASSERT(((CYG_ADDRESS)(_base_) & (HAL_L2CACHE_LINE_SIZE-1)) == 0,"cache alignment"); \
    register CYG_ADDRESS _endaddr_ = (CYG_ADDRESS)(_base_) + (_asize_);                   \
    register CYG_ADDRESS _addr_ = (CYG_ADDRESS)(_base_);                                \
    register volatile CYG_WORD irqs;                                                    \
    HAL_DISABLE_INTERRUPTS(irqs);                                                       \
    for( ; _addr_ < _endaddr_; _addr_ += HAL_L2CACHE_LINE_SIZE )                        \
		 asm volatile (                                                                   \
		     "mcr p15, 0, %0, c7, c6, 1;"  /* Invalidate data cache line by MVA to PoC */  \
		     :                                                                            \
		     : "r"(_addr_)                                                                \
		     :                                                                            \
		     );                                                                           \
	 HAL_ARM_DSB();                                                                      \
    HAL_RESTORE_INTERRUPTS(irqs);                                                       \
    CYG_MACRO_END

// Invalidate cache lines in the given range without writing to memory.
#define HAL_ICACHE_INVALIDATE_DEFINED
#define HAL_ICACHE_INVALIDATE( _base_ , _asize_ )                                       \
    CYG_MACRO_START                                                                     \
    CYG_ASSERT(((CYG_ADDRESS)(_base_) & (HAL_L2CACHE_LINE_SIZE-1)) == 0,"cache alignment"); \
    register CYG_ADDRESS _endaddr_ = (CYG_ADDRESS)(_base_) + (_asize_);                   \
    register CYG_ADDRESS _addr_ = (CYG_ADDRESS)(_base_);                                \
    register volatile CYG_WORD irqs;                                                    \
    HAL_DISABLE_INTERRUPTS(irqs);                                                       \
    for( ; _addr_ < _endaddr_; _addr_ += HAL_L2CACHE_LINE_SIZE )                        \
		 asm volatile (                                                                   \
		     "mcr p15, 0, %0, c7, c5, 1;"  /* Invalidate instruction cahces by MVA to PoU */  \
		     "mcr p15, 0, %0, c7, c5, 7;"  /* Invalidate MVA from branch prediction array */  \
           "mcr p15, 0, r1, c7, c5, 4;"  /* flush prefetch buffer */                    \
		     :                                                                            \
		     : "r"(_addr_)                                                                \
		     :                                                                            \
		     );                                                                           \
	 HAL_ARM_DSB();                                                                      \
    HAL_RESTORE_INTERRUPTS(irqs);                                                       \
    CYG_MACRO_END

//-----------------------------------------------------------------------------
// Global control of Instruction cache

// Enable the instruction cache
#define HAL_ICACHE_ENABLE()                                             \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc p15, 0, r1, c1, c0, 0;"                                    \
        "orr r1, r1, #0x1000;"                                          \
        "orr r1, r1, #0x0001;"  /* enable ICache (also ensures   */     \
                                /* that MMU  */     \
                                /* are enabled)                  */     \
        "mcr p15, 0, r1, c1, c0, 0"                                     \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)                                  \
CYG_MACRO_START                                                         \
    register cyg_uint32 reg;                                            \
    asm volatile (                                                      \
        "mrc p15, 0, %0, c1, c0, 0"                                     \
        : "=r"(reg)                                                     \
        :                                                               \
        );                                                              \
                                                                        \
    (_state_) = (0 != (0x1000 & reg)); /* Bit 12 is ICache enable */    \
CYG_MACRO_END

// Disable the instruction cache
#define HAL_ICACHE_DISABLE()	                                        \
CYG_MACRO_START                                                         \
    asm volatile (                                                      \
        "mrc p15, 0, r1, c1, c0, 0;"                                    \
        "bic r1, r1, #0x1000;" /* disable ICache (but not MMU, etc) */  \
        "mcr p15, 0, r1, c1, c0, 0;"                                    \
        "mov r1, #0;"                                                   \
        "nop;" /* next few instructions may be via cache    */          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
	     :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
CYG_MACRO_END

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()                                     \
CYG_MACRO_START                                                         \
    /* this macro can discard dirty cache lines (N/A for ICache) */     \
    register volatile CYG_WORD irqs;                                    \
    HAL_DISABLE_INTERRUPTS(irqs);                                       \
    asm volatile (                                                      \
        "mov r1, #0;"                                                   \
        "mcr p15, 0, r1, c7, c5, 0;"  /* flush ICache */                \
        "mcr p15, 0, r1, c8, c5, 0;"  /* flush ITLB only */             \
        "mcr p15, 0, r1, c7, c5, 4;"  /* flush prefetch buffer */       \
        "nop;" /* next few instructions may be via cache    */          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
        "nop;"                                                          \
	     "nop;"                                                          \
        :                                                               \
        :                                                               \
        : "r1" /* Clobber list */                                       \
        );                                                              \
	 HAL_ARM_DSB();                                                      \
    HAL_RESTORE_INTERRUPTS(irqs);                                       \
CYG_MACRO_END

// Synchronize the contents of the cache with memory.
// (which includes flushing out pending writes)
#define HAL_ICACHE_SYNC()                                       \
CYG_MACRO_START                                                 \
    HAL_DCACHE_SYNC(); /* ensure data gets to RAM */            \
    HAL_ICACHE_INVALIDATE_ALL(); /* forget all we know */       \
CYG_MACRO_END


/*********************** Exported macros *******************/

#define CYGARC_HAL_MMU_OFF(__paddr__)  \
        "mrc p15, 0, r0, c1, c0, 0;" /* read c1 */                      \
        "bic r0, r0, #0x5;" /* disable DCache and MMU */                \
        "bic r0, r0, #0x1000;" /* disable ICache */                     \
        "mcr p15, 0, r0, c1, c0, 0;" /*  */                             \
        "nop;" /* flush i+d-TLBs */                                     \
        "nop;" /* flush i+d-TLBs */                                     \
        "nop;" /* flush i+d-TLBs */

#define HAL_MMU_OFF() \
CYG_MACRO_START          \
    asm volatile (                                                      \
        CYGARC_HAL_MMU_OFF()   \
    );      \
CYG_MACRO_END



#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
