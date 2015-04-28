#ifndef CYGONCE_HAL_PLATFORM_INTS_H
#define CYGONCE_HAL_PLATFORM_INTS_H
//==========================================================================
//
//      hal_platform_ints.h
//
//      HAL Interrupt and clock assignments for AT91SAM7
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2006 Free Software Foundation, Inc.
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
// Author(s):    ITR-GmbH
// Contributors: 
// Date:         2012-06-25
// Purpose:      Define Interrupt support
// Description:  The interrupt specifics for the Xilinx XC7Z platform are
//               defined here.
//
// Usage:        #include <cyg/hal/hal_platform_ints.h>
//               ...
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal_arm_xc7z.h>

// CPU Private Peripheral Interrupts (PPI)
#define CYGNUM_HAL_INTERRUPT_GLOBALTMR          27  /* SCU Global Timer interrupt */
#define CYGNUM_HAL_INTERRUPT_FIQ                28  /* FIQ from FPGA fabric */
#define CYGNUM_HAL_INTERRUPT_SCU_CPUTMR         29  /* SCU Private Timer interrupt */
#define CYGNUM_HAL_INTERRUPT_SCU_WDTTMR         30  /* SCU Private WDT interrupt */
#define CYGNUM_HAL_INTERRUPT_IRQ                31  /* IRQ from FPGA fabric */ 


// Shared Peripheral Interrupts (SPI)
#define CYGNUM_HAL_INTERRUPT_CORE_PARITY0       32  /* L1 cache (Rising edge) */
#define CYGNUM_HAL_INTERRUPT_CORE_PARITY1       33  /* L1 cache (Rising edge) */
#define CYGNUM_HAL_INTERRUPT_L2CC               34  /* L2 cache (High level) */
#define CYGNUM_HAL_INTERRUPT_OCMINTR            35  /* OCM (High level) */
#define CYGNUM_HAL_INTERRUPT_ECC                36  /* ECC (High level) */
#define CYGNUM_HAL_INTERRUPT_PMU0               37  /* PMU0 (High level) */
#define CYGNUM_HAL_INTERRUPT_PMU1               38  /* PMU0 (High level) */
#define CYGNUM_HAL_INTERRUPT_SYSMON             39  /* XADC (High level) */
#define CYGNUM_HAL_INTERRUPT_DVC                40  /* DVI (High level) */
#define CYGNUM_HAL_INTERRUPT_WDT                41  /* SWDT (High level) */
#define CYGNUM_HAL_INTERRUPT_TTC0_0             42  /* TTC0_0 (High level) */
#define CYGNUM_HAL_INTERRUPT_TTC0_1             43  /* TTC0_1 (High level) */
#define CYGNUM_HAL_INTERRUPT_TTC0_2             44  /* TTC0_2 (High level) */
#define CYGNUM_HAL_INTERRUPT_DMA0_ABORT         45  /* DMAC Abort (High level) */
#define CYGNUM_HAL_INTERRUPT_DMA0               46  /* DMAC0 (High level) */
#define CYGNUM_HAL_INTERRUPT_DMA1               47  /* DMAC1 (High level) */
#define CYGNUM_HAL_INTERRUPT_DMA2               48  /* DMAC2 (High level) */
#define CYGNUM_HAL_INTERRUPT_DMA3               49  /* DMAC3 (High level) */
#define CYGNUM_HAL_INTERRUPT_SMC                50  /* SMC (High level) */
#define CYGNUM_HAL_INTERRUPT_QSPI               51  /* QSPI (High level) */
#define CYGNUM_HAL_INTERRUPT_GPIO               52  /* GPIO (High level) */
#define CYGNUM_HAL_INTERRUPT_USB0               53  /* USB0 (High level) */
#define CYGNUM_HAL_INTERRUPT_GEM0               54  /* Ethernet 0 (High level) */
#define CYGNUM_HAL_INTERRUPT_GEM0_WAKE          55  /* Ethernet 0 Wakeup (High level) */
#define CYGNUM_HAL_INTERRUPT_SDIO0              56  /* SDIO 0 (High level) */
#define CYGNUM_HAL_INTERRUPT_I2C0               57  /* I2C 0 (High level) */
#define CYGNUM_HAL_INTERRUPT_SPI0               58  /* SPI 0 (High level) */
#define CYGNUM_HAL_INTERRUPT_UART0              59  /* UART 0 (High level) */
#define CYGNUM_HAL_INTERRUPT_CAN0               60  /* CAN 0 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA0              61  /* FPGA 0 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA1              62  /* FPGA 1 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA2              63  /* FPGA 2 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA3              64  /* FPGA 3 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA4              65  /* FPGA 4 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA5              66  /* FPGA 5 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA6              67  /* FPGA 6 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA7              68  /* FPGA 7 (High level) */
#define CYGNUM_HAL_INTERRUPT_TTC1_0             69  /* TTC1_0 (High level) */
#define CYGNUM_HAL_INTERRUPT_TTC1_1             70  /* TTC1_1 (High level) */
#define CYGNUM_HAL_INTERRUPT_TTC1_2             71  /* TTC1_2 (High level) */
#define CYGNUM_HAL_INTERRUPT_DMA4               72  /* DMA4 (High level) */
#define CYGNUM_HAL_INTERRUPT_DMA5               73  /* DMA5 (High level) */
#define CYGNUM_HAL_INTERRUPT_DMA6               74  /* DMA6 (High level) */
#define CYGNUM_HAL_INTERRUPT_DMA7               75  /* DMA7 (High level) */
#define CYGNUM_HAL_INTERRUPT_USB1               76  /* USB1 (High level) */
#define CYGNUM_HAL_INTERRUPT_GEM1               77  /* Ethernet 1 (High level) */
#define CYGNUM_HAL_INTERRUPT_GEM1_WAKE          78  /* Ethernet 1 Wakeup (High level) */
#define CYGNUM_HAL_INTERRUPT_SDIO1              79  /* SDIO 1 (High level) */
#define CYGNUM_HAL_INTERRUPT_I2C1               80  /* I2C 1 (High level) */
#define CYGNUM_HAL_INTERRUPT_SPI1               81  /* SPI 1 (High level) */
#define CYGNUM_HAL_INTERRUPT_UART1              82  /* UART 1 (High level) */
#define CYGNUM_HAL_INTERRUPT_CAN1               83  /* CAN 1 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA8              84  /* FPGA 8 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA9              85  /* FPGA 9 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA10             86  /* FPGA 10 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA11             87  /* FPGA 11 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA12             88  /* FPGA 12 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA13             89  /* FPGA 13 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA14             90  /* FPGA 14 (High level) */
#define CYGNUM_HAL_INTERRUPT_FPGA15             91  /* FPGA 15 (High level) */
#define CYGNUM_HAL_INTERRUPT_SCU_PARITY         92  /* SCU Parity (High level) */

#define CYGNUM_HAL_ISR_MIN                      0
#define CYGNUM_HAL_ISR_MAX                      94

#define CYGNUM_HAL_ISR_COUNT                    (CYGNUM_HAL_ISR_MAX + 1)

#define CYGNUM_HAL_INTERRUPT_RTC                CYGNUM_HAL_INTERRUPT_SCU_CPUTMR

externC void hal_reset(void);
#define HAL_PLATFORM_RESET()                    hal_reset()

#define HAL_PLATFORM_RESET_ENTRY                0x0000000

#endif // CYGONCE_HAL_PLATFORM_INTS_H
