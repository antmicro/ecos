#ifndef CYGONCE_HAL_VAR_INTR_H
#define CYGONCE_HAL_VAR_INTR_H
//==========================================================================
//
//      var_intr.h
//
//      HAL Interrupt and clock assignments for Vybrid variants
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2011 Free Software Foundation, Inc.                        
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
// Author(s):    Antmicro Ltd <contact@antmicro.com>
// Based on:	 {...}/hal/packages/cortexm/kinetis/var/current/include/var_intr.h
// Date:         2014-03-28
// Purpose:      Define Interrupt support
// Description:  The interrupt specifics for Freescale Vybrid variants are
//               defined here.
//
// Usage:        #include <cyg/hal/var_intr.h>
//               However applications should include using <cyg/hal/hal_intr.h>
//               instead to allow for platform overrides.
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <cyg/hal/plf_intr.h>

//==========================================================================

typedef enum {
    CYGNUM_HAL_INTERRUPT_CPU2CPU_0
    	= CYGNUM_HAL_INTERRUPT_EXTERNAL,  // CPU to CPU int 0
    CYGNUM_HAL_INTERRUPT_CPU2CPU_1,       // CPU to CPU int 1
    CYGNUM_HAL_INTERRUPT_CPU2CPU_2,       // CPU to CPU int 2
    CYGNUM_HAL_INTERRUPT_CPU2CPU_3,       // CPU to CPU int 3
    CYGNUM_HAL_INTERRUPT_SEMA4,           // Directed Cortex-M4(= SEMA4)
    CYGNUM_HAL_INTERRUPT_MCM,             // Directed Cortex-M4(= MCM)
    CYGNUM_HAL_INTERRUPT_DIRECTED1,       // Directed Cortex-M4
    CYGNUM_HAL_INTERRUPT_DIRECTED2,       // Directed Cortex-M4
    CYGNUM_HAL_INTERRUPT_DMA0,            // DMA Channel 0 Transfer Complete
    CYGNUM_HAL_INTERRUPT_DMA0_ERROR,      // DMA Channel 0 Error int
    CYGNUM_HAL_INTERRUPT_DMA1,            // DMA Channel 1 Transfer Complete
    CYGNUM_HAL_INTERRUPT_DMA1_ERROR,      // DMA Channel 1 Error int
    CYGNUM_HAL_INTERRUPT_RESERVED_00,
    CYGNUM_HAL_INTERRUPT_RESERVED_01,
    CYGNUM_HAL_INTERRUPT_MSCM_ECC0,       // Error Correction and Control 0
    CYGNUM_HAL_INTERRUPT_MSCM_ECC1,       // Error Correction and Control 1
    CYGNUM_HAL_INTERRUPT_CSU_ALARM,       // CSU interrupt
    CYGNUM_HAL_INTERRUPT_RESERVED_02,
    CYGNUM_HAL_INTERRUPT_MSCM_ACTZS,      // Miscellaneous System Control Module - TrustZone Security
    CYGNUM_HAL_INTERRUPT_RESERVED_03,
    CYGNUM_HAL_INTERRUPT_WDOG_A5,         // WDOG A5 int
    CYGNUM_HAL_INTERRUPT_WDOG_M4,         // WDOG M4 int
    CYGNUM_HAL_INTERRUPT_WDOG_SNVS,       // TrustZone Watchdog
    CYGNUM_HAL_INTERRUPT_CP1,             // CP1 Boot Fail
    CYGNUM_HAL_INTERRUPT_QSPI0,           // QuadSPI0 int
    CYGNUM_HAL_INTERRUPT_QSPI1,           // QuadSPI1 int
    CYGNUM_HAL_INTERRUPT_DDRMC,           // SDRAM Memory Controller
    CYGNUM_HAL_INTERRUPT_SDHC0,           // Secure Digital Host Controller 0
    CYGNUM_HAL_INTERRUPT_SDHC1,           // Secure Digital Host Controller 1
    CYGNUM_HAL_INTERRUPT_RESERVED_04,
    CYGNUM_HAL_INTERRUPT_DCU0,            // Display Control Unit 0
    CYGNUM_HAL_INTERRUPT_DCU1,            // Display Control Unit 1
    CYGNUM_HAL_INTERRUPT_VIU,             // Video In Unit 
    CYGNUM_HAL_INTERRUPT_RESERVED_05,
    CYGNUM_HAL_INTERRUPT_RESERVED_06,
    CYGNUM_HAL_INTERRUPT_RLE,             // Run Length Encoding (Decoder unit)
    CYGNUM_HAL_INTERRUPT_SEG_LCD,         // LCD Controller
    CYGNUM_HAL_INTERRUPT_RESERVED_07,
    CYGNUM_HAL_INTERRUPT_RESERVED_08,
    CYGNUM_HAL_INTERRUPT_PIT,             // Periodic interrupt timer
    CYGNUM_HAL_INTERRUPT_LPT0,            // LPTimer0
    CYGNUM_HAL_INTERRUPT_RESERVED_09,
    CYGNUM_HAL_INTERRUPT_FLXT0,           // FlexTimer 0
    CYGNUM_HAL_INTERRUPT_FLXT1,           // FlexTimer 1
    CYGNUM_HAL_INTERRUPT_FLXT2,           // FlexTimer 2
    CYGNUM_HAL_INTERRUPT_FLXT3,           // FlexTimer 3
    CYGNUM_HAL_INTERRUPT_RESERVED_10,
    CYGNUM_HAL_INTERRUPT_RESERVED_11,
    CYGNUM_HAL_INTERRUPT_RESERVED_12,
    CYGNUM_HAL_INTERRUPT_RESERVED_13,
    CYGNUM_HAL_INTERRUPT_USBPHY0,         // USB PHY 0
    CYGNUM_HAL_INTERRUPT_USBPHY1,         // USB PHY 1
    CYGNUM_HAL_INTERRUPT_RESERVED_14,
    CYGNUM_HAL_INTERRUPT_ADC0,            // AD Converter 0
    CYGNUM_HAL_INTERRUPT_ADC1,            // AD Converter 1
    CYGNUM_HAL_INTERRUPT_DAC0,            // DA Converter 0
    CYGNUM_HAL_INTERRUPT_DAC1,            // DA Converter 1
    CYGNUM_HAL_INTERRUPT_RESERVED_15,
    CYGNUM_HAL_INTERRUPT_FLXCAN0,         // FlexCAN 0
    CYGNUM_HAL_INTERRUPT_FLXCAN1,         // FlexCAN 1
    CYGNUM_HAL_INTERRUPT_RESERVED_16,
    CYGNUM_HAL_INTERRUPT_UART0_RX_TX,     // UART0 Controller
    CYGNUM_HAL_INTERRUPT_UART1_RX_TX,     // UART1 Controller
    CYGNUM_HAL_INTERRUPT_UART2_RX_TX,     // UART2 Controller
    CYGNUM_HAL_INTERRUPT_UART3_RX_TX,     // UART3 Controller
    CYGNUM_HAL_INTERRUPT_UART4_RX_TX,     // UART4 Controller
    CYGNUM_HAL_INTERRUPT_UART5_RX_TX,     // UART5 Controller
    CYGNUM_HAL_INTERRUPT_SPI0,            // SPI0
    CYGNUM_HAL_INTERRUPT_SPI1,            // SPI1
    CYGNUM_HAL_INTERRUPT_SPI2,            // SPI2
    CYGNUM_HAL_INTERRUPT_SPI3,            // SPI3
    CYGNUM_HAL_INTERRUPT_I2C0,            // I2C0
    CYGNUM_HAL_INTERRUPT_I2C1,            // I2C1
    CYGNUM_HAL_INTERRUPT_I2C2,            // I2C2
    CYGNUM_HAL_INTERRUPT_I2C3,            // I2C3
    CYGNUM_HAL_INTERRUPT_USBC0,           // USB 0 Controller
    CYGNUM_HAL_INTERRUPT_USBC1,           // USB 1 Controller
    CYGNUM_HAL_INTERRUPT_RESERVED_17,
    CYGNUM_HAL_INTERRUPT_ENET0,           // Ethernet MAC 0
    CYGNUM_HAL_INTERRUPT_ENET1,           // Ethernet MAC 1
    CYGNUM_HAL_INTERRUPT_1588_0,          // IEEE 1588 T0
    CYGNUM_HAL_INTERRUPT_1588_1,          // IEEE 1588 T1
    CYGNUM_HAL_INTERRUPT_ENET_SWI,        // Ethernet L2 swich
    CYGNUM_HAL_INTERRUPT_NFC,             // Nand Flash Controller
    CYGNUM_HAL_INTERRUPT_SAI0,            // Synchronous Audio Interface 0
    CYGNUM_HAL_INTERRUPT_SAI1,            // Synchronous Audio Interface 1
    CYGNUM_HAL_INTERRUPT_SAI2,            // Synchronous Audio Interface 2
    CYGNUM_HAL_INTERRUPT_SAI3,            // Synchronous Audio Interface 3
    CYGNUM_HAL_INTERRUPT_ESAI_BIFIFO,     // Enhanced Serial Audio Interface Bus Interface and FIFO
    CYGNUM_HAL_INTERRUPT_SPDIF,           // Sony/Philips Digital Interface
    CYGNUM_HAL_INTERRUPT_ASRC,            // Audio Sample Rate Converter
    CYGNUM_HAL_INTERRUPT_VREG,            // HVD Int
    CYGNUM_HAL_INTERRUPT_WKPU0,           // Wake Up 0
    CYGNUM_HAL_INTERRUPT_RESERVED_18,
    CYGNUM_HAL_INTERRUPT_CCM,             // FXOSC ready int
    CYGNUM_HAL_INTERRUPT_CCM_2,           // Logical OR of LRF of PLL1, PLL2, PLL3, PLL4
    CYGNUM_HAL_INTERRUPT_SRC,             // System Reset Controller
    CYGNUM_HAL_INTERRUPT_PDB,             // Programmable Delay Block
    CYGNUM_HAL_INTERRUPT_EWM,             // External Watchdog Monitor
    CYGNUM_HAL_INTERRUPT_RESERVED_19,
    CYGNUM_HAL_INTERRUPT_RESERVED_20,
    CYGNUM_HAL_INTERRUPT_RESERVED_21,
    CYGNUM_HAL_INTERRUPT_RESERVED_22,
    CYGNUM_HAL_INTERRUPT_RESERVED_23,
    CYGNUM_HAL_INTERRUPT_RESERVED_24,
    CYGNUM_HAL_INTERRUPT_RESERVED_25,
    CYGNUM_HAL_INTERRUPT_RESERVED_26,
    CYGNUM_HAL_INTERRUPT_GPIO0,           // GPIO PORT0 interrupts/ Wake-ups
    CYGNUM_HAL_INTERRUPT_GPIO1,           // GPIO PORT1 interrupts/ Wake-ups
    CYGNUM_HAL_INTERRUPT_GPIO2,           // GPIO PORT2 interrupts/ Wake-ups
    CYGNUM_HAL_INTERRUPT_GPIO3,           // GPIO PORT3 interrupts/ Wake-ups
    CYGNUM_HAL_INTERRUPT_GPIO4            // GPIO PORT4 interrupts/ Wake-ups
} VybridExtInterrupt_e;


// Ranges of usable interrupt sources
#define CYGNUM_HAL_INTERRUPT_NVIC_MAX	CYGNUM_HAL_INTERRUPT_GPIO4
#define CYGNUM_HAL_ISR_MIN            0
#define CYGNUM_HAL_ISR_MAX		 CYGNUM_HAL_INTERRUPT_GPIO4
#define CYGNUM_HAL_ISR_COUNT          (CYGNUM_HAL_ISR_MAX + 1)

#define CYGNUM_HAL_VSR_MIN            0
#ifndef CYGNUM_HAL_VSR_MAX
# define CYGNUM_HAL_VSR_MAX           (CYGNUM_HAL_VECTOR_SYS_TICK+ \
                                       CYGNUM_HAL_INTERRUPT_NVIC_MAX)
#endif

#define CYGNUM_HAL_VSR_COUNT          (CYGNUM_HAL_VSR_MAX+1)

//==========================================================================
// Interrupt mask and config for variant-specific devices

// PORT Pin interrupts

#define CYGHWR_HAL_VYBRID_PIN_IRQ_VECTOR(__pin) \
    (CYGNUM_HAL_INTERRUPT_PORTA + CYGHWR_HAL_VYBRID_PIN_PORT(__pin))

//===========================================================================
// Interrupt resources exported by HAL to device drivers

// Export Interrupt vectors to serial driver.

#define CYGNUM_IO_SERIAL_FREESCALE_UART0_INT_VECTOR \
            CYGNUM_HAL_INTERRUPT_UART0_RX_TX
#define CYGNUM_IO_SERIAL_FREESCALE_UART1_INT_VECTOR \
            CYGNUM_HAL_INTERRUPT_UART1_RX_TX
#define CYGNUM_IO_SERIAL_FREESCALE_UART2_INT_VECTOR \
            CYGNUM_HAL_INTERRUPT_UART2_RX_TX
#define CYGNUM_IO_SERIAL_FREESCALE_UART3_INT_VECTOR \
            CYGNUM_HAL_INTERRUPT_UART3_RX_TX
#define CYGNUM_IO_SERIAL_FREESCALE_UART4_INT_VECTOR \
            CYGNUM_HAL_INTERRUPT_UART4_RX_TX
#define CYGNUM_IO_SERIAL_FREESCALE_UART5_INT_VECTOR \
            CYGNUM_HAL_INTERRUPT_UART5_RX_TX

// Export Interrupt vectors to ENET driver.

#define CYGNUM_FREESCALE_ENET0_1588_TIMER_INT_VECTOR \
            CYGNUM_HAL_INTERRUPT_ENET_1588_TIMER
#define CYGNUM_FREESCALE_ENET0_TRANSMIT_INT_VECTOR   \
            CYGNUM_HAL_INTERRUPT_ENET_TRANSMIT
#define CYGNUM_FREESCALE_ENET0_RECEIVE_INT_VECTOR    \
            CYGNUM_HAL_INTERRUPT_ENET_RECEIVE
#define CYGNUM_FREESCALE_ENET0_ERROR_INT_VECTOR      \
            CYGNUM_HAL_INTERRUPT_ENET_ERROR

#define HAL_VAR_INTERRUPT_CONFIGURE( __vector, __level, __up )  CYG_EMPTY_STATEMENT
#define HAL_VAR_INTERRUPT_ACKNOWLEDGE( __vector ) CYG_EMPTY_STATEMENT

#define HAL_VAR_INTERRUPT_SET_LEVEL( __vector, __level ) 				\
{																		\
    cyg_uint16 reg;                                                     \
	if(__vector > 0 )													\
	{																	\
        HAL_READ_UINT16((CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_BASE + ((__vector - 1 )*2)), reg);\
        reg |= (1 << CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_CP1En_S);                                 \
		HAL_WRITE_UINT16((CYGHWR_HAL_VYBRID_MSCM_IRSPRCn_BASE + ((__vector - 1 )*2)), reg);\
	}																	\
}

#define HAL_VAR_INTERRUPT_MASK( __vector ) 				\
{																		\
	HAL_WRITE_UINT32( CYGARC_REG_NVIC_BASE+CYGARC_REG_NVIC_CPR(__vector-CYGNUM_HAL_INTERRUPT_EXTERNAL),	\
	                          CYGARC_REG_NVIC_IBIT(__vector-CYGNUM_HAL_INTERRUPT_EXTERNAL) );				\
}

#define HAL_VAR_INTERRUPT_UNMASK( __vector ) 				\
{																		\
	HAL_WRITE_UINT32( CYGARC_REG_NVIC_BASE+CYGARC_REG_NVIC_CPR(__vector-CYGNUM_HAL_INTERRUPT_EXTERNAL),	\
	                          CYGARC_REG_NVIC_IBIT(__vector-CYGNUM_HAL_INTERRUPT_EXTERNAL) );				\
}
//----------------------------------------------------------------------------
#endif // CYGONCE_HAL_VAR_INTR_H
// EOF var_intr.h
