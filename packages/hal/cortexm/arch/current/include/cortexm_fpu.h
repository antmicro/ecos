#ifndef CYGONCE_CORTEXM_FPU_H
#define CYGONCE_CORTEXM_FPU_H
//==========================================================================
//
//      cortexm_fpu.h
//
//      Cortex-M General Floating Point Unit definitions 
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
// Date:           2012-04-25
// Description:    Cortex-M4F General Floating Point Unit definitions and macros
// Usage:          include <cyg/hal/cortexm_fpu.h>
//
//####DESCRIPTIONEND####
//
//========================================================================


//===========================================================================
// Floating Point Unit
//
// FPU is optional unit of Cortex-M4

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm.h>
#include <cyg/hal/cortexm_core.h>

#ifdef CYGHWR_HAL_CORTEXM_FPU

# if defined CYGSEM_HAL_ROM_MONITOR || defined CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#  define CYGSEM_HAL_DEBUG_FPU
# endif

# ifdef CYGINT_HAL_FPV4_SP_D16
#  include <cyg/hal/fpv4_sp_d16.h>
# else
#  error "Unknown FPU unit!"
# endif

#else // CYGHWR_HAL_CORTEXM_FPU

# define CYGARC_CORTEXM_GDB_REG_FPA

# define GDB_STUB_SAVEDREG_FRAME_TYPE(__type) (__type->u.type)

# define HAL_SAVEDREG_AUTO_FRAME_SIZE (8*4)

# define HAL_SAVEDREG_FPU_THREAD_S
# define HAL_SAVEDREG_MAN_FPU_EXCEPTION_S
# define HAL_SAVEDREG_AUTO_FPU_EXCEPTION_S
# define HAL_THREAD_INIT_FPU_CONTEXT(__regs) CYG_EMPTY_STATEMENT

# define GDB_STUB_SAVEDREG_FPU_THREAD_GET(__gdbreg,__regs) CYG_EMPTY_STATEMENT
# define GDB_STUB_SAVEDREG_FPU_THREAD_SET(__gdbreg,__regs) CYG_EMPTY_STATEMENT
# define GDB_STUB_SAVEDREG_FPU_EXCEPTION_GET(__gdbreg,__regs) CYG_EMPTY_STATEMENT
# define GDB_STUB_SAVEDREG_FPU_EXCEPTION_SET(__gdbreg,__regs) CYG_EMPTY_STATEMENT

#endif// CYGHWR_HAL_CORTEXM_FPU


//==========================================================================
#endif //CYGONCE_CORTEXM_FPU_H
