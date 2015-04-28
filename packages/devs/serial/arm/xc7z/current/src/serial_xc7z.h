// ====================================================================
//
//      serial_xc7z.h
//
//      Xilinx Zynq Cadence UART driver
//
// ====================================================================
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
// ====================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):           Ant Micro <www.antmicro.com>
// Contributors:        
// Date:                29.08.2012
// Purpose:             Xilinx Zynq Cadence UART driver
// Description:
//
//####DESCRIPTIONEND####
//
// ====================================================================


#define PS_CLK          33333000
#define UART_REF_CLK    50000000

#define XUARTPS0_BASE 			    0xE0000000
#define XUARTPS1_BASE 			    0xE0001000

// UART registers address offset
#define XUARTPS_CONTROL 	        0x00
#define XUARTPS_MODE 			    0x04
#define XUARTPS_INTRPT_EN           0x08
#define XUARTPS_INTRPT_DIS	        0x0C
#define XUARTPS_CHNL_INT_STS   		0x14
#define XUARTPS_BAUD_RATE_GEN  		0x18
#define XUARTPS_RX_TOUT	            0x1C
#define XUARTPS_RX_WM	            0x20
#define XUARTPS_TX_RX_FIFO 		    0x30
#define XUARTPS_BAUD_RATE_DIVIDER 	0x34
#define XUARTPS_CHANNEL_STS         0x2C

// Control Register bits definition
#define XUARTPS_CONTROL_TXDIS 		0x00000020
#define XUARTPS_CONTROL_TXEN 		0x00000010
#define XUARTPS_CONTROL_RXDIS       0x00000008
#define XUARTPS_CONTROL_RXEN        0x00000004
#define XUARTPS_CONTROL_TXRES       0x00000002
#define XUARTPS_CONTROL_RXRES       0x00000001

// Interrupt Enable Register bits definition
#define XUARTPS_INTRPT_EN_TOVR      0x00001000
#define XUARTPS_INTRPT_EN_TNFUL     0x00000800
#define XUARTPS_INTRPT_EN_TIMEOUT   0x00000100
#define XUARTPS_INTRPT_EN_TFUL      0x00000010
#define XUARTPS_INTRPT_EN_TEMPTY    0x00000008
#define XUARTPS_INTRPT_EN_RFULL     0x00000004
#define XUARTPS_INTRPT_EN_REMPTY    0x00000002
#define XUARTPS_INTRPT_EN_RTRIG     0x00000001

// Channel Interrupt Status Register bits definition
#define XUARTPS_CHNL_INT_STS_RTOUT  0x00000100
#define XUARTPS_CHNL_INT_STS_TFULL  0x00000010
#define XUARTPS_CHNL_INT_STS_TEMPTY 0x00000008
#define XUARTPS_CHNL_INT_STS_RFULL  0x00000004
#define XUARTPS_CHNL_INT_STS_REMPTY 0x00000002
#define XUARTPS_CHNL_INT_STS_RTRIG  0x00000001

// Channel Status register bits definition
#define XUARTPS_CHANNEL_STS_TNFUL   0x00000400
#define XUARTPS_CHANNEL_STS_TTRIG   0x00000200
#define XUARTPS_CHANNEL_STS_FDELT   0x00000100
#define XUARTPS_CHANNEL_STS_TACTIVE 0x00000800
#define XUARTPS_CHANNEL_STS_RACTIVE 0x00000400
#define XUARTPS_CHANNEL_STS_DMSI    0x00000200
#define XUARTPS_CHANNEL_STS_TIMEOUT 0x00000100
#define XUARTPS_CHANNEL_STS_PARE    0x00000080
#define XUARTPS_CHANNEL_STS_FRAME   0x00000040
#define XUARTPS_CHANNEL_STS_ROVR    0x00000020
#define XUARTPS_CHANNEL_STS_TFUL    0x00000010
#define XUARTPS_CHANNEL_STS_TEMPTY  0x00000008
#define XUARTPS_CHANNEL_STS_RFUL    0x00000004
#define XUARTPS_CHANNEL_STS_REMPTY  0x00000002
#define XUARTPS_CHANNEL_STS_RTRIG   0x00000001

