#ifndef CYGONCE_XC7Z020_H
#define CYGONCE_XC7Z020_H

//=============================================================================
//
//      xc7z020.h
//
//      Platform specific support (register layout, etc)
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
// Author(s):    ITR-GmbH
// Contributors: 
// Date:         2012-06-27
// Purpose:      Zynq-7000 platform specific support definitions
// Description: 
// Usage:        #include <cyg/hal/xc7z020.h>
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal_arm_xc7z020.h>

// Memory mapping

// Canonical definitions for DDR MEMORY
#define XC7Z_DDR_MEM_BASEADDR           0x00000000
#define XC7Z_DDR_MEM_HIGHADDR           0x3FFFFFFF 

//
// This block contains constant declarations for the peripherals
// within the hardblock. These have been put for bacwards compatibilty
//

#define XC7Z_PERIPHERAL_BASEADDR        0xE0000000
#define XC7Z_UART0_BASEADDR             0xE0000000
#define XC7Z_UART1_BASEADDR             0xE0001000
#define XC7Z_USB0_BASEADDR              0xE0002000
#define XC7Z_USB1_BASEADDR              0xE0003000
#define XC7Z_I2C0_BASEADDR              0xE0004000
#define XC7Z_I2C1_BASEADDR              0xE0005000
#define XC7Z_SPI0_BASEADDR              0xE0006000
#define XC7Z_SPI1_BASEADDR              0xE0007000
#define XC7Z_CAN0_BASEADDR              0xE0008000
#define XC7Z_CAN1_BASEADDR              0xE0009000
#define XC7Z_GPIO_BASEADDR              0xE000A000
#define XC7Z_GEM0_BASEADDR              0xE000B000
#define XC7Z_GEM1_BASEADDR              0xE000C000
#define XC7Z_QSPI_BASEADDR              0xE000D000
#define XC7Z_PARPORT_CRTL_BASEADDR      0xE000E000
#define XC7Z_SDIO0_BASEADDR             0xE0100000
#define XC7Z_SDIO1_BASEADDR             0xE0101000
#define XC7Z_IOU_BUS_CFG_BASEADDR       0xE0200000
#define XC7Z_NAND_BASEADDR              0xE1000000
#define XC7Z_PARPORT0_BASEADDR          0xE2000000
#define XC7Z_PARPORT1_BASEADDR          0xE4000000
#define XC7Z_QSPI_LINEAR_BASEADDR       0xFC000000
#define XC7Z_SYS_CTRL_BASEADDR          0xF8000000  /* AKA SLCR */
#define XC7Z_TTC0_BASEADDR              0xF8001000
#define XC7Z_TTC1_BASEADDR              0xF8002000
#define XC7Z_DMAC0_SEC_BASEADDR         0xF8003000
#define XC7Z_DMAC0_NON_SEC_BASEADDR     0xF8004000
#define XC7Z_WDT_BASEADDR               0xF8005000
#define XC7Z_DDR_CTRL_BASEADDR          0xF8006000
#define XC7Z_DEV_CFG_APB_BASEADDR       0xF8007000
#define XC7Z_AFI0_BASEADDR              0xF8008000
#define XC7Z_AFI1_BASEADDR              0xF8009000
#define XC7Z_AFI2_BASEADDR              0xF800A000
#define XC7Z_AFI3_BASEADDR              0xF800B000
#define XC7Z_OCM_BASEADDR               0xF800C000
#define XC7Z_EFUSE_BASEADDR             0xF800D000
#define XC7Z_CORESIGHT_BASEADDR         0xF8800000
#define XC7Z_TOP_BUS_CFG_BASEADDR       0xF8900000
#define XC7Z_SCU_PERIPH_BASE            0xF8F00000
#define XC7Z_L2CC_BASEADDR              0xF8F02000
#define XC7Z_SAM_RAM_BASEADDR           0xFFFC0000
#define XC7Z_FPGA_AXI_S0_BASEADDR       0x40000000
#define XC7Z_FPGA_AXI_S1_BASEADDR       0x80000000
#define XC7Z_IOU_S_SWITCH_BASEADDR      0xE0000000
#define XC7Z_PERIPH_APB_BASEADDR        0xF8000000
#define XC7Z_SCU_TIMER_BASEADDR         0xF8F00600
#define XC7Z_SCU_WDT_BASEADDR           0xF8F00620
 
#define XC7Z_XQSPIPS_0_LINEAR_BASEADDR  XC7Z_QSPI_LINEAR_BASEADDR
#define XC7Z_XPARPORTPS_CTRL_BASEADDR   XC7Z_PARPORT_CRTL_BASEADDR

// Canonical definitions for SLCR
#define XC7Z_XSLCR_0_BASEADDR           XC7Z_SYS_CTRL_BASEADDR

// Canonical definitions for SCU GIC
#define XC7Z_SCUGIC_CPU_BASEADDR        (XC7Z_SCU_PERIPH_BASE + 0x0100)
#define XC7Z_SCUGIC_DIST_BASEADDR       (XC7Z_SCU_PERIPH_BASE + 0x1000)

// Canonical definitions for Global Timer
#define XC7Z_GLOBAL_TMR_BASEADDR        (XC7Z_SCU_PERIPH_BASE + 0x0200)
 



#endif // CYGONCE_XC7Z020_H
//-----------------------------------------------------------------------------
// end of xc7z020.h
