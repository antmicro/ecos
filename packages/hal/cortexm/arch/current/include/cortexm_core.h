#ifndef CYGONCE_CORTEXM_CORE_H
#define CYGONCE_CORTEXM_CORE_H
//==========================================================================
//
//      cortexm_core.h
//
//      Cortex-M some core registers
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
// Date:           2012-06-26
// Description:    Some Cortex-M core register definitions
//
//####DESCRIPTIONEND####
//
//========================================================================


// Coprocessor Access Control Register
#define CYGARC_REG_FPU_CPACR 0xE000ED88

#define CYGARC_REG_FPU_CPACR_ACCESS_DENIED     0
#define CYGARC_REG_FPU_CPACR_ACCESS_PRIVILEGED 1
#define CYGARC_REG_FPU_CPACR_ACCESS_RESERVED   2
#define CYGARC_REG_FPU_CPACR_ACCESS_FULL       (CYGARC_REG_FPU_CPACR_ACCESS_PRIVILEGED | \
                                               CYGARC_REG_FPU_CPACR_ACCESS_RESERVED)

#define CYGARC_REG_FPU_CPACR_CP10(_access) ((_access) << 20)
#define CYGARC_REG_FPU_CPACR_CP11(_access) ((_access) << 22)

#define CYGARC_REG_FPU_CPACR_ENABLE                                       \
           (CYGARC_REG_FPU_CPACR_CP10(CYGARC_REG_FPU_CPACR_ACCESS_FULL) | \
            CYGARC_REG_FPU_CPACR_CP11(CYGARC_REG_FPU_CPACR_ACCESS_FULL))

// CONTROL register
// The CONTROL register is not memory mapped. Use CYGARC_MSR() and CYGARC_MRS().
#define CYGARC_REG_CONTROL_PRIV_M       0x1
#define CYGARC_REG_CONTROL_SPSEL_M      0x2
#define CYGARC_REG_CONTROL_FPCA_M       0x4

//==========================================================================
#endif //CYGONCE_CORTEXM_CORE_H
