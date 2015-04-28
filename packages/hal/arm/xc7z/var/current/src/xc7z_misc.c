/**************************************************************************/
/**
*
* @file     xc7z_misc.c
*
* @brief    XC7Z Cortex-A9 GIC functions
*
***************************************************************************/
/*==========================================================================
//
//      xc7z_misc.c
//
//      HAL misc board support code for Xilinx XC7Z
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
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
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//========================================================================*/


#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // necessary?
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_if.h>             // calling interface
#include <cyg/hal/hal_misc.h>           // helper functions
#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
#include <cyg/hal/drv_api.h>            // HAL ISR support
#endif
#include <cyg/hal/var_io.h>


/************************** Functions Definitions *****************************/

/****************************************************************************/
/**
*
* Hardware initialization.
*
* @return   none
*
*****************************************************************************/
void hal_hardware_init(void)
{
    cyg_uint32 dw_i, dwCPUID;
    
    // -------- Ininializing GIC -------------------------------------------
    
    // Connect GIC to CORE0
    dwCPUID = 1;
    
    HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + XSCUGIC_DIST_EN_OFFSET, 0UL);
    
    // For the Shared Peripheral Interrupts INT_ID[MAX..32], set:

    //
    // 1. The trigger mode in the int_config register
    // Only write to the SPI interrupts, so start at 32
    //
    for (dw_i = 32; dw_i < XSCUGIC_MAX_NUM_INTR_INPUTS; dw_i += 16) {
        //
        // Each INT_ID uses two bits, or 16 INT_ID per register
        // Set them all to be level sensitive, active HIGH.
        //
        HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + 
                    XSCUGIC_INT_CFG_OFFSET_CALC(dw_i),
                    0UL);
    }
    
    for (dw_i = 0; dw_i < XSCUGIC_MAX_NUM_INTR_INPUTS; dw_i += 4) {
        //
        // 2. The priority using int the priority_level register
        // The priority_level and spi_target registers use one byte per
        // INT_ID.
        // Write a default value that can be changed elsewhere.
        //
        HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + 
                    XSCUGIC_PRIORITY_OFFSET_CALC(dw_i),
                    DEFAULT_PRIORITY);
    }
    
    for (dw_i = 32; dw_i < XSCUGIC_MAX_NUM_INTR_INPUTS; dw_i += 4) {
        //
        // 3. The CPU interface in the spi_target register
        // Only write to the SPI interrupts, so start at 32
        //
        dwCPUID |= dwCPUID << 8;
        dwCPUID |= dwCPUID << 16;

        HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + 
                     XSCUGIC_SPI_TARGET_OFFSET_CALC(dw_i),
                     dwCPUID);
    }

    for (dw_i = 0; dw_i < XSCUGIC_MAX_NUM_INTR_INPUTS; dw_i += 32) {
        //
        // 4. Enable the SPI using the enable_set register. Leave all
        // disabled for now.
        //
        HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + 
        XSCUGIC_ENABLE_DISABLE_OFFSET_CALC(XSCUGIC_DISABLE_OFFSET, dw_i),
            0xFFFFFFFFUL);

    }

    HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + XSCUGIC_DIST_EN_OFFSET,
                        XSCUGIC_EN_INT_MASK);
                        
    //
    // Program the priority mask of the CPU using the Priority mask register
    //
    HAL_WRITE_UINT32(XC7Z_SCUGIC_CPU_BASEADDR + XSCUGIC_CPU_PRIOR_OFFSET, 0xF0);
    
    //
    // If the CPU operates in both security domains, set parameters in the
    // control_s register.
    // 1. Set FIQen=1 to use FIQ for secure interrupts,
    // 2. Program the AckCtl bit
    // 3. Program the SBPR bit to select the binary pointer behavior
    // 4. Set EnableS = 1 to enable secure interrupts
    // 5. Set EnbleNS = 1 to enable non secure interrupts
    //

    //
    // If the CPU operates only in the secure domain, setup the
    // control_s register.
    // 1. Set FIQen=1,
    // 2. Set EnableS=1, to enable the CPU interface to signal secure interrupts.
    // Only enable the IRQ output unless secure interrupts are needed.
    //
    HAL_WRITE_UINT32(XC7Z_SCUGIC_CPU_BASEADDR + XSCUGIC_CONTROL_OFFSET, 0x07);  

#ifdef HAL_PLF_HARDWARE_INIT
    // Perform any platform specific initializations
    HAL_PLF_HARDWARE_INIT();
#endif

    // Set up eCos/ROM interfaces
    hal_if_init();

	HAL_CLOCK_INITIALIZE(CYGNUM_HAL_RTC_PERIOD);
}

/****************************************************************************/
/**
*
* IRQ handler.
* This routine is called to respond to a hardware interrupt (IRQ).  It
* should interrogate the hardware and return the IRQ vector number.
*
* @return   none
*
*****************************************************************************/
int hal_IRQ_handler(void)
{
    cyg_uint32 irq_num;

    HAL_READ_UINT32(XC7Z_SCUGIC_CPU_BASEADDR + XSCUGIC_INT_ACK_OFFSET, irq_num);
    irq_num &= XSCUGIC_ACK_INTID_MASK;

    // An invalid interrupt source is treated as a spurious interrupt    
    if (irq_num < CYGNUM_HAL_ISR_MIN || irq_num > CYGNUM_HAL_ISR_MAX)
      irq_num = CYGNUM_HAL_INTERRUPT_NONE;

    return irq_num;
}

/****************************************************************************/
/**
*
* Interrupt control: disable interrupt.
*
* @param    vector - interrupt number [0..94].
*
* @return   none
*
*****************************************************************************/
void hal_interrupt_mask(int vector)
{
    cyg_uint32 dwMask;
    
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    
    //
    // The Int_Id is used to create the appropriate mask for the
    // desired bit position. Int_Id currently limited to 0 - 31
    //
    dwMask = 0x00000001 << (vector % 32);

    //
    // Disable the selected interrupt source by setting the
    // corresponding bit in the IDR.
    //
    HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + XSCUGIC_DISABLE_OFFSET +
                        ((vector / 32) * 4), dwMask);
}

/****************************************************************************/
/**
*
* Interrupt control: enable interrupt.
*
* @param    vector - interrupt number [0..94].
*
* @return   none
*
*****************************************************************************/
void hal_interrupt_unmask(int vector)
{
    cyg_uint32 dwMask;
    
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    //
    // The vector is used to create the appropriate mask for the
    // desired bit position. vector currently limited to 0 - 31
    //
    dwMask = 0x00000001 << (vector % 32);

    //
    // Enable the selected interrupt source by setting the
    // corresponding bit in the Enable Set register.
    //
    HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + XSCUGIC_ENABLE_SET_OFFSET +
                        ((vector / 32) * 4), dwMask);
}

/****************************************************************************/
/**
*
* Interrupt acknowlage.
*
* @param    vector - interrupt number [0..94].
*
* @return   none
*
*****************************************************************************/
void hal_interrupt_acknowledge(int vector)
{
    // No check for valid vector here! Spurious interrupts
    // must be acknowledged, too.
    
    HAL_WRITE_UINT32(XC7Z_SCUGIC_CPU_BASEADDR + XSCUGIC_EOI_OFFSET, vector); 
}

/****************************************************************************/
/**
*
* Interrupt control: Set interrupt configuration.
*
* @param    vector - interrupt number [0..94].
* @param    level  - priority is the new priority for the IRQ source. 0x00 is highest, 0xFF lowest.
* @param    up     - trigger type for the IRQ source.
*
* @return   none
*
*****************************************************************************/
void hal_interrupt_configure(int vector, int level, int up)
{
    cyg_uint32 dwRegValue;
    
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");

    //
    // Determine the register to write to using the vector.
    //
    HAL_READ_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + 
            XSCUGIC_PRIORITY_OFFSET_CALC(vector), dwRegValue);

    //
    // Shift and Mask the correct bits for the priority and trigger in the
    // register
    //
    dwRegValue &= ~(XSCUGIC_PRIORITY_MASK << ((vector % 4) * 8));
    dwRegValue |= level << ((vector % 4) * 8);

    //
    // Write the value back to the register.
    //
    HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + XSCUGIC_PRIORITY_OFFSET_CALC(vector),
                dwRegValue);

    //
    // Determine the register to write to using the vector.
    //
    HAL_READ_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + 
            XSCUGIC_INT_CFG_OFFSET_CALC (vector), dwRegValue);

    //
    // Shift and Mask the correct bits for the priority and trigger in the
    // register
    //
    dwRegValue &= ~(XSCUGIC_INT_CFG_MASK << (( vector % 16 ) * 2));
    dwRegValue |= up << (( vector % 16 ) * 2);

    //
    // Write the value back to the register.
    //
    HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + XSCUGIC_INT_CFG_OFFSET_CALC(vector),
                dwRegValue);
                
}

/****************************************************************************/
/**
*
* Interrupt control: Set reduced interrupt configuration.
*
* @param    vector - interrupt number [0..94].
* @param    level  - priority is the new priority for the IRQ source. 0x00 is highest, 0xFF lowest.
*
* @return   none
*
*****************************************************************************/
void hal_interrupt_set_level(int vector, int level)
{
    cyg_uint32 dwRegValue;
    
    CYG_ASSERT(vector <= CYGNUM_HAL_ISR_MAX &&
               vector >= CYGNUM_HAL_ISR_MIN , "Invalid vector");
    CYG_ASSERT(level >= 0 && level <= 7, "Invalid level");

    //
    // Determine the register to write to using the vector.
    //
    HAL_READ_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + 
            XSCUGIC_PRIORITY_OFFSET_CALC(vector), dwRegValue);

    //
    // Shift and Mask the correct bits for the priority and trigger in the
    // register
    //
    dwRegValue &= ~(XSCUGIC_PRIORITY_MASK << (( vector % 4) * 8));
    dwRegValue |= level << (( vector % 4) * 8);

    //
    // Write the value back to the register.
    //
    HAL_WRITE_UINT32(XC7Z_SCUGIC_DIST_BASEADDR + XSCUGIC_PRIORITY_OFFSET_CALC(vector),
                dwRegValue);
}

/****************************************************************************/
/**
*
* NOP function.
*
* @param    vector   - interrupt number [0..94].
* @param    data     - Unknown.
* @param    handler  - Unknown.
*
* @return   none
*
*****************************************************************************/
void hal_show_IRQ(int vector, int data, int handler)
{
//    UNDEFINED(__FUNCTION__);  // FIXME
}


//--------------------------------------------------------------------------
// EOF xc7z_misc.c
