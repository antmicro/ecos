#ifndef CYGONCE_PLF_CACHE_H
#define CYGONCE_PLF_CACHE_H

//=============================================================================
//
//      plf_cache.h
//
//      Platform HAL cache details
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
// Author(s):   Piotr Skrzypek
// Contributors:
// Date:        2012-04-02
// Purpose:     Platform cache control API
// Description: The macros defined here provide the platform specific
//              cache control operations / behavior.
// Usage:       Is included via the architecture cache header:
//              #include <cyg/hal/hal_cache.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

//-----------------------------------------------------------------------------
// Data Cache dimensions
//
// Size of the data cache can be adjusted in the configuration file.
#define HAL_DCACHE_SIZE                 CYGHWR_DCACHE_SIZE
//
// Default line size is 16 bytes. Only 32KB cache has line size of 32 bytes.
#if CYGHWR_DCACHE_SIZE == 0x8000
#define HAL_DCACHE_LINE_SIZE            32
#else
#define HAL_DCACHE_LINE_SIZE            16
#endif
//
// Currently only 1 way cache is implemented.
#define HAL_DCACHE_WAYS                 1
//
// Cache mode (write-through / write-back) can be selected in the configuration
// file. Default mode is write-through. Cache mode is selected at synthesis 
// time and cannot be configured by the software.
#if defined(CYGHWR_DCACHE_MODE_WRITETHROUGH)
#define HAL_DCACHE_MODE_WRITETHROUGH
#elif defined(CYGHWR_DCACHE_MODE_WRITEBACK)
#define HAL_DCACHE_MODE_WRITEBACK
#else
#error Unsupported cache mode
#endif
//
// Compute the number of sets based on previous values
#define HAL_DCACHE_SETS (HAL_DCACHE_SIZE/(HAL_DCACHE_LINE_SIZE*HAL_DCACHE_WAYS))

//-----------------------------------------------------------------------------
// Instruction Cache dimensions
//
// Size of the instruction cache can be adjusted in the configuration file.
#define HAL_ICACHE_SIZE                 CYGHWR_ICACHE_SIZE
//
// Default line size is 16 bytes. Only 32KB cache has line size of 32 bytes.
#if CYGHWR_ICACHE_SIZE == 0x8000
#define HAL_ICACHE_LINE_SIZE            32
#else
#define HAL_ICACHE_LINE_SIZE            16
#endif
//
// Currently only 1 way cache is implemented.
#define HAL_ICACHE_WAYS                 1
//
// Compute the number of sets based on previous values
#define HAL_ICACHE_SETS (HAL_ICACHE_SIZE/(HAL_ICACHE_LINE_SIZE*HAL_ICACHE_WAYS))

#endif // ifndef CYGONCE_PLF_CACHE_H
// End of plf_cache.h
