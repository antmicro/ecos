//==========================================================================
//
//      i2c_stm32.c
//
//      I2C driver for STM32
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2010 Free Software Foundation, Inc.
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
// Author(s):    Martin Rösch <roscmar@gmail.com>
// Contributors: 
// Date:         2010-10-28
// Description:  I2C bus driver for STM32
//              
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_i2c_cortexm_stm32.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/io/i2c.h>
#include <cyg/io/i2c_stm32.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>

//--------------------------------------------------------------------------
// Diagnostic support
#if CYGPKG_DEVS_I2C_CORTEXM_STM32_DEBUG_LEVEL > 0
   #define error_printf(args...) diag_printf(args)
#else
   #define error_printf(args...)
#endif
#if CYGPKG_DEVS_I2C_CORTEXM_STM32_DEBUG_LEVEL > 1
   #define debug_printf(args...) diag_printf(args)
#else
   #define debug_printf(args...)
#endif

//--------------------------------------------------------------------------
// I2C bus instances
#ifdef CYGHWR_DEVS_I2C_CORTEXM_STM32_BUS1
CYG_STM32_I2C_BUS(i2c_bus1, CYGHWR_HAL_STM32_I2C1,
        CYGNUM_HAL_INTERRUPT_I2C1_EV, CYGNUM_HAL_INTERRUPT_I2C1_EE);
#endif

#ifdef CYGHWR_DEVS_I2C_CORTEXM_STM32_BUS2
CYG_STM32_I2C_BUS(i2c_bus2, CYGHWR_HAL_STM32_I2C2,
        CYGNUM_HAL_INTERRUPT_I2C2_EV, CYGNUM_HAL_INTERRUPT_I2C2_EE);
#endif

#ifdef CYGHWR_DEVS_I2C_CORTEXM_STM32_BUS3
CYG_STM32_I2C_BUS(i2c_bus3, CYGHWR_HAL_STM32_I2C3,
        CYGNUM_HAL_INTERRUPT_I2C3_EV, CYGNUM_HAL_INTERRUPT_I2C3_ER);
#endif

//--------------------------------------------------------------------------
// I2C event mask definitions for polling mode
#define I2C_MS_EV5 (((CYGHWR_HAL_STM32_I2C_SR2_MSL | CYGHWR_HAL_STM32_I2C_SR2_BUSY)<< 16) | CYGHWR_HAL_STM32_I2C_SR1_SB)
#define I2C_MS_RX_EV6 (((CYGHWR_HAL_STM32_I2C_SR2_MSL | CYGHWR_HAL_STM32_I2C_SR2_BUSY)<< 16) | CYGHWR_HAL_STM32_I2C_SR1_ADDR)
#define I2C_MS_RX_EV7 (((CYGHWR_HAL_STM32_I2C_SR2_MSL | CYGHWR_HAL_STM32_I2C_SR2_BUSY)<< 16) | CYGHWR_HAL_STM32_I2C_SR1_RxNE | CYGHWR_HAL_STM32_I2C_SR1_BTF)
#define I2C_MS_RX_EV7_1 (((CYGHWR_HAL_STM32_I2C_SR2_MSL | CYGHWR_HAL_STM32_I2C_SR2_BUSY)<< 16) | CYGHWR_HAL_STM32_I2C_SR1_RxNE)
#define I2C_MS_TX_EV6 (((CYGHWR_HAL_STM32_I2C_SR2_MSL | CYGHWR_HAL_STM32_I2C_SR2_BUSY | CYGHWR_HAL_STM32_I2C_SR2_TRA)<< 16) | CYGHWR_HAL_STM32_I2C_SR1_TxE | CYGHWR_HAL_STM32_I2C_SR1_ADDR)
#define I2C_MS_EV8_1 (((CYGHWR_HAL_STM32_I2C_SR2_MSL | CYGHWR_HAL_STM32_I2C_SR2_BUSY | CYGHWR_HAL_STM32_I2C_SR2_TRA)<< 16) | CYGHWR_HAL_STM32_I2C_SR1_TxE)
#define I2C_MS_EV8 (((CYGHWR_HAL_STM32_I2C_SR2_MSL | CYGHWR_HAL_STM32_I2C_SR2_BUSY | CYGHWR_HAL_STM32_I2C_SR2_TRA)<< 16) | CYGHWR_HAL_STM32_I2C_SR1_TxE | CYGHWR_HAL_STM32_I2C_SR1_BTF)
#define I2C_MS_EV8_2 (((CYGHWR_HAL_STM32_I2C_SR2_MSL | CYGHWR_HAL_STM32_I2C_SR2_BUSY | CYGHWR_HAL_STM32_I2C_SR2_TRA)<< 16) | CYGHWR_HAL_STM32_I2C_SR1_TxE | CYGHWR_HAL_STM32_I2C_SR1_BTF)

//--------------------------------------------------------------------------
// Poll I2C status registers (SR2, SR1) until they are equal to a given event
// mask or until timeout.
inline static cyg_uint32 i2c_wait_for_event(cyg_uint32 event_mask, \
        CYG_ADDRESS base)
{
    static cyg_uint32 sr1 = 0, sr2 = 0, tmo = 0;
    do {
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, sr1);
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR2, sr2);
        tmo++;
    } while(((sr2<<16 |sr1) != event_mask) && (tmo < 0x000FFFFF));
    return (sr2<<16 |sr1) ^ event_mask;
}

//--------------------------------------------------------------------------
// The event ISR does the actual work. It is not that much work to justify
// putting it in the DSR, and it is also not clear whether this would
// even work.
//
// The logic for extra->i2c_rxnak is commented out. It's purpose is to 
// follow the API and let the caller decide whether to NACK. However,
// when it is set to false, the STOP fails, and even a reset of the target
// does not seem to clean things up. Only a power cycle cleared the problem.
// Therefore, it is in the code for completeness, but disabled so that the
// caller can't shoot themselves in the foot.
static cyg_uint32 stm32_i2c_ev_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    static cyg_stm32_i2c_extra* extra = NULL;
    static CYG_ADDRESS base = 0;
    static cyg_uint16  sr1, sr2, cr1, cr2;
    static cyg_bool call_dsr = false;
    extra = (cyg_stm32_i2c_extra*)data;
    base = extra->i2c_base;
    call_dsr = false;
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, sr1);
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR2, sr2);
    extra->i2c_status = (sr2 << 16) | sr1;
    if((sr2 & CYGHWR_HAL_STM32_I2C_SR2_MSL) > 0)
    {
        // Master Mode
        if((sr1 & CYGHWR_HAL_STM32_I2C_SR1_SB) > 0)
        {
            // Start condition generated, send address
            HAL_WRITE_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, extra->i2c_addr);
        }
        if((sr1 & CYGHWR_HAL_STM32_I2C_SR1_ADDR) > 0)
        {
            // Address acknowledged, read SR1 and SR2 to clear
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, sr1);
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR2, sr2);
            // check whether data has to be transmitted
            if((sr2 & CYGHWR_HAL_STM32_I2C_SR2_TRA) > 0)
            {
                // Master transmitter
                // send 1st data byte
                HAL_WRITE_UINT8(base + CYGHWR_HAL_STM32_I2C_DR,
                        *(extra->i2c_txbuf));
                extra->i2c_txbuf++;
            }
            else
            {
                // Master receiver
                if(extra->i2c_count == 1)
                {
                    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
                    // Only NACK last byte if enabled
//                    if (extra->i2c_rxnak)
                        cr1 &= ~CYGHWR_HAL_STM32_I2C_CR1_ACK;
                    if(extra->i2c_stop)
                    {
                        cr1 |= CYGHWR_HAL_STM32_I2C_CR1_STOP;
                    }
                    else
                    {
                        // Create restart condition
                        cr1 |= CYGHWR_HAL_STM32_I2C_CR1_START;
                    }
                    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
                }
            }
        }
        if((sr1 & CYGHWR_HAL_STM32_I2C_SR1_TxE) > 0)
        {
            if((sr1 & CYGHWR_HAL_STM32_I2C_SR1_BTF) > 0)
            {
                // Last byte transfered
                if(extra->i2c_stop)
                {
                    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
                    cr1 |= CYGHWR_HAL_STM32_I2C_CR1_STOP;
                    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
                }
                else
                {
                    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
                    cr1 |= CYGHWR_HAL_STM32_I2C_CR1_START;
                    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
                }
                // Disable EVT IT to prevent another BTF IT
                HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, cr2);
                cr2 &= ~CYGHWR_HAL_STM32_I2C_CR2_ITEVTEN;
                HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, cr2);
                // Wake up driver
                call_dsr = true;
            }
            else
            {
                extra->i2c_count--;
                if(extra->i2c_count > 0)
                {
                    HAL_WRITE_UINT8(base + CYGHWR_HAL_STM32_I2C_DR,
                            *(extra->i2c_txbuf));
                    extra->i2c_txbuf++;
                }
                else
                {
                    // If  no data remains to write, disable the BUF interrupt
                    // to prevent another TxE interrupt.
                    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, cr2);
                    cr2 &= ~CYGHWR_HAL_STM32_I2C_CR2_ITBUFEN;
                    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, cr2);
                }
            }
        }
        if((sr1 & CYGHWR_HAL_STM32_I2C_SR1_RxNE) > 0)
        {
            if(extra->i2c_count > 1)
            {
                // Data ACKed, read next byte
                HAL_READ_UINT8(base + CYGHWR_HAL_STM32_I2C_DR,
                        *(extra->i2c_rxbuf));
                extra->i2c_rxbuf++;
                extra->i2c_count--;
                // Disable ACK for the last byte
                if(extra->i2c_count == 1)
                {
                    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
                    // Only NACK last byte if enabled
//                    if (extra->i2c_rxnak)
                        cr1 &= ~CYGHWR_HAL_STM32_I2C_CR1_ACK;
                    if(extra->i2c_stop)
                    {
                        cr1 |= CYGHWR_HAL_STM32_I2C_CR1_STOP;
                    }
                    else
                    {
                        // Create restart condition
                        cr1 |= CYGHWR_HAL_STM32_I2C_CR1_START;
                    }
                    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
                }
            }
            else
            {
                // Disable interrupts, to prevent another interrupt frm
                // from a restart condition
                HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, cr2);
                cr2 &= ~CYGHWR_HAL_STM32_I2C_CR2_ITBUFEN;
                cr2 &= ~CYGHWR_HAL_STM32_I2C_CR2_ITEVTEN;
                HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, cr2);

                // Data NACKed, read last byte
                HAL_READ_UINT8(base + CYGHWR_HAL_STM32_I2C_DR,
                        *(extra->i2c_rxbuf));
                extra->i2c_count--;

                // last byte received, wakeup driver
                call_dsr = true;
            }
        }
    }
    else
    {
        // Slave mode
        // TODO: implement
        diag_printf("Slave mode.\n");
    }
    // Acknowledge interrupt
    cyg_drv_interrupt_acknowledge(vec);

    // We need to call the DSR only if there is really something to signal
    if (call_dsr)
    {
        return CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
    }
    else
    {
        return CYG_ISR_HANDLED;
    }
}

//--------------------------------------------------------------------------
// The error ISR wakes up the driver in case an error occurred. Both status
// registers are stored in the i2c_status field (high and low word for SR2
// and SR1 respectively) of the driver data.
static cyg_uint32 stm32_i2c_err_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    static cyg_stm32_i2c_extra* extra = NULL;
    CYG_ADDRESS base;
    cyg_uint16  sr1, sr2, cr1;

    extra = (cyg_stm32_i2c_extra*)data;
    base = extra->i2c_base;
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, sr1);
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR2, sr2);
    extra->i2c_status = (sr2 << 16) | sr1;
    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, 0);  //clear it
    // Stop the transfer
    if((sr2 & CYGHWR_HAL_STM32_I2C_SR2_BUSY) > 0)
    {
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
        cr1 |= CYGHWR_HAL_STM32_I2C_CR1_STOP;
        HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, cr1);
    }
    // Acknowledge interrupt
    cyg_drv_interrupt_acknowledge(vec);
    return CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
}

//--------------------------------------------------------------------------
// The event DSR just wakes up the driver code.
static void stm32_i2c_ev_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)data;
    CYG_ASSERT(extra->i2c_wait.mutex->locked == 0, "i2c_wait locked");
    cyg_drv_cond_signal(&extra->i2c_wait);
}

//--------------------------------------------------------------------------
// The error DSR just wakes up the driver code.
static void stm32_i2c_err_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)data;
    error_printf("I2C error: 0x%x\n", extra->i2c_status);
    cyg_drv_cond_signal(&extra->i2c_wait);
}

//--------------------------------------------------------------------------
// Transmits a buffer to a device in interrupt mode
cyg_uint32 cyg_stm32_i2c_tx_int(const cyg_i2c_device *dev,
                            cyg_bool              send_start,
                            const cyg_uint8      *tx_data,
                            cyg_uint32            count,
                            cyg_bool              send_stop)
{
    cyg_stm32_i2c_extra* extra =
                           (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint16 reg;
    extra->i2c_addr  = dev->i2c_address << 1;
    extra->i2c_count = count;
    extra->i2c_txbuf = tx_data;
    extra->i2c_stop = send_stop;

    while(!cyg_drv_mutex_lock(&extra->i2c_lock));
    cyg_drv_dsr_lock();
    cyg_drv_interrupt_unmask(extra->i2c_ev_vec);
    cyg_drv_interrupt_unmask(extra->i2c_err_vec);

    if(send_start)
    {
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        if((reg & CYGHWR_HAL_STM32_I2C_SR1_SB) == 0)
        {
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
            reg |= CYGHWR_HAL_STM32_I2C_CR1_START;
            HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        }
    }
    else
    {
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        HAL_WRITE_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, extra->i2c_addr);
    }

    // Enable I2C bus interrupts
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR2_ITEVTEN;
    reg |= CYGHWR_HAL_STM32_I2C_CR2_ITBUFEN;
    reg |= CYGHWR_HAL_STM32_I2C_CR2_ITERREN;
    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    // The ISR will do most of the work, and the DSR will signal when an
    // error occurred or the transfer finished
    cyg_drv_cond_wait(&extra->i2c_wait);

    cyg_drv_interrupt_mask(extra->i2c_ev_vec);
    cyg_drv_interrupt_mask(extra->i2c_err_vec);
    cyg_drv_mutex_unlock(&extra->i2c_lock);
    cyg_drv_dsr_unlock();

    // Disable I2C bus interrupts
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg &= ~CYGHWR_HAL_STM32_I2C_CR2_ITEVTEN;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR2_ITBUFEN;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR2_ITERREN;
    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    count -= extra->i2c_count;

    extra->i2c_addr  = 0;
    extra->i2c_count = 0;
    extra->i2c_txbuf = NULL;
    return count;
}

//--------------------------------------------------------------------------
// Receive into a buffer from a device in interrupt mode
cyg_uint32 cyg_stm32_i2c_rx_int(const cyg_i2c_device *dev,
                            cyg_bool              send_start,
                            cyg_uint8            *rx_data,
                            cyg_uint32            count,
                            cyg_bool              send_nak,
                            cyg_bool              send_stop)
{
    cyg_stm32_i2c_extra* extra =
                           (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint16 reg;
    extra->i2c_addr  = (dev->i2c_address << 1) | 1;
    extra->i2c_count = count;
    extra->i2c_rxbuf = rx_data;
    extra->i2c_rxnak = send_nak;
    extra->i2c_stop = send_stop;

    cyg_drv_mutex_lock(&extra->i2c_lock);
    cyg_drv_dsr_lock();
    cyg_drv_interrupt_unmask(extra->i2c_ev_vec);
    cyg_drv_interrupt_unmask(extra->i2c_err_vec);

    // If we continue a previous transfer, read the last byte
    if(send_start)
    {
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        if((reg & CYGHWR_HAL_STM32_I2C_SR1_SB) == 0)
        {
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
            reg |= CYGHWR_HAL_STM32_I2C_CR1_START;
            HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        }
    }
    else
    {
        // Restart condition generated, send address
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        HAL_WRITE_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, extra->i2c_addr);
    }

    // Enable I2C bus interrupts
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR2_ITEVTEN;
    reg |= CYGHWR_HAL_STM32_I2C_CR2_ITBUFEN;
    reg |= CYGHWR_HAL_STM32_I2C_CR2_ITERREN;
    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    // The ISR will do most of the work, and the DSR will signal when an
    // error occurred or the transfer finished
    cyg_drv_cond_wait(&extra->i2c_wait);

    cyg_drv_interrupt_mask(extra->i2c_ev_vec);
    cyg_drv_interrupt_mask(extra->i2c_err_vec);
    cyg_drv_dsr_unlock();
    cyg_drv_mutex_unlock(&extra->i2c_lock);

    // Disable I2C bus interrupts
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg &= ~CYGHWR_HAL_STM32_I2C_CR2_ITEVTEN;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR2_ITBUFEN;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR2_ITERREN;
    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    // Enable Acknowledgment to be ready for another reception
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR1_ACK;
    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);

    count -= extra->i2c_count;

    extra->i2c_addr  = 0;
    extra->i2c_count = 0;
    extra->i2c_rxbuf = NULL;

    return count;
}

//--------------------------------------------------------------------------
// Transmit a buffer to a device in polling mode
cyg_uint32 cyg_stm32_i2c_tx_poll(const cyg_i2c_device *dev,
                            cyg_bool              send_start,
                            const cyg_uint8      *tx_data,
                            cyg_uint32            count,
                            cyg_bool              send_stop)
{
    cyg_stm32_i2c_extra* extra =
                           (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint16 reg;
    extra->i2c_addr  = dev->i2c_address << 1;
    extra->i2c_count = 0;
    extra->i2c_txbuf = tx_data;

    cyg_drv_dsr_lock();
    // If we continue a previous transfer, load the next byte
    if(send_start) 
    {
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        if((reg & CYGHWR_HAL_STM32_I2C_SR1_SB) == 0)
        {
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
            reg |= CYGHWR_HAL_STM32_I2C_CR1_START;
            HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);

            // Master EV5
            extra->i2c_status = i2c_wait_for_event(I2C_MS_EV5, base);
            if(extra->i2c_status != 0)
            {
                error_printf("I2C: Start condition failed. 0x%x\n",
                        extra->i2c_status);
                cyg_drv_dsr_unlock();
                return 0;
            }
            debug_printf("I2C: Start condition generated.\n");
        }
    }
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
    // Send address
    HAL_WRITE_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, extra->i2c_addr);
    // Master Tx EV6
    extra->i2c_status = i2c_wait_for_event(I2C_MS_TX_EV6, base);
    if(extra->i2c_status != 0)
    {
        error_printf("I2C: Send address failed. 0x%x, EV6=0x%x\n",
                extra->i2c_status,I2C_MS_TX_EV6);
        cyg_drv_dsr_unlock();
        return 0;
    }
    // Read SR1 then SR2 to clear ADDR bit
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR2, reg);
    extra->i2c_status = i2c_wait_for_event(I2C_MS_EV8_1, base);
    if(extra->i2c_status != 0)
    {
        error_printf("I2C: TxE not set. 0x%x, EV8_1=0x%x\n",
                extra->i2c_status, I2C_MS_EV8_1);
        cyg_drv_dsr_unlock();
        return 0;
    }
    debug_printf("I2C: Address sent.\n");
    // Master EV8
    do {
        HAL_WRITE_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, *(extra->i2c_txbuf));
        // Master EV8
        // Wait for byte to be transferred
        extra->i2c_status = i2c_wait_for_event(I2C_MS_EV8, base);
        if(extra->i2c_status != 0)
        {
            error_printf("I2C: Could not transfer byte. 0x%x, EV8=0x%x\n",
                    extra->i2c_status, I2C_MS_EV8);
            break;
        }
        extra->i2c_txbuf++;
        extra->i2c_count++;
    } while(extra->i2c_count < count);
    debug_printf("I2C: %u bytes sent.\n", extra->i2c_count);

    if(send_stop)
    {
        // Set stop condition
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        reg |= CYGHWR_HAL_STM32_I2C_CR1_STOP;
        HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        // Make sure that the STOP bit is cleared by hardware
        do
        {
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        } while((reg & CYGHWR_HAL_STM32_I2C_CR1_STOP) != 0);
        debug_printf("I2C: Stop condition generated.\n");
    }
    else
    {
        // Set start condition again
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        reg |= CYGHWR_HAL_STM32_I2C_CR1_START;
        HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        // Master EV5
        extra->i2c_status = i2c_wait_for_event(I2C_MS_EV5, base);
        if(extra->i2c_status != 0)
        {
            error_printf("I2C: Start condition failed. 0x%x\n",
                    extra->i2c_status);
            cyg_drv_dsr_unlock();
            return 0;
        }
        debug_printf("I2C: Start condition generated.\n");
    }
    cyg_drv_dsr_unlock();
    return extra->i2c_count;
}

//--------------------------------------------------------------------------
// Receive into a buffer from a device in polling mode
//
// The logic for extra->i2c_rxnak is commented out. It's purpose is to 
// follow the API and let the caller decide whether to NACK. However,
// when it is set to false, the STOP fails, and even a reset of the target
// does not seem to clean things up. Only a power cycle cleared the problem.
// Therefore, it is in the code for completeness, but disabled so that the
// caller can't shoot themselves in the foot.
cyg_uint32 cyg_stm32_i2c_rx_poll(const cyg_i2c_device *dev,
                            cyg_bool              send_start,
                            cyg_uint8            *rx_data,
                            cyg_uint32            count,
                            cyg_bool              send_nak,
                            cyg_bool              send_stop)
{
    cyg_stm32_i2c_extra* extra =
                           (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint16 reg;
    extra->i2c_addr  = (dev->i2c_address << 1) | 0x01;
    extra->i2c_count = 0;
    extra->i2c_rxbuf = rx_data;
    extra->i2c_rxnak = send_nak;

    if(count == 0)
        return 0;

    cyg_drv_dsr_lock();
    if(count == 2)
    {
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        if((reg & CYGHWR_HAL_STM32_I2C_SR1_SB) == 0)
        {
            // In polling mode, the reception of 2 bytes needs special treatment
            // Generate start condition and set POS
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
            reg |= CYGHWR_HAL_STM32_I2C_CR1_START;
            reg |= CYGHWR_HAL_STM32_I2C_CR1_POS;
            HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
            // Master EV5
            extra->i2c_status = i2c_wait_for_event(I2C_MS_EV5, base);
            if(extra->i2c_status != 0)
            {
                error_printf("I2C: Start condition failed. 0x%x\n", extra->i2c_status);
                cyg_drv_dsr_unlock();
                return 0;
            }
            debug_printf("I2C: Start condition generated.\n");
        }
        else
        {
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
            reg |= CYGHWR_HAL_STM32_I2C_CR1_POS;
            HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        }
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        // Send address
        HAL_WRITE_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, extra->i2c_addr);
        // Master EV6
        extra->i2c_status = i2c_wait_for_event(I2C_MS_RX_EV6, base);
        if(extra->i2c_status != 0)
        {
            error_printf("I2C: Send address failed. 0x%x, EV6=0x%x\n",
                    extra->i2c_status, I2C_MS_RX_EV6);
            cyg_drv_dsr_unlock();
            return 0;
        }
        cyg_drv_isr_lock();
        // Read SR1 then SR2 to clear ADDR bit
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR2, reg);

        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        // Only NACK last byte if enabled
//        if (extra->i2c_rxnak)
            reg &= ~CYGHWR_HAL_STM32_I2C_CR1_ACK;
        HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        cyg_drv_isr_unlock();
        // Wait until BTF bit is set
        do
        {
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        } while ((reg & CYGHWR_HAL_STM32_I2C_SR1_BTF) == 0);
        cyg_drv_isr_lock();
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        if(send_stop)
            reg |= CYGHWR_HAL_STM32_I2C_CR1_STOP;
        else
            reg |= CYGHWR_HAL_STM32_I2C_CR1_START;
        HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        // Read DataN-1
        HAL_READ_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, *(extra->i2c_rxbuf++));
        cyg_drv_isr_unlock();
        extra->i2c_count++;
        // Read DataN
        HAL_READ_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, *(extra->i2c_rxbuf));
        extra->i2c_count++;
        if(send_stop)
        {
            // Wait until STOP bit is cleared
            do
            {
                HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
            } while ((reg & CYGHWR_HAL_STM32_I2C_CR1_STOP) > 0);
        }
        // Enable ACK again to be ready for other reception
        // Clear POS flag
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        reg &= ~CYGHWR_HAL_STM32_I2C_CR1_POS;
        reg |= CYGHWR_HAL_STM32_I2C_CR1_ACK;
        HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        return extra->i2c_count;

    }
    if(send_start) 
    {
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
        if((reg & CYGHWR_HAL_STM32_I2C_SR1_SB) == 0)
        {
            // Generate start condition
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
            reg |= CYGHWR_HAL_STM32_I2C_CR1_START;
            HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);

            // Master EV5
            extra->i2c_status = i2c_wait_for_event(I2C_MS_EV5, base);
            if(extra->i2c_status != 0)
            {
                error_printf("I2C: Start condition failed. 0x%x\n",
                        extra->i2c_status);
                cyg_drv_dsr_unlock();
                return 0;
            }
            debug_printf("I2C: Start condition generated.\n");
        }
    }
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
    // Send address
    HAL_WRITE_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, extra->i2c_addr);
    // Master EV6
    extra->i2c_status = i2c_wait_for_event(I2C_MS_RX_EV6, base);
    if(extra->i2c_status != 0)
    {
        error_printf("I2C: Send address failed. 0x%x, EV6=0x%x\n",
                extra->i2c_status, I2C_MS_RX_EV6);
        cyg_drv_dsr_unlock();
        return 0;
    }
    // Read SR1 then SR2 to clear ADDR bit
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR1, reg);
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_SR2, reg);

    while(count > (extra->i2c_count + 2))
    {
        extra->i2c_status = i2c_wait_for_event(I2C_MS_RX_EV7, base);
        if(extra->i2c_status != 0)
        {
            error_printf("I2C: Byte reception failed. 0x%x, EV7=0x%x\n",
                    extra->i2c_status, I2C_MS_RX_EV7);
            cyg_drv_dsr_unlock();
            return extra->i2c_count;
        }
        HAL_READ_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, *(extra->i2c_rxbuf++));
        extra->i2c_count++;
    }
    if(count > 2)
    {
        extra->i2c_status = i2c_wait_for_event(I2C_MS_RX_EV7_1, base);
        if(extra->i2c_status != 0)
        {
            error_printf("I2C: Byte reception failed. 0x%x, EV7=0x%x\n",
                    extra->i2c_status, I2C_MS_RX_EV7);
            cyg_drv_dsr_unlock();
            return extra->i2c_count;
        }
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        reg &= ~CYGHWR_HAL_STM32_I2C_CR1_ACK;
        HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        cyg_drv_isr_lock();
        HAL_READ_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, *(extra->i2c_rxbuf++));
        extra->i2c_count++;
    }
    else
    {
        cyg_drv_isr_lock();
    }
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    // Only NACK last byte if enabled
//    if (extra->i2c_rxnak)
        reg &= ~CYGHWR_HAL_STM32_I2C_CR1_ACK;
    if(send_stop)
        reg |= CYGHWR_HAL_STM32_I2C_CR1_STOP;
    else
        reg |= CYGHWR_HAL_STM32_I2C_CR1_START;
    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    cyg_drv_isr_unlock();

    extra->i2c_status = i2c_wait_for_event(I2C_MS_RX_EV7_1, base);
    if(extra->i2c_status != 0)
    {
        error_printf("I2C: Last Byte reception failed. 0x%x, EV7_1=0x%x\n",
                extra->i2c_status, I2C_MS_RX_EV7_1);
        cyg_drv_dsr_unlock();
        return extra->i2c_count;
    }

    // Read DataN
    HAL_READ_UINT8(base + CYGHWR_HAL_STM32_I2C_DR, *(extra->i2c_rxbuf));
    extra->i2c_count++;
    cyg_drv_dsr_unlock();
    if(send_stop)
    {
        // Wait until STOP bit is cleared
        do
        {
            HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
        } while ((reg & CYGHWR_HAL_STM32_I2C_CR1_STOP) > 0);
    }
    // Enable ACK again to be ready for other reception
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR1_ACK;
    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    return extra->i2c_count;
}

//--------------------------------------------------------------------------
//  Generate a STOP condtition
void cyg_stm32_i2c_stop(const cyg_i2c_device *dev)
{
    cyg_stm32_i2c_extra* extra =
                           (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint16 reg;
    extra = extra; // Avoid compiler warning in case of singleton
    // Set stop condition
    HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR1_STOP;
    HAL_WRITE_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    // Make sure that the STOP bit is cleared by hardware
    do
    {
        HAL_READ_UINT16(base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    } while((reg & CYGHWR_HAL_STM32_I2C_CR1_STOP) != 0);
    debug_printf("I2C: Stop condition generated.\n");
}

//--------------------------------------------------------------------------
// This function is called from the generic I2C infrastructure to initialize
// a bus device. It should perform any work needed to start up the device.
void cyg_stm32_i2c_init(struct cyg_i2c_bus *bus)
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)bus->i2c_extra;
    extern cyg_uint32 hal_stm32_pclk1;
    cyg_uint32 pinspec_scl, pinspec_sda; // Pin configurations
    cyg_uint32 bus_freq = 100000; // I2C bus frequency in Hz
    cyg_uint8  duty = 0; // duty cycle for fast mode (0 = 2, 1 = 16/9)
    cyg_uint8  smbus = 0; // smbus mode (0 = I2C, 1 = SMBUS)
    cyg_uint8  bus_addr = 0x01; // I2C bus address
    CYG_ADDRESS base;
    cyg_uint16 reg;
    cyg_uint16 tmp;
    cyg_uint32 rcc;

    // Assuming the I2C peripheral is disabled as this is called after reset.

    // Setup GPIO Pins
    // The built in assumptions are that the peripheral clock must be started for 
    // the reset to work and that reset works when the peripheral is disabled.
    if(extra->i2c_base == CYGHWR_HAL_STM32_I2C1)
    {
#ifdef CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS1_BUSFREQ
        bus_freq = CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS1_BUSFREQ;
#endif
#ifdef CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS1_DUTY_CYCLE_16TO9
        duty = 1;
#endif
#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS1_MODE
        smbus = 1;
#endif
#ifdef CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS1_ADDRESS
        bus_addr = CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS1_ADDRESS;
#endif

#if defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1)
#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS1_REMAP
        base = CYGHWR_HAL_STM32_AFIO;
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_AFIO_MAPR, rcc);
        // Remap I2C1 due to conflict with FSMCs NADV signal
        rcc |= CYGHWR_HAL_STM32_AFIO_MAPR_I2C1_RMP;
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_AFIO_MAPR, rcc);
        pinspec_scl = CYGHWR_HAL_STM32_GPIO(B, 8, OUT_50MHZ, ALT_OPENDRAIN);
        pinspec_sda = CYGHWR_HAL_STM32_GPIO(B, 9, OUT_50MHZ, ALT_OPENDRAIN);
#else
        pinspec_scl = CYGHWR_HAL_STM32_GPIO(B, 6, OUT_50MHZ, ALT_OPENDRAIN);
        pinspec_sda = CYGHWR_HAL_STM32_GPIO(B, 7, OUT_50MHZ, ALT_OPENDRAIN);
#endif
#elif defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_HIPERFORMANCE)
#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS1_REMAP
	pinspec_scl = CYGHWR_HAL_STM32_GPIO(B, 8, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
	pinspec_sda = CYGHWR_HAL_STM32_GPIO(B, 7, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
#else
//XXX
//	pinspec_scl = CYGHWR_HAL_STM32_GPIO(B, 6, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
	pinspec_scl = CYGHWR_HAL_STM32_GPIO(B, 8, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
	pinspec_sda = CYGHWR_HAL_STM32_GPIO(B, 9, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
#endif
#endif

#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS1_MODE_POLL
        // register functions
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
#endif
#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS1_MODE_INT
        // Initialize i2c event interrupt
        cyg_drv_interrupt_create(extra->i2c_ev_vec,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS1_MODE_INT_EV_PRI,
                (cyg_addrword_t) extra,
                &stm32_i2c_ev_isr,
                &stm32_i2c_ev_dsr,
                &(extra->i2c_ev_interrupt_handle),
                &(extra->i2c_ev_interrupt_data));
        cyg_drv_interrupt_attach(extra->i2c_ev_interrupt_handle);

        // Initialize i2c error interrupt
        cyg_drv_interrupt_create(extra->i2c_err_vec,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS1_MODE_INT_EE_PRI,
                (cyg_addrword_t) extra,
                &stm32_i2c_err_isr,
                &stm32_i2c_err_dsr,
                &(extra->i2c_err_interrupt_handle),
                &(extra->i2c_err_interrupt_data));
        cyg_drv_interrupt_attach(extra->i2c_err_interrupt_handle);
        // register functions
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_int;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_int;
#endif
        // Enable peripheral clock
        base = CYGHWR_HAL_STM32_RCC;
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
#if defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1)
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB2ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB2ENR_IOPB);        
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB2ENR_AFIO);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB2ENR, rcc);
#elif defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_HIPERFORMANCE)
        // This register exists, but not these bits.
#endif

        // Reset peripheral
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
    }
    else if(extra->i2c_base == CYGHWR_HAL_STM32_I2C2)
    {
#ifdef CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS2_BUSFREQ
        bus_freq = CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS2_BUSFREQ;
#endif
#ifdef CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS2_DUTY_CYCLE_16TO9
        duty = 1;
#endif
#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS2_MODE
        smbus = 1;
#endif
#ifdef CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS2_ADDRESS
        bus_addr = CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS2_ADDRESS;
#endif
#if defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1)
        pinspec_scl = CYGHWR_HAL_STM32_GPIO(B, 10, OUT_50MHZ, ALT_OPENDRAIN);
        pinspec_sda = CYGHWR_HAL_STM32_GPIO(B, 11, OUT_50MHZ, ALT_OPENDRAIN);
#elif defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_HIPERFORMANCE)
	pinspec_scl = CYGHWR_HAL_STM32_GPIO(B, 10, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
	pinspec_sda = CYGHWR_HAL_STM32_GPIO(B, 11, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
#endif

#ifdef CYGINT_DEVS_I2C_CORTEXM_STM32_BUS2_MODE_POLL
        // register functions
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
#endif
#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS2_MODE_INT
        // Initialize i2c event interrupt
        cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_I2C2_EV,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS2_MODE_INT_EV_PRI,
                (cyg_addrword_t) extra,
                &stm32_i2c_ev_isr,
                &stm32_i2c_ev_dsr,
                &(extra->i2c_ev_interrupt_handle),
                &(extra->i2c_ev_interrupt_data));
        cyg_drv_interrupt_attach(extra->i2c_ev_interrupt_handle);

        // Initialize i2c error interrupt
        cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_I2C2_EE,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS2_MODE_INT_EE_PRI,
                (cyg_addrword_t) extra,
                &stm32_i2c_err_isr,
                &stm32_i2c_err_dsr,
                &(extra->i2c_err_interrupt_handle),
                &(extra->i2c_err_interrupt_data));
        cyg_drv_interrupt_attach(extra->i2c_err_interrupt_handle);
        // register functions
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_int;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_int;
#endif
        // Enable peripheral clock
        base = CYGHWR_HAL_STM32_RCC;
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
#if defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1)
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB2ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB2ENR_IOPB);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB2ENR_AFIO);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB2ENR, rcc);
#elif defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_HIPERFORMANCE)
        // This register exists, but not these bits.
#endif

        // Reset peripheral
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
    }
    else if(extra->i2c_base == CYGHWR_HAL_STM32_I2C3)
    {
#ifdef CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS3_BUSFREQ
        bus_freq = CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS3_BUSFREQ;
#endif
#ifdef CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS3_DUTY_CYCLE_16TO9
        duty = 1;
#endif
#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS3_MODE
        smbus = 1;
#endif
#ifdef CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS3_ADDRESS
        bus_addr = CYGNUM_DEVS_I2C_CORTEXM_STM32_BUS3_ADDRESS;
#endif
#if defined (CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS3_REMAP)
    pinspec_scl = CYGHWR_HAL_STM32_GPIO(A, 8, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
    pinspec_sda = CYGHWR_HAL_STM32_GPIO(C, 9, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);

#else
    pinspec_scl = CYGHWR_HAL_STM32_GPIO(H, 7, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
    pinspec_sda = CYGHWR_HAL_STM32_GPIO(H, 8, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
#endif

#ifdef CYGINT_DEVS_I2C_CORTEXM_STM32_BUS3_MODE_POLL
        // register functions
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
#endif
#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS3_MODE_INT
        // Initialize i2c event interrupt
        cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_I2C3_EV,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS3_MODE_INT_EV_PRI,
                (cyg_addrword_t) extra,
                &stm32_i2c_ev_isr,
                &stm32_i2c_ev_dsr,
                &(extra->i2c_ev_interrupt_handle),
                &(extra->i2c_ev_interrupt_data));
        cyg_drv_interrupt_attach(extra->i2c_ev_interrupt_handle);

        // Initialize i2c error interrupt
        cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_I2C3_ER,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS3_MODE_INT_EE_PRI,
                (cyg_addrword_t) extra,
                &stm32_i2c_err_isr,
                &stm32_i2c_err_dsr,
                &(extra->i2c_err_interrupt_handle),
                &(extra->i2c_err_interrupt_data));
        cyg_drv_interrupt_attach(extra->i2c_err_interrupt_handle);
        // register functions
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_int;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_int;
#endif
        // Enable peripheral clock
        base = CYGHWR_HAL_STM32_RCC;
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C3);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
#if defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1)
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB2ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB2ENR_IOPB);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB2ENR_AFIO);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB2ENR, rcc);
#elif defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_HIPERFORMANCE)
        // This register exists, but not these bits.
#endif

        // Reset peripheral
        HAL_READ_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C3);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C3);
        HAL_WRITE_UINT32 (base + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
    }

    CYGHWR_HAL_STM32_GPIO_SET(pinspec_scl);
    CYGHWR_HAL_STM32_GPIO_SET(pinspec_sda);

    // Software reset to be double sure things are in a default state
    base = extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1;
    HAL_READ_UINT16(base, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR1_SWRST;
    HAL_WRITE_UINT16(base, reg);
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_SWRST;
    HAL_WRITE_UINT16(base, reg);

    // Setup I2C peripheral clock and disable interrupts as a side effect
    // Interrupts will be managed during transfer.
    base = extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2;
    reg = CYGHWR_HAL_STM32_I2C_CR2_FREQ(hal_stm32_pclk1/1000000);
    HAL_WRITE_UINT16(base, reg);

    // Disable I2C peripheral because the datasheet says some commands
    // don't work with it enabled, for example slew rate.
    base = extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1;
    HAL_READ_UINT16(base, reg);
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_PE; // Disable peripheral
    HAL_WRITE_UINT16(base, reg);

    // Setup I2C bus frequency and set bus mode
    base = extra->i2c_base + CYGHWR_HAL_STM32_I2C_CCR;
    reg = 0;
    if(bus_freq <= 100000)
    {
        // Standard mode
        reg = (hal_stm32_pclk1 / bus_freq) / 2; // CCR = Thigh/Tpclk
        if(reg < 0x04)
            reg = 0x04;
        reg &= CYGHWR_HAL_STM32_I2C_CCR_MASK;
        reg &= ~CYGHWR_HAL_STM32_I2C_CCR_F_S; // Set standard mode
        reg &= ~CYGHWR_HAL_STM32_I2C_CCR_DUTY; // ignored in standard
        HAL_WRITE_UINT16(base, reg);

        // Setup I2C rise time for standard mode
        base = extra->i2c_base + CYGHWR_HAL_STM32_I2C_TRISE;
        tmp = hal_stm32_pclk1/1000000;
        reg = (tmp + 1) & CYGHWR_HAL_STM32_I2C_CCR_MASK;
        HAL_WRITE_UINT16(base, reg);
    }
    else
    {
        // Fast mode
        if(duty == 0)
        {
            // Fast mode speed calculate: Tlow/Thigh = 2
            reg = (hal_stm32_pclk1 / bus_freq) / 3;
            reg &= CYGHWR_HAL_STM32_I2C_CCR_MASK;
            reg &= ~CYGHWR_HAL_STM32_I2C_CCR_DUTY;
        }
        else
        {
            // Fast mode speed calculate: Tlow/Thigh = 16/9
            reg = (hal_stm32_pclk1 / bus_freq) / 25;
            reg &= CYGHWR_HAL_STM32_I2C_CCR_MASK;
            reg |= CYGHWR_HAL_STM32_I2C_CCR_DUTY;
        }
        if((reg & CYGHWR_HAL_STM32_I2C_CCR_MASK) == 0)
            reg |= (cyg_uint16)0x0001;
        reg |= CYGHWR_HAL_STM32_I2C_CCR_F_S; // Set fast mode
        HAL_WRITE_UINT16(base, reg);

        // Set I2C rise time for fast mode
        base = extra->i2c_base + CYGHWR_HAL_STM32_I2C_TRISE;
        tmp = hal_stm32_pclk1/1000000;
        reg = (((tmp * 300) / 1000) + 1) & CYGHWR_HAL_STM32_I2C_CCR_MASK;
        HAL_WRITE_UINT16(base, reg);
    }

    // Set bus for I2C mode without ARP, PEC and SMBUS support
    base = extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1;
    HAL_READ_UINT16(base, reg);
    if (smbus == 1)
    {
        reg |= CYGHWR_HAL_STM32_I2C_CR1_SMBUS; // Enable SMBus mode
        reg |= CYGHWR_HAL_STM32_I2C_CR1_SMBTYPE; // SMBUS Host
    }
    else
    {
        reg &= ~CYGHWR_HAL_STM32_I2C_CR1_SMBUS; // Enable I2C mode
        reg &= ~CYGHWR_HAL_STM32_I2C_CR1_SMBTYPE; // SMBUS Device (ignored)
    }
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_ENARP; // Disable ARP
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_ENPEC; // Disable PEC
    reg |= CYGHWR_HAL_STM32_I2C_CR1_ACK; // Enable ACK
    HAL_WRITE_UINT16(base, reg);

    // Set own address register
    base = extra->i2c_base + CYGHWR_HAL_STM32_I2C_OAR1;
    reg = CYGHWR_HAL_STM32_I2C_OAR1_ADD(bus_addr << 1); // Own Address
    reg &= ~CYGHWR_HAL_STM32_I2C_OAR1_ADDMODE; // 7bit address acked
    HAL_WRITE_UINT16(base, reg);

    // Enable I2C peripheral it is ready to use
    base = extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1;
    HAL_READ_UINT16(base, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR1_PE;
    HAL_WRITE_UINT16(base, reg);

    // Initialize synchronization objects
    cyg_drv_mutex_init(&extra->i2c_lock);
    cyg_drv_cond_init(&extra->i2c_wait, &extra->i2c_lock);

#if CYGPKG_DEVS_I2C_CORTEXM_STM32_DEBUG_LEVEL > 0
    HAL_READ_UINT16(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    debug_printf("I2C: CR1 = 0x%x\n", reg);
    HAL_READ_UINT16(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    debug_printf("I2C: CR2 = 0x%x\n", reg);
    HAL_READ_UINT16(extra->i2c_base + CYGHWR_HAL_STM32_I2C_OAR1, reg);
    debug_printf("I2C: OAR1 = 0x%x\n", reg);
    HAL_READ_UINT16(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CCR, reg);
    debug_printf("I2C: CCR = 0x%x\n", reg);
    HAL_READ_UINT16(extra->i2c_base + CYGHWR_HAL_STM32_I2C_TRISE, reg);
    debug_printf("I2C: TRISE = 0x%x\n", reg);
    HAL_READ_UINT16(extra->i2c_base + CYGHWR_HAL_STM32_I2C_SR1, reg);
    debug_printf("I2C: SR1 = 0x%x\n", reg);
    HAL_READ_UINT16(extra->i2c_base + CYGHWR_HAL_STM32_I2C_SR2, reg);
    debug_printf("I2C: SR2 = 0x%x\n", reg);

    debug_printf("I2C Bus initialized.\n");
#endif
}
