#ifndef CYGONCE_HAL_CACHE_H
#define CYGONCE_HAL_CACHE_H

//=============================================================================
//
//      hal_cache.h
//
//      HAL cache control API
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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
// Author(s):   Scott Furman
// Contributors:Piotr Skrzypek
// Date:        2003-02-08
// Purpose:     Cache control API
// Description: The macros defined here provide the HAL APIs for handling
//              cache control operations.
// Usage:
//              #include <cyg/hal/hal_cache.h>
//              ...
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#ifndef __ASSEMBLER__

#include <cyg/hal/plf_cache.h>
#include <cyg/hal/hal_arch.h>

//-----------------------------------------------------------------------------
// Data cache
//
// If HAL_DCACHE_SIZE is undefined, assume that device does not implement
// data cache. Provide set of empty macros.
#ifndef HAL_DCACHE_SIZE

//Enable the data cache
#define HAL_DCACHE_ENABLE()

//Disable the data cache
#define HAL_DCACHE_DISABLE()

//Invalidate the entire cache
#define HAL_DCACHE_INVALIDATE_ALL()

//Synchronize the contents of the cache with memory
#define HAL_DCACHE_SYNC()

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)          \
    CYG_MACRO_START                             \
    (_state_) = 0;                              \
    CYG_MACRO_END

// If HAL_DCACHE_SIZE is defined, then implement proper macros.
#else //#ifndef HAL_DCACHE_SIZE

// Enable the data cache
#define HAL_DCACHE_ENABLE() MTSPR(SPR_SR, MFSPR(SPR_SR) | SPR_SR_DCE)

// Disable the data cache
#define HAL_DCACHE_DISABLE() MTSPR(SPR_SR, MFSPR(SPR_SR) & ~SPR_SR_DCE)

// Support macro. Enable or disable the data cache, depending on argument, 
// which is required to be 0 or 1.
#define HAL_SET_DCACHE_ENABLED(enable)                          \
    MTSPR(SPR_SR, MFSPR(SPR_SR) | (SPR_SR_DCE & -(enable)))

// Invalidate the entire data cache
#define HAL_DCACHE_INVALIDATE_ALL()                             \
    CYG_MACRO_START                                             \
    int cache_enabled, addr;                                    \
                                                                \
    /* Save current cache mode (disabled/enabled) */            \
    HAL_DCACHE_IS_ENABLED(cache_enabled);                       \
                                                                \
    /* Disable cache, so that invalidation ignores cache tags */\
    HAL_DCACHE_DISABLE();                                       \
    addr = HAL_DCACHE_SIZE;                                     \
    do {                                                        \
        MTSPR(SPR_DCBIR, addr);                                 \
        addr -= HAL_DCACHE_LINE_SIZE;                           \
    } while (addr > 0);                                         \
                                                                \
    /* Re-enable cache if it was enabled on entry */            \
    HAL_SET_DCACHE_ENABLED(cache_enabled);                      \
    CYG_MACRO_END

// Synchronize the contents of the cache with memory.
#define HAL_DCACHE_SYNC() HAL_DCACHE_FLUSH(0, HAL_DCACHE_SIZE)

// Query the state of the data cache
#define HAL_DCACHE_IS_ENABLED(_state_)                          \
    CYG_MACRO_START                                             \
    (_state_) = (1 - !(MFSPR(SPR_SR) & SPR_SR_DCE));            \
    CYG_MACRO_END

// Write dirty cache lines to memory and invalidate the cache entries
// for the given address range.
#define HAL_DCACHE_FLUSH( _base_ , _size_ )                          \
    CYG_MACRO_START                                                  \
    int addr;                                                        \
    int end = _base_ + _size_ - 1;                                   \
    for (addr = end; addr >= _base_; addr -= HAL_DCACHE_LINE_SIZE) { \
        MTSPR(SPR_DCBFR, addr);                                      \
    }                                                                \
    CYG_MACRO_END

// Invalidate cache lines in the given range without writing to memory
#define HAL_DCACHE_INVALIDATE( _base_ , _size_ )                     \
    CYG_MACRO_START                                                  \
    int addr;                                                        \
    int end = _base_ + _size_ - 1;                                   \
    for (addr = end; addr >= _base_; addr -= HAL_DCACHE_LINE_SIZE) { \
        MTSPR(SPR_DCBIR, addr);                                      \
    }                                                                \
    CYG_MACRO_END

// Write dirty cache lines to memory for the given address range
#if defined(HAL_DCACHE_MODE_WRITETHROUGH)

#define HAL_DCACHE_STORE( _base_ , _size_ )

#elif defined(HAL_DCACHE_MODE_WRITEBACK)

#define HAL_DCACHE_STORE( _base_ , _size_ )                          \
    CYG_MACRO_START                                                  \
    int addr;                                                        \
    int end = _base_ + _size_ - 1;                                   \
    for (addr = end; addr >= _base_; addr -= HAL_DCACHE_LINE_SIZE) { \
        MTSPR(SPR_DCBWR, addr);                                      \
    }                                                                \
    CYG_MACRO_END

#else

#error Unsupported cache mode

#endif

#endif //#ifndef HAL_DCACHE_SIZE

//-----------------------------------------------------------------------------
// Instruction cache
//
// If HAL_ICACHE_SIZE is undefined, assume that device does not implement
// instruction cache. Provide set of empty macros.
#ifndef HAL_ICACHE_SIZE

// Enable the instruction cache
#define HAL_ICACHE_ENABLE()

// Disable the instruction cache
#define HAL_ICACHE_DISABLE()

// Invalidate the entire cache
#define HAL_ICACHE_INVALIDATE_ALL()

// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC()

// Query the state of the instruction cache (does not affect the caching)
#define HAL_ICACHE_IS_ENABLED(_state_)          \
    CYG_MACRO_START                             \
    (_state_) = 0;                              \
    CYG_MACRO_END

#else //#ifndef HAL_ICACHE_SIZE

// Enable the instruction cache
#define HAL_ICACHE_ENABLE() MTSPR(SPR_SR, MFSPR(SPR_SR) | SPR_SR_ICE)

// Disable the instruction cache
#define HAL_ICACHE_DISABLE() MTSPR(SPR_SR, MFSPR(SPR_SR) & ~SPR_SR_ICE)

// Support macro. Enable or disable the data cache, depending on argument, 
// which must be 0 or 1.
#define HAL_SET_ICACHE_ENABLED(enable)                          \
    MTSPR(SPR_SR, MFSPR(SPR_SR) | (SPR_SR_ICE & -(enable)))

// Invalidate the entire instruction cache
#define HAL_ICACHE_INVALIDATE_ALL()                             \
    CYG_MACRO_START                                             \
    int cache_enabled, addr;                                    \
                                                                \
    /* Save current cache mode (disabled/enabled) */            \
    HAL_ICACHE_IS_ENABLED(cache_enabled);                       \
                                                                \
    /* Disable cache, so that invalidation ignores cache tags */\
    HAL_ICACHE_DISABLE();                                       \
    addr = HAL_ICACHE_SIZE;                                     \
    do {                                                        \
        MTSPR(SPR_ICBIR, addr);                                 \
        addr -= HAL_ICACHE_LINE_SIZE;                           \
    } while (addr > 0);                                         \
                                                                \
    /* Re-enable cache if it was enabled on entry */            \
    HAL_SET_ICACHE_ENABLED(cache_enabled);                      \
    CYG_MACRO_END

// Synchronize the contents of the cache with memory.
#define HAL_ICACHE_SYNC() HAL_ICACHE_INVALIDATE_ALL()

// Query the state of the instruction cache
#define HAL_ICACHE_IS_ENABLED(_state_)                          \
    CYG_MACRO_START                                             \
    (_state_) = (1 - !(MFSPR(SPR_SR) & SPR_SR_ICE));            \
    CYG_MACRO_END

#endif //#ifndef HAL_ICACHE_SIZE

#endif /* __ASSEMBLER__ */

#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
