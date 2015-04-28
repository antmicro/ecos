#ifndef CYGONCE_FPV4_SP_D16_LIBM_H
#define CYGONCE_FPV4_SP_D16_LIBM_H
//==========================================================================
//
//      fpv4_sp_d16_libm.h
//
//      FPv4spD16 Floating Point Unit mathematical functions
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2013 Free Software Foundation, Inc.                        
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
// Date:           2013-06-10
// Description:    FPv4spD16 Floating Point Unit builtin mathematical functions.
// Usage:          include <cyg/hal/fpv4_sp_d16_libm.h>
//
//####DESCRIPTIONEND####
//
//========================================================================

#ifdef CYGSEM_LIBM_IEEE_API_INLINE

// Builtin mathematical functions
#define __ieee754_sqrtf(__x) __builtin_sqrtf(__x)

#endif // CYGSEM_LIBM_IEEE_API_INLINE

//==========================================================================
#endif //CYGONCE_FPV4_SP_D16_LIBM_H
