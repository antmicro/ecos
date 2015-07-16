#ifndef CYGONCE_HAL_VAR_IOCLKGAT_H
#define CYGONCE_HAL_VAR_IOCLKGAT_H
//===========================================================================
//
//      var_io_clkgat.h
//
//      Vybrid clock gating
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Antmicro Ltd <contact@antmicro.com>
// Based on:	 {...}/hal/packages/cortexm/kinetis/var/current/include/var_io_clkgat.h
// Date:         2014-03-28
// Purpose:      Vybrid clock distribution macros
// Description:
// Usage:         This file is included by <cyg/hal/var_io.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================

//---------------------------------------------------------------------------
// Clock distribution

// Clock gating definitions
#define CYGHWR_HAL_VYBRID_CCM_CCGR_UART0 CYGHWR_HAL_VYBRID_CCM_CCGR(0, 7)
#define CYGHWR_HAL_VYBRID_CCM_CCGR_UART1 CYGHWR_HAL_VYBRID_CCM_CCGR(0, 8)
#define CYGHWR_HAL_VYBRID_CCM_CCGR_UART2 CYGHWR_HAL_VYBRID_CCM_CCGR(0, 9)
#define CYGHWR_HAL_VYBRID_CCM_CCGR_UART3 CYGHWR_HAL_VYBRID_CCM_CCGR(0, 10)
#define CYGHWR_HAL_VYBRID_CCM_CCGR_UART4 CYGHWR_HAL_VYBRID_CCM_CCGR(6, 105)
#define CYGHWR_HAL_VYBRID_CCM_CCGR_UART5 CYGHWR_HAL_VYBRID_CCM_CCGR(6, 106)


#endif // CYGONCE_HAL_VAR_IOCLKGAT_H
