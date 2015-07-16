#ifndef CYGONCE_HAL_PLF_IO_H
#define CYGONCE_HAL_PLF_IO_H
//=============================================================================
//
//      plf_io.h
//
//      Platform specific registers
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Antmicro Ltd <contact@antmicro.com>
// Based on:	{...}/hal/cortexm/kinetis/twr_k70f120m/current/include/plf_io.h
// Date:        2014-03-28
// Purpose:     Colibri VF61 platform specific registers
// Description:
// Usage:       #include <cyg/hal/plf_io.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm_vybrid_col_vf61.h>

#ifndef CYGHWR_HAL_FREESCALE_UART0_PIN_RX
# define CYGHWR_HAL_FREESCALE_UART0_PIN_RX 	CYGHWR_HAL_VYBRID_PIN(B, 11, 1, 0)
# define CYGHWR_HAL_FREESCALE_UART0_PIN_TX 	CYGHWR_HAL_VYBRID_PIN(B, 10, 1, 0)
# define CYGHWR_HAL_FREESCALE_UART0_PIN_RTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART0_PIN_CTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_IO_FREESCALE_UART0_PIN_RX 	CYGHWR_HAL_FREESCALE_UART0_PIN_RX
# define CYGHWR_IO_FREESCALE_UART0_PIN_TX 	CYGHWR_HAL_FREESCALE_UART0_PIN_TX
# define CYGHWR_IO_FREESCALE_UART0_PIN_RTS 	CYGHWR_HAL_FREESCALE_UART0_PIN_RTS
# define CYGHWR_IO_FREESCALE_UART0_PIN_CTS 	CYGHWR_HAL_FREESCALE_UART0_PIN_CTS
#endif

#ifndef CYGHWR_HAL_FREESCALE_UART1_PIN_RX
# define CYGHWR_HAL_FREESCALE_UART1_PIN_RX 	CYGHWR_HAL_VYBRID_PIN(B, 5, 2, 0)
# define CYGHWR_HAL_FREESCALE_UART1_PIN_TX	CYGHWR_HAL_VYBRID_PIN(B, 4, 2, 0)
# define CYGHWR_HAL_FREESCALE_UART1_PIN_RTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART1_PIN_CTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_IO_FREESCALE_UART1_PIN_RX 	CYGHWR_HAL_FREESCALE_UART1_PIN_RX
# define CYGHWR_IO_FREESCALE_UART1_PIN_TX 	CYGHWR_HAL_FREESCALE_UART1_PIN_TX
# define CYGHWR_IO_FREESCALE_UART1_PIN_RTS	CYGHWR_HAL_FREESCALE_UART1_PIN_RTS
# define CYGHWR_IO_FREESCALE_UART1_PIN_CTS	CYGHWR_HAL_FREESCALE_UART1_PIN_CTS
#endif

#ifndef CYGHWR_HAL_FREESCALE_UART2_PIN_RX
# define CYGHWR_HAL_FREESCALE_UART2_PIN_RX  CYGHWR_HAL_VYBRID_PIN(D, 1, 2, 0)
# define CYGHWR_HAL_FREESCALE_UART2_PIN_TX  CYGHWR_HAL_VYBRID_PIN(D, 0, 2, 0)
# define CYGHWR_HAL_FREESCALE_UART2_PIN_RTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART2_PIN_CTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_IO_FREESCALE_UART2_PIN_RX 	CYGHWR_HAL_FREESCALE_UART2_PIN_RX
# define CYGHWR_IO_FREESCALE_UART2_PIN_TX 	CYGHWR_HAL_FREESCALE_UART2_PIN_TX
# define CYGHWR_IO_FREESCALE_UART2_PIN_RTS 	CYGHWR_HAL_FREESCALE_UART2_PIN_RTS
# define CYGHWR_IO_FREESCALE_UART2_PIN_CTS 	CYGHWR_HAL_FREESCALE_UART2_PIN_CTS
#endif

#ifndef CYGHWR_HAL_FREESCALE_UART3_PIN_RX
# define CYGHWR_HAL_FREESCALE_UART3_PIN_RX  CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART3_PIN_TX  CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART3_PIN_RTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART3_PIN_CTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_IO_FREESCALE_UART3_PIN_RX 	CYGHWR_HAL_FREESCALE_UART3_PIN_RX
# define CYGHWR_IO_FREESCALE_UART3_PIN_TX 	CYGHWR_HAL_FREESCALE_UART3_PIN_TX
# define CYGHWR_IO_FREESCALE_UART3_PIN_RTS 	CYGHWR_HAL_FREESCALE_UART3_PIN_RTS
# define CYGHWR_IO_FREESCALE_UART3_PIN_CTS 	CYGHWR_HAL_FREESCALE_UART3_PIN_CTS
#endif

#ifndef CYGHWR_HAL_FREESCALE_UART4_PIN_RX
# define CYGHWR_HAL_FREESCALE_UART4_PIN_RX  CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART4_PIN_TX  CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART4_PIN_RTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART4_PIN_CTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_IO_FREESCALE_UART4_PIN_RX 	CYGHWR_HAL_FREESCALE_UART4_PIN_RX
# define CYGHWR_IO_FREESCALE_UART4_PIN_TX 	CYGHWR_HAL_FREESCALE_UART4_PIN_TX
# define CYGHWR_IO_FREESCALE_UART4_PIN_RTS 	CYGHWR_HAL_FREESCALE_UART4_PIN_RTS
# define CYGHWR_IO_FREESCALE_UART4_PIN_CTS 	CYGHWR_HAL_FREESCALE_UART4_PIN_CTS
#endif

#ifndef CYGHWR_IO_FREESCALE_UART5_PIN_RX
# define CYGHWR_HAL_FREESCALE_UART5_PIN_RX 	CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART5_PIN_TX 	CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART5_PIN_RTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_HAL_FREESCALE_UART5_PIN_CTS CYGHWR_HAL_VYBRID_PORT_PIN_NONE
# define CYGHWR_IO_FREESCALE_UART5_PIN_RX 	CYGHWR_HAL_FREESCALE_UART5_PIN_RX
# define CYGHWR_IO_FREESCALE_UART5_PIN_TX 	CYGHWR_HAL_FREESCALE_UART5_PIN_TX
# define CYGHWR_IO_FREESCALE_UART5_PIN_RTS 	CYGHWR_HAL_FREESCALE_UART5_PIN_RTS
# define CYGHWR_IO_FREESCALE_UART5_PIN_CTS 	CYGHWR_HAL_FREESCALE_UART5_PIN_CTS
#endif

//=============================================================================
// Memory access checks.
//
// Accesses to areas not backed by real devices or memory can cause
// the CPU to hang. These macros allow the GDB stubs to avoid making
// accidental accesses to these areas.

__externC int cyg_hal_stub_permit_data_access( void* addr, cyg_uint32 count );
#define CYG_HAL_STUB_PERMIT_DATA_READ(_addr_, _count_) cyg_hal_stub_permit_data_access( _addr_, _count_ )
#define CYG_HAL_STUB_PERMIT_DATA_WRITE(_addr_, _count_ ) cyg_hal_stub_permit_data_access( _addr_, _count_ )

//=============================================================================


//-----------------------------------------------------------------------------
// end of plf_io.h
#endif // CYGONCE_HAL_PLF_IO_H

