#ifndef CYGONCE_HAL_VAR_IO_H
#define CYGONCE_HAL_VAR_IO_H
//===========================================================================
//
//      var_io.h
//
//      Variant specific registers
//
//===========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2011, 2013 Free Software Foundation, Inc.                        
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
// Based on:	 {...}/hal/packages/cortexm/kinetis/var/current/include/var_io.h
// Date:         2014-03-28
// Purpose:       Vybrid variant specific registers
// Description:
// Usage:         #include <cyg/hal/var_io.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================

#include <pkgconf/hal_cortexm_vybrid.h>

#include <cyg/hal/plf_io.h>

//===========================================================================
// Cortex-M architecture
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Cortex-M architecture overrides
//---------------------------------------------------------------------------
// VTOR - Vector Table Offset Register

#ifndef CYGARC_REG_NVIC_VTOR_TBLBASE_SRAM
#ifdef CYG_HAL_VTOR_ADRESS
#define CYGARC_REG_NVIC_VTOR_TBLBASE_SRAM CYG_HAL_VTOR_ADRESS
#else
#define CYGARC_REG_NVIC_VTOR_TBLBASE_SRAM 0x1f800000
#endif
#endif

//===========================================================================
// Vybrid Memory layout
//---------------------------------------------------------------------------
#define CYGHWR_HAL_VYBRID_OCRAM1_BASE   0x3f040000
#define CYGHWR_HAL_VYBRID_OCRAM1_SIZE   0x0003ffff

//===========================================================================
// Vybrid Peripherals
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// MSCM - Miscellaneous System Control Module
#define CYGHWR_HAL_VYBRID_MSCM_BASE                     0x40001000
#define CYGHWR_HAL_VYBRID_MSCM_CPxTYPE                  (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x000)
#define CYGHWR_HAL_VYBRID_MSCM_CPxTYPE_PERSONALITY_M    0xFFFFFF00
#define CYGHWR_HAL_VYBRID_MSCM_CPxTYPE_PERSONALITY_S    8
#define CYGHWR_HAL_VYBRID_MSCM_CPxTYPE_RYPZ_M           0x000000FF
#define CYGHWR_HAL_VYBRID_MSCM_CPxTYPE_RYPZ_S           0
#define CYGHWR_HAL_VYBRID_MSCM_CPxNUM                   (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x004)
#define CYGHWR_HAL_VYBRID_MSCM_CPxNUM_CPN_M             0x00000001
#define CYGHWR_HAL_VYBRID_MSCM_CPxNUM_CPN_S             0
#define CYGHWR_HAL_VYBRID_MSCM_CPxMASTER                (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x008)
#define CYGHWR_HAL_VYBRID_MSCM_CPxMASTER_PPN_M          0x0000001f
#define CYGHWR_HAL_VYBRID_MSCM_CPxMASTER_PPN_S          0
#define CYGHWR_HAL_VYBRID_MSCM_CPxCOUNT                 (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x00c)
#define CYGHWR_HAL_VYBRID_MSCM_CPxNUM_PCNT_M            0x00000001
#define CYGHWR_HAL_VYBRID_MSCM_CPxNUM_PCNT_S            0
#define CYGHWR_HAL_VYBRID_MSCM_CPxCFG0                  (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x010)
#define CYGHWR_HAL_VYBRID_MSCM_CPxCFG1                  (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x014)
#define CYGHWR_HAL_VYBRID_MSCM_CPxCFG2                  (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x018)
#define CYGHWR_HAL_VYBRID_MSCM_CPxCFG3                  (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x01c)
#define CYGHWR_HAL_VYBRID_MSCM_IRCP0IR                  (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x800)
#define CYGHWR_HAL_VYBRID_MSCM_IRCP0IR_INT0_M           0x00000001
#define CYGHWR_HAL_VYBRID_MSCM_IRCP0IR_INT0_S           0
#define CYGHWR_HAL_VYBRID_MSCM_IRCP0IR_INT1_M           0x00000002
#define CYGHWR_HAL_VYBRID_MSCM_IRCP0IR_INT1_S           1
#define CYGHWR_HAL_VYBRID_MSCM_IRCP0IR_INT2_M           0x00000004
#define CYGHWR_HAL_VYBRID_MSCM_IRCP0IR_INT2_S           2
#define CYGHWR_HAL_VYBRID_MSCM_IRCP0IR_INT3_M           0x00000008
#define CYGHWR_HAL_VYBRID_MSCM_IRCP0IR_INT3_S           3
#define CYGHWR_HAL_VYBRID_MSCM_IRCP1IR                  (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x804)
#define CYGHWR_HAL_VYBRID_MSCM_IRCP1IR_INT0_M           0x00000001
#define CYGHWR_HAL_VYBRID_MSCM_IRCP1IR_INT0_S           0
#define CYGHWR_HAL_VYBRID_MSCM_IRCP1IR_INT1_M           0x00000002
#define CYGHWR_HAL_VYBRID_MSCM_IRCP1IR_INT1_S           1
#define CYGHWR_HAL_VYBRID_MSCM_IRCP1IR_INT2_M           0x00000004
#define CYGHWR_HAL_VYBRID_MSCM_IRCP1IR_INT2_S           2
#define CYGHWR_HAL_VYBRID_MSCM_IRCP1IR_INT3_M           0x00000008
#define CYGHWR_HAL_VYBRID_MSCM_IRCP1IR_INT3_S           3
#define CYGHWR_HAL_VYBRID_MSCM_IRCPGIR                  (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x820)
#define CYGHWR_HAL_VYBRID_MSCM_IRCPGIR_INTID_M          0x00000003
#define CYGHWR_HAL_VYBRID_MSCM_IRCPGIR_INTID_S          0
#define CYGHWR_HAL_VYBRID_MSCM_IRCPGIR_CPUTL_M          0x00030000
#define CYGHWR_HAL_VYBRID_MSCM_IRCPGIR_CPUTL_S          16
#define CYGHWR_HAL_VYBRID_MSCM_IRCPGIR_TLF_M            0x03000000
#define CYGHWR_HAL_VYBRID_MSCM_IRCPGIR_TLF_S            24
#define CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_BASE             (CYGHWR_HAL_VYBRID_MSCM_BASE + 0x880)
#define CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_CP1En_M          0x0002
#define CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_CP1En_S          1
#define CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_CP0En_M          0x0001
#define CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_CP0En_S          0
#define CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_RO_M             0x8000
#define CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_RO_S             15

__externC int hal_get_core_num(void);

//---------------------------------------------------------------------------
// SEMA4 - IPS Semaphores
// SEMA4 - Peripheral register structure
typedef volatile struct cyghwr_hal_vybrid_sema4_s
{
    cyg_uint8   gate[16];
    cyg_uint8   reserved0[50];
    cyg_uint16  cp0ine;
    cyg_uint8   reserved1[6];
    cyg_uint16  cp1ine;
    cyg_uint8   reserved2[54];
    cyg_uint16  cp0ntf;
    cyg_uint8   reserved3[6];
    cyg_uint16  cp1ntf;
    cyg_uint8   reserved4[118];
    cyg_uint16  rstgt;
    cyg_uint8   reserved5[2];
    cyg_uint16  rstntf;
} cyghwr_hal_vybrid_sema4_t;

#define CYGHWR_HAL_VYBRID_SEMA4_GATE_GTFSM_M       0x03
#define CYGHWR_HAL_VYBRID_SEMA4_P ((cyghwr_hal_vybrid_sema4_t *) 0x4001d000)

//---------------------------------------------------------------------------
// CCM - Clock Controller Module
// CCM - Peripheral register structure
typedef volatile struct cyghwr_hal_vybrid_ccm_s {
    cyg_uint32 ccr;           // 
    cyg_uint32 csr;           // 
    cyg_uint32 ccsr;          // 
    cyg_uint32 cacrr;         // 
    cyg_uint32 cscmr1;        // 
    cyg_uint32 cscdr1;        // 
    cyg_uint32 cscdr2;        // 
    cyg_uint32 cscdr3;        // 
    cyg_uint32 cscmr2;        // 
    cyg_uint32 reserved0;     //
    cyg_uint32 ctor;          // 
    cyg_uint32 clpcr;         // 
    cyg_uint32 cisr;          // 
    cyg_uint32 cimr;          // 
    cyg_uint32 ccosr;         // 
    cyg_uint32 cgpr;          // 
    cyg_uint32 ccgr0;         // 
    cyg_uint32 ccgr1;         // 
    cyg_uint32 ccgr2;         // 
    cyg_uint32 ccgr3;         // 
    cyg_uint32 ccgr4;         // 
    cyg_uint32 ccgr5;         // 
    cyg_uint32 ccgr6;         // 
    cyg_uint32 ccgr7;         // 
    cyg_uint32 ccgr8;         // 
    cyg_uint32 ccgr9;         // 
    cyg_uint32 ccgr10;        // 
    cyg_uint32 ccgr11;        // 
    cyg_uint32 cmeor0;        // 
    cyg_uint32 cmeor1;        // 
    cyg_uint32 cmeor2;        // 
    cyg_uint32 cmeor3;        // 
    cyg_uint32 cmeor4;        // 
    cyg_uint32 cmeor5;        // 
    cyg_uint32 cppdsr;        // 
    cyg_uint32 ccowr;         // 
    cyg_uint32 ccpgr0;        // 
    cyg_uint32 ccpgr1;        // 
    cyg_uint32 ccpgr2;        // 
    cyg_uint32 ccpgr3;        // 

} cyghwr_hal_vybrid_ccm_t;


#define CYGHWR_HAL_VYBRID_CCM_P ((cyghwr_hal_vybrid_ccm_t *) 0x4006B000)

// CCR Fields
#define CYGHWR_HAL_VYBRID_CCM_CCR_FIRC_M				0x00010000
#define CYGHWR_HAL_VYBRID_CCM_CCR_FIRC_S				16
#define CYGHWR_HAL_VYBRID_CCM_CCR_FXOSC_EN_M			0x00001000
#define CYGHWR_HAL_VYBRID_CCM_CCR_FXOSC_EN_S			12
#define CYGHWR_HAL_VYBRID_CCM_CCR_OSCNT_M				0x000000ff
#define CYGHWR_HAL_VYBRID_CCM_CCR_OSCNT_S				0

// CSR Fields
#define CYGHWR_HAL_VYBRID_CCM_CSR_FXOSC_RDY_M			0x00000020
#define CYGHWR_HAL_VYBRID_CCM_CSR_FXOSC_RDY_S			5


// CACRR Bit Fields
#define CYGHWR_HAL_VYBRID_CCM_CACRR_FLEX_CLK_DIV_M      0x1C00000
#define CYGHWR_HAL_VYBRID_CCM_CACRR_FLEX_CLK_DIV_S      22
#define CYGHWR_HAL_VYBRID_CCM_CACRR_PLL6_CLK_DIV_M      0x0200000
#define CYGHWR_HAL_VYBRID_CCM_CACRR_PLL6_CLK_DIV_S      21
#define CYGHWR_HAL_VYBRID_CCM_CACRR_PLL3_CLK_DIV_M      0x0100000
#define CYGHWR_HAL_VYBRID_CCM_CACRR_PLL3_CLK_DIV_S      20
#define CYGHWR_HAL_VYBRID_CCM_CACRR_PLL1_PFD_CLK_DIV_M  0x0030000
#define CYGHWR_HAL_VYBRID_CCM_CACRR_PLL1_PFD_CLK_DIV_S  16
#define CYGHWR_HAL_VYBRID_CCM_CACRR_IPG_CLK_DIV_M       0x0001800
#define CYGHWR_HAL_VYBRID_CCM_CACRR_IPG_CLK_DIV_S       11
#define CYGHWR_HAL_VYBRID_CCM_CACRR_PLL4_CLK_DIV_M      0x00001C0
#define CYGHWR_HAL_VYBRID_CCM_CACRR_PLL4_CLK_DIV_S      6
#define CYGHWR_HAL_VYBRID_CCM_CACRR_BUS_CLK_DIV_M       0x0000038
#define CYGHWR_HAL_VYBRID_CCM_CACRR_BUS_CLK_DIV_S       3
#define CYGHWR_HAL_VYBRID_CCM_CACRR_ARM_CLK_DIV_M       0x0000007
#define CYGHWR_HAL_VYBRID_CCM_CACRR_ARM_CLK_DIV_S       0

// CCSR Bit Fields
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL3_PFD4_EN_M       0x80000000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL3_PFD4_EN_S       31
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL3_PFD3_EN_M       0x40000000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL3_PFD3_EN_S       30
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL3_PFD2_EN_M       0x20000000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL3_PFD2_EN_S       29
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL3_PFD1_EN_M       0x10000000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL3_PFD1_EN_S       28
#define CYGHWR_HAL_VYBRID_CCM_CCSR_DAP_EN_M     	  	0x01000000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_DAP_EN_S 	    	24
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD_CLK_SEL_M  	0x00380000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD_CLK_SEL_S  	19
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD_CLK_SEL_M	0x00070000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD_CLK_SEL_S	16
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD4_EN_M       0x00008000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD4_EN_S       15
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD3_EN_M       0x00004000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD3_EN_S       14
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD2_EN_M       0x00002000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD2_EN_S       13
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD1_EN_M       0x00001000
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD1_EN_S       12
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD4_EN_M       0x00000800
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD4_EN_S       11
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD3_EN_M       0x00000400
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD3_EN_S       10
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD2_EN_M       0x00000200
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD2_EN_S       9
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD1_EN_M       0x00000100
#define CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD1_EN_S       8
#define CYGHWR_HAL_VYBRID_CCM_CCSR_DDRC_CLK_SEL_M       0x00000040
#define CYGHWR_HAL_VYBRID_CCM_CCSR_DDRC_CLK_SEL_S       6
#define CYGHWR_HAL_VYBRID_CCM_CCSR_FAST_CLK_SEL_M       0x00000020
#define CYGHWR_HAL_VYBRID_CCM_CCSR_FAST_CLK_SEL_S       5
#define CYGHWR_HAL_VYBRID_CCM_CCSR_SLOW_CLK_SEL_M       0x00000010
#define CYGHWR_HAL_VYBRID_CCM_CCSR_SLOW_CLK_SEL_S       4
#define CYGHWR_HAL_VYBRID_CCM_CCSR_SYS_CLK_SEL_M        0x00000007
#define CYGHWR_HAL_VYBRID_CCM_CCSR_SYS_CLK_SEL_S        0


// CSCMR1 Bit Fields
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_DCU1_CLK_SEL_M      0x20000000
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_DCU1_CLK_SEL_S      29
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_DCU0_CLK_SEL_M      0x10000000
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_DCU0_CLK_SEL_S      28
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_QSPI1_CLK_SEL_M     0x03000000
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_QSPI1_CLK_SEL_S     24
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_QSPIO_CLK_SEL_M     0x00C00000
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_QSPIO_CLK_SEL_S     22
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_ESAI_CLK_SEL_M      0x00300000
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_ESAI_CLK_SEL_S      20
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_ESDHC1_CLK_SEL_M    0x000C0000
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_ESDHC1_CLK_SEL_S    18
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_ESDHC0_CLK_SEL_M    0x00030000
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_ESDHC0_CLK_SEL_S    16
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_NFC_CLK_SEL_M       0x00003000
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_NFC_CLK_SEL_S       12
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_VADC_CLK_SEL_M      0x00000300
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_VADC_CLK_SEL_S      8
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_SAI3_CLK_SEL_M      0x000000C0
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_SAI3_CLK_SEL_S      6
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_SAI2_CLK_SEL_M      0x00000030
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_SAI2_CLK_SEL_S      4
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_SAI1_CLK_SEL_M      0x0000000C
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_SAI1_CLK_SEL_S      2
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_SAI0_CLK_SEL_M      0x00000003
#define CYGHWR_HAL_VYBRID_CCM_CSCMR1_SAI0_CLK_SEL_S      0

//---------------------------------------------------------------------------
// FTM - Flex Timer Module 
typedef volatile struct cyghwr_hal_vybrid_ftm_chnl_s {
	cyg_uint32 sc;
	cyg_uint32 v;	
}cyghwr_hal_vybrid_ftm_chnl_t;

	
typedef volatile struct cyghwr_hal_vybrid_ftm_s {
	cyg_uint32 sc;
	cyg_uint32 cnt;
	cyg_uint32 mod;
	cyghwr_hal_vybrid_ftm_chnl_t c[8];	
	cyg_uint32 cntin;
	cyg_uint32 status;
	cyg_uint32 mode;
	cyg_uint32 sync;
	cyg_uint32 outinit;
	cyg_uint32 outmask;
	cyg_uint32 combine;
	cyg_uint32 deadtime;
	cyg_uint32 exttrig;
	cyg_uint32 pol;
	cyg_uint32 fms;
	cyg_uint32 filter;
	cyg_uint32 fltctrl;
	cyg_uint32 qdctrl;
	cyg_uint32 conf;
	cyg_uint32 fltpol;
	cyg_uint32 synconf;
	cyg_uint32 invctrl;
	cyg_uint32 swoctrl;
	cyg_uint32 pwmload;	
} cyghwr_hal_vybrid_ftm_t;


#define CYGHWR_HAL_VYBRID_FTM0_P ((cyghwr_hal_vybrid_ftm_t *)0x40038000)
#define CYGHWR_HAL_VYBRID_FTM1_P ((cyghwr_hal_vybrid_ftm_t *)0x40039000)
#define CYGHWR_HAL_VYBRID_FTM2_P ((cyghwr_hal_vybrid_ftm_t *)0x400b8000)
#define CYGHWR_HAL_VYBRID_FTM3_P ((cyghwr_hal_vybrid_ftm_t *)0x400b9000)


//---------------------------------------------------------------------------
// ANADIG - Analog components control digital interface
// ANADIG - Peripheral register structure

// The ANADIG module has a very wide memory layout.
// Between each register is a 12 bytes free space. Therefore typical implementation
// of this module as structure is wasteful.

#define CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR              0x40050000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL              (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x270)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_SS                (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x280)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_NUM               (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x290)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_DENOM             (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x2a0)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD               (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x2b0)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL              (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x30)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_SS                (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x40)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_NUM               (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x50)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_DENOM             (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x60)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD               (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x100)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL              (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x10)
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD               (CYGHWR_HAL_VYBRID_ANADIG_BASE_ADDR + 0x60)

// PLL1_CTRL Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_LOCK_M           0x80000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_LOCK_S           31
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_PFD_OFFSET_EN_M  0x00040000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_PFD_OFFSET_EN_S  18
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_DITHER_ENABLE_M  0x00020000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_DITHER_ENABLE_S  17
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_BYPASS_M         0x00010000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_BYPASS_S         16
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_BYPASS_CLK_SRC_M 0x00004000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_BYPASS_CLK_SRC_S 14
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_ENABLE_M         0x00002000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_ENABLE_S         13
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_POWERDOWN_M      0x00001000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_POWERDOWN_S      12
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_DIV_SELECT_M     0x00000002
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_DIV_SELECT_S     1

// PLL1_NUM Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_NUM_M                 0x7FFFFFFF

// PLL1_DENOM Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_DENOM_M               0x7FFFFFFF

// PLL2_PFD Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD4_CLKGATE_M    0x80000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD4_CLKGATE_S    31
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD4_STABLE_M     0x40000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD4_STABLE_S     30
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD4_FRAC_M       0x3F000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD4_FRAC_S       24
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD3_CLKGATE_M    0x00800000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD3_CLKGATE_S    23
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD3_STABLE_M     0x00400000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD3_STABLE_S     22
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD3_FRAC_M       0x003F0000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD3_FRAC_S       16
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD2_CLKGATE_M    0x00008000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD2_CLKGATE_S    15
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD2_STABLE_M     0x00004000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD2_STABLE_S     14
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD2_FRAC_M       0x00003F00
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD2_FRAC_S       8
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD1_CLKGATE_M    0x00000080
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD1_CLKGATE_S    7
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD1_STABLE_M     0x00000040
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD1_STABLE_S     6
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD1_FRAC_M       0x0000003F
#define CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD_PFD1_FRAC_S       0

// PLL2_CTRL Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_LOCK_M           0x80000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_LOCK_S           31
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_PFD_OFFSET_EN_M  0x00040000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_PFD_OFFSET_EN_S  18
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DITHER_ENABLE_M  0x00020000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DITHER_ENABLE_S  17
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_BYPASS_M         0x00010000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_BYPASS_S         16
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_BYPASS_CLK_SRC_M 0x00004000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_BYPASS_CLK_SRC_S 14
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_ENABLE_M         0x00002000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_ENABLE_S         13
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_POWERDOWN_M      0x00001000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_POWERDOWN_S      12
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DIV_SELECT_M     0x00000002
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DIV_SELECT_S     1

// PLL2_NUM Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_NUM_M                 0x7FFFFFFF

// PLL2_DENOM Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_DENOM_M               0x7FFFFFFF

// PLL2_PFD Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD4_CLKGATE_M    0x80000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD4_CLKGATE_S    31
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD4_STABLE_M     0x40000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD4_STABLE_S     30
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD4_FRAC_M       0x3F000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD4_FRAC_S       24
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD3_CLKGATE_M    0x00800000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD3_CLKGATE_S    23
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD3_STABLE_M     0x00400000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD3_STABLE_S     22
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD3_FRAC_M       0x003F0000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD3_FRAC_S       16
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD2_CLKGATE_M    0x00008000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD2_CLKGATE_S    15
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD2_STABLE_M     0x00004000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD2_STABLE_S     14
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD2_FRAC_M       0x00003F00
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD2_FRAC_S       8
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD1_CLKGATE_M    0x00000080
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD1_CLKGATE_S    7
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD1_STABLE_M     0x00000040
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD1_STABLE_S     6
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD1_FRAC_M       0x0000003F
#define CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD_PFD1_FRAC_S       0

// PLL3_CTRL Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_LOCK_M           0x80000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_LOCK_S           31
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_BYPASS_M         0x00010000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_BYPASS_S         16
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_BYPASS_CLK_SRC_M 0x00004000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_BYPASS_CLK_SRC_S 14
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_ENABLE_M         0x00002000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_ENABLE_S         13
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_POWER_M          0x00001000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_POWER_S          12
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_EN_USB_CLKS_M    0x00000040
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_EN_USB_CLKS_S    6
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_DIV_SELECT_M     0x00000002
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_DIV_SELECT_S     1

// PLL3_PFD Bit Fields
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD4_CLKGATE_M    0x80000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD4_CLKGATE_S    31
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD4_STABLE_M     0x40000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD4_STABLE_S     30
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD4_FRAC_M       0x3F000000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD4_FRAC_S       24
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD3_CLKGATE_M    0x00800000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD3_CLKGATE_S    23
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD3_STABLE_M     0x00400000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD3_STABLE_S     22
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD3_FRAC_M       0x003F0000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD3_FRAC_S       16
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD2_CLKGATE_M    0x00008000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD2_CLKGATE_S    15
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD2_STABLE_M     0x00004000
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD2_STABLE_S     14
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD2_FRAC_M       0x00003F00
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD2_FRAC_S       8
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD1_CLKGATE_M    0x00000080
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD1_CLKGATE_S    7
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD1_STABLE_M     0x00000040
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD1_STABLE_S     6
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD1_FRAC_M       0x0000003F
#define CYGHWR_HAL_VYBRID_ANADIG_PLL3_PFD_PFD1_FRAC_S       0

//---------------------------------------------------------------------------
// Clock distribution
// The following encodes the control register and clock bit number
// into clock configuration descriptor (CLKCD).
#define CYGHWR_HAL_VYBRID_CCM_CCGR(__reg,__cg) (((__reg) & 0x0F) + ((((__cg - (__reg * 16)) * 2) << 8) & 0x1F00))

// Macros to extract encoded values.
#define CYGHWR_HAL_VYBRID_CCM_CCGR_REG(__val) (((__val) & 0x0F))
#define CYGHWR_HAL_VYBRID_CCM_CCGR_BIT(__val) (((__val) >> 8) & 0x1F)

// Functions and macros to enable/disable clocks.
#define CYGHWR_HAL_CCGR_NONE (0xFFFFFFFF)
__externC void hal_clock_enable(cyg_uint32 ccgr);
__externC void hal_clock_disable(cyg_uint32 ccgr);

#define CYGHWR_HAL_CLOCK_ENABLE(__ccgr) hal_clock_enable(__ccgr)
#define CYGHWR_HAL_CLOCK_DISABLE(__ccgr) hal_clock_disable(__ccgr)
#include <cyg/hal/var_io_clkgat.h>


//---------------------------------------------------------------------------
// PORT - Peripheral register structure

typedef volatile struct cyghwr_hal_vybrid_port_s {
    cyg_uint32 pcr[32];      // Pin Control Register n, array
    cyg_uint8 reserved0[24];
    cyg_uint32 isfr;         // Interrupt Status Flag Register
    cyg_uint8 reserved1[28];
    cyg_uint32 dfer;         // Digital Filter Enable Register
    cyg_uint32 dfcr;         // Digital Filter Clock Register
    cyg_uint32 dfwr;         // Digital Filter Width Register
} cyghwr_hal_vybrid_port_t;


// PORT - Peripheral instance base addresses
#define CYGHWR_HAL_VYBRID_PORTA_P  ((cyghwr_hal_vybrid_port_t *)0x40049000)
#define CYGHWR_HAL_VYBRID_PORTB_P  ((cyghwr_hal_vybrid_port_t *)0x4004A000)
#define CYGHWR_HAL_VYBRID_PORTC_P  ((cyghwr_hal_vybrid_port_t *)0x4004B000)
#define CYGHWR_HAL_VYBRID_PORTD_P  ((cyghwr_hal_vybrid_port_t *)0x4004C000)
#define CYGHWR_HAL_VYBRID_PORTE_P  ((cyghwr_hal_vybrid_port_t *)0x4004D000)
#define CYGHWR_HAL_VYBRID_PORTF_P  ((cyghwr_hal_vybrid_port_t *)0x4004E000)

enum {
    CYGHWR_HAL_VYBRID_PORTA, CYGHWR_HAL_VYBRID_PORTB,
    CYGHWR_HAL_VYBRID_PORTC, CYGHWR_HAL_VYBRID_PORTD,
    CYGHWR_HAL_VYBRID_PORTE, CYGHWR_HAL_VYBRID_PORTF
};

#define CYGHWR_HAL_VYBRID_PORT(__port, __reg) \
        (CYGHWR_HAL_VYBRID_PORT##__port##_P)->__reg

// PCR Bit Fields TODO To remove
#define CYGHWR_HAL_VYBRID_PORT_PCR_PS_M          0x1
#define CYGHWR_HAL_VYBRID_PORT_PCR_PS_S          0
#define CYGHWR_HAL_VYBRID_PORT_PCR_PE_M          0x2
#define CYGHWR_HAL_VYBRID_PORT_PCR_PE_S          1
#define CYGHWR_HAL_VYBRID_PORT_PCR_SRE_M         0x4
#define CYGHWR_HAL_VYBRID_PORT_PCR_SRE_S         2
#define CYGHWR_HAL_VYBRID_PORT_PCR_PFE_M         0x10
#define CYGHWR_HAL_VYBRID_PORT_PCR_PFE_S         4
#define CYGHWR_HAL_VYBRID_PORT_PCR_ODE_M         0x20
#define CYGHWR_HAL_VYBRID_PORT_PCR_ODE_S         5
#define CYGHWR_HAL_VYBRID_PORT_PCR_DSE_M         0x40
#define CYGHWR_HAL_VYBRID_PORT_PCR_DSE_S         6
#define CYGHWR_HAL_VYBRID_PORT_PCR_MUX_M         0x700
#define CYGHWR_HAL_VYBRID_PORT_PCR_MUX_S         8	// VYBRID
#define CYGHWR_HAL_VYBRID_PORT_PCR_MUX(__val)    \
        VALUE_(CYGHWR_HAL_VYBRID_PORT_PCR_MUX_S, __val)
#define CYGHWR_HAL_VYBRID_PORT_PCR_LK_M          0x8000
#define CYGHWR_HAL_VYBRID_PORT_PCR_LK_S          15
#define CYGHWR_HAL_VYBRID_PORT_PCR_IRQC_M        0xF0000
#define CYGHWR_HAL_VYBRID_PORT_PCR_IRQC_S        16
#define CYGHWR_HAL_VYBRID_PORT_PCR_IRQC(__val)   \
        VALUE_(CYGHWR_HAL_VYBRID_PORT_PCR_IRQC_S, __val)
#define CYGHWR_HAL_VYBRID_PORT_PCR_ISF_M         0x1000000
#define CYGHWR_HAL_VYBRID_PORT_PCR_ISF_S         24

#define CYGHWR_HAL_VYBRID_PORT_PCR_MUX_ANALOG    0
#define CYGHWR_HAL_VYBRID_PORT_PCR_MUX_DIS       0
#define CYGHWR_HAL_VYBRID_PORT_PCR_MUX_GPIO      1

#define CYGHWR_HAL_VYBRID_PORT_PCR_ISFR_CLEAR(__port, __pin) \
        CYGHWR_HAL_VYBRID_PORT(__port, pcr[__pin]) |= BIT_(24)

#define CYGHWR_HAL_VYBRID_PORT_ISFR_CLEAR(__port, __pin)     \
        CYGHWR_HAL_VYBRID_PORT(__port, isfr) |= BIT_(__pin)



#define CYGHWR_HAL_VYBRID_PIN_CFG(__port, __bit, __mux, __irqc, __cnf) \
        ((CYGHWR_HAL_VYBRID_PORT##__port << 20) | ((__bit) << 27)      \
         | CYGHWR_HAL_VYBRID_PORT_PCR_IRQC(__irqc)                     \
         | CYGHWR_HAL_VYBRID_PORT_PCR_MUX(__mux) | (__cnf))

#define CYGHWR_HAL_VYBRID_PIN(__port, __bit, __mux, __cnf) \
        CYGHWR_HAL_VYBRID_PIN_CFG(__port, __bit, __mux, 0, __cnf)

#define CYGHWR_HAL_VYBRID_PIN_PORT(__pin) (((__pin) >> 20) & 0x7)
#define CYGHWR_HAL_VYBRID_PIN_BIT(__pin)  (((__pin) >> 27 ) & 0x1f)
#define CYGHWR_HAL_VYBRID_PIN_FUNC(__pin) ((__pin) & 0x010f8777)
#define CYGHWR_HAL_VYBRID_PIN_NONE (0xffffffff)

// GPCLR Bit Fields
#define CYGHWR_HAL_VYBRID_PORT_GPCLR_GPWD_M      0xFFFF
#define CYGHWR_HAL_VYBRID_PORT_GPCLR_GPWD_S      0
#define CYGHWR_HAL_VYBRID_PORT_GPCLR_GPWD(__val) \
        VALUE_(CYGHWR_HAL_VYBRID_PORT_GPCLR_GPWD_S, __val)
#define CYGHWR_HAL_VYBRID_PORT_GPCLR_GPWE_M      0xFFFF0000
#define CYGHWR_HAL_VYBRID_PORT_GPCLR_GPWE_S      16
#define CYGHWR_HAL_VYBRID_PORT_GPCLR_GPWE(__val) \
        VALUE_(CYGHWR_HAL_VYBRID_PORT_GPCLR_GPWE_S, __val)
// GPCHR Bit Fields
#define CYGHWR_HAL_VYBRID_PORT_GPCHR_GPWD_M      0xFFFF
#define CYGHWR_HAL_VYBRID_PORT_GPCHR_GPWD_S      0
#define CYGHWR_HAL_VYBRID_PORT_GPCHR_GPWD(__val) \
        VALUE_(CYGHWR_HAL_VYBRID_PORT_GPCHR_GPWD_S, __val)
#define CYGHWR_HAL_VYBRID_PORT_GPCHR_GPWE_M      0xFFFF0000
#define CYGHWR_HAL_VYBRID_PORT_GPCHR_GPWE_S      16
#define CYGHWR_HAL_VYBRID_PORT_GPCHR_GPWE(__val) \
        VALUE_(CYGHWR_HAL_VYBRID_PORT_GPCHR_GPWE_S, __val)
// ISFR Bit Fields
#define CYGHWR_HAL_VYBRID_PORT_ISFR_ISF_M        0xFFFFFFFF
#define CYGHWR_HAL_VYBRID_PORT_ISFR_ISF_S        0
#define CYGHWR_HAL_VYBRID_PORT_ISFR_ISF(__val)   \
        VALUE_(CYGHWR_HAL_VYBRID_PORT_ISFR_ISF_S, __val)
// DFER Bit Fields
#define CYGHWR_HAL_VYBRID_PORT_DFER_DFE_M        0xFFFFFFFF
#define CYGHWR_HAL_VYBRID_PORT_DFER_DFE_S        0
#define CYGHWR_HAL_VYBRID_PORT_DFER_DFE(__val)   \
        VALUE_(CYGHWR_HAL_VYBRID_PORT_DFER_DFE_S, __val)
// DFCR Bit Fields
#define CYGHWR_HAL_VYBRID_PORT_DFCR_CS_M         0x1
#define CYGHWR_HAL_VYBRID_PORT_DFCR_CS_S         0
// DFWR Bit Fields
#define CYGHWR_HAL_VYBRID_PORT_DFWR_FILT_M       0x1F
#define CYGHWR_HAL_VYBRID_PORT_DFWR_FILT_S       0
#define CYGHWR_HAL_VYBRID_PORT_DFWR_FILT(__val)  \
        VALUE_(CYGHWR_HAL_VYBRID_PORT_DFWR_FILT_S, __val)

#ifndef __ASSEMBLER__

// Pin configuration related functions
__externC void  hal_set_pin_function(cyg_uint32 pin);
__externC void  hal_dump_pin_function(cyg_uint32 pin);
__externC void  hal_dump_pin_setting(cyg_uint32 pin);
__externC void  hal_gpio_set_pin(cyg_uint32 pin);
__externC void  hal_gpio_clear_pin(cyg_uint32 pin);
__externC void  hal_gpio_toggle_pin(cyg_uint32 pin);
__externC cyg_uint32 hal_gpio_get_pin(cyg_uint32 pin);

#endif

#define HAL_SET_PINS(_pin_array) \
CYG_MACRO_START \
    const cyg_uint32 *_pin_p; \
    for(_pin_p = &_pin_array[0]; \
        _pin_p < &_pin_array[0] + sizeof(_pin_array)/sizeof(_pin_array[0]); \
        hal_set_pin_function(*_pin_p++)); \
CYG_MACRO_END

//---------------------------------------------------------------------------
// PMC Power Management Controller

typedef volatile struct cyghwr_hal_vybrid_pmc_s {
    cyg_uint8 lvdsc1;   // Low Voltage Detect Status and Control 1 Register
    cyg_uint8 lvdsc2;   // Low Voltage Detect Status and Control 2 Register
    cyg_uint8 regsc;    // Regulator Status and Control Register
} cyghwr_hal_vybrid_pmc_t;

// PMC base address
#define CYGHWR_HAL_VYBRID_PMC_P  ((cyghwr_hal_vybrid_pmc_t *)0x4007D000)

// LVDSC1 Bit Fields
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDV_M      0x3
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDV(__val) \
        ((__val) & CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDV_M)
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDRE_M     0x10
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDRE_S     4
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDIE_M     0x20
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDIE_S     5
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDACK_M    0x40
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDACK_S    6
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDF_M      0x80
#define CYGHWR_HAL_VYBRID_PMC_LVDSC1_LVDF_S      7
// LVDSC2 Bit Fields
#define CYGHWR_HAL_VYBRID_PMC_LVDSC2_LVWV_M      0x3
#define CYGHWR_HAL_VYBRID_PMC_LVDSC2_LVWV(__val) \
        ((__val) & CYGHWR_HAL_VYBRID_PMC_LVDSC2_LVWV_M)
#define CYGHWR_HAL_VYBRID_PMC_LVDSC2_LVWIE_M     0x20
#define CYGHWR_HAL_VYBRID_PMC_LVDSC2_LVWIE_S     5
#define CYGHWR_HAL_VYBRID_PMC_LVDSC2_LVWACK_M    0x40
#define CYGHWR_HAL_VYBRID_PMC_LVDSC2_LVWACK_S    6
#define CYGHWR_HAL_VYBRID_PMC_LVDSC2_LVWF_M      0x80
#define CYGHWR_HAL_VYBRID_PMC_LVDSC2_LVWF_S      7
// REGSC Bit Fields
#define CYGHWR_HAL_VYBRID_PMC_REGSC_BGBE_M       0x1
#define CYGHWR_HAL_VYBRID_PMC_REGSC_BGBE_S       0
#define CYGHWR_HAL_VYBRID_PMC_REGSC_REGONS_M     0x4
#define CYGHWR_HAL_VYBRID_PMC_REGSC_REGONS_S     2
#define CYGHWR_HAL_VYBRID_PMC_REGSC_ACKISO_M     0x8
#define CYGHWR_HAL_VYBRID_PMC_REGSC_ACKISO_S     3
#define CYGHWR_HAL_VYBRID_PMC_REGSC_BGEN_M       0x10
#define CYGHWR_HAL_VYBRID_PMC_REGSC_BGEN_S       4


//---------------------------------------------------------------------------
// FlexBus
#ifdef CYGPKG_HAL_CORTEXM_VYBRID_FLEXBUS
# include <cyg/hal/var_io_flexbus.h>
#endif

//----------------------------------------------------------------------------
// DDRMC - SDRAM controller
#ifdef CYGPKG_HAL_CORTEXM_VYBRID_DDRMC
# include <cyg/hal/var_io_ddrmc.h>
#endif

//---------------------------------------------------------------------------
// GPIO
#include <cyg/hal/var_io_gpio.h>

//=============================================================================
// DEVS:
// Following macros may also be, and usually are borrowed by some device drivers.
//-----------------------------------------------------------------------------
#include <cyg/hal/var_io_devs.h>

// End Peripherals

// Some miscelaneous function prototypes
// Clock related functions are in vybrid_clocking.c
__externC void hal_start_clocks(void);
__externC void hal_update_clock_var(void);

//-----------------------------------------------------------------------------
// end of var_io.h

#endif // CYGONCE_HAL_VAR_IO_H

