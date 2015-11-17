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
   #define EVLOG(__x)
#else
   #define error_printf(args...)
   #define EVLOG(__x)
#endif
#if CYGPKG_DEVS_I2C_CORTEXM_STM32_DEBUG_LEVEL > 1
   #define debug_printf(args...) diag_printf(args)
   #define EVLOG(__x) (extra->i2c_evlog[extra->i2c_evlogidx++]=(__x))
#else
   #define debug_printf(args...)
   #define EVLOG(__x)
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

#ifdef CYGHWR_DEVS_I2C_CORTEXM_STM32_BUS4
CYG_STM32_I2C_BUS(i2c_bus3, CYGHWR_HAL_STM32_I2C4,
        CYGNUM_HAL_INTERRUPT_I2C4_EV, CYGNUM_HAL_INTERRUPT_I2C4_ER);
#endif

//--------------------------------------------------------------------------
// I2C macros for remap pins
#define stm32_i2c_scl( port_pin ) CYGHWR_HAL_STM32_GPIO( port_pin , ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ)
#define stm32_i2c_sda( port_pin ) CYGHWR_HAL_STM32_GPIO( port_pin , ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ)

//--------------------------------------------------------------------------
inline static cyg_bool i2c_wait_for_txis(CYG_ADDRESS base)
{
    volatile cyg_uint32 isr = 0, tmo = 0;
    do {
        HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_ISR, isr);
        tmo++;
        if(tmo > 0x000FFFFF) {
            error_printf("I2C: %s(%u) ISR = 0x%08x\n", __FUNCTION__, __LINE__, isr);
            return false;
        }
    } while(isr & CYGHWR_HAL_STM32_I2C_ISR_TXIS);
    return true;
}

static cyg_bool i2c_wait_for_flag_set(CYG_ADDRESS addr, cyg_uint32 flag_mask)
{
    volatile cyg_uint32 reg = 0, tmo = 0;
    do {
        HAL_READ_UINT32(addr, reg);
        tmo++;
        if(tmo >= 0x00FFFFFF)
        {
            error_printf("I2C: %s(%u) (%p) = 0x%08x (mask 0x%08x)\r\n", __FUNCTION__, __LINE__, (void*)addr, reg, flag_mask);
            return false;
        }
    }while((reg & flag_mask) == 0);

    debug_printf("I2C: %s(%u) (%p) = 0x%08x (mask 0x%08x)\r\n", __FUNCTION__, __LINE__, (void*)addr, reg, flag_mask);
    return true;
}

static cyg_bool i2c_wait_for_flag_reset(CYG_ADDRESS addr, cyg_uint32 flag_mask)
{
    volatile cyg_uint32 reg = 0, tmo = 0;
    do {
        HAL_READ_UINT32(addr, reg);
        tmo++;
        if(tmo >= 0x000FFFFF)
        {
            error_printf("I2C: %s(%u) (%p) = 0x%08x (mask 0x%08x)\r\n", __FUNCTION__, __LINE__, (void*)addr, reg, flag_mask);
            return false;
        }
    }while((reg & flag_mask) != 0);

    debug_printf("I2C: %s(%u) (%p) = 0x%08x (mask 0x%08x)\r\n", __FUNCTION__, __LINE__, (void*)addr, reg, flag_mask);
    return true;
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
    // static cyg_stm32_i2c_extra* extra = NULL;
    // static CYG_ADDRESS base = 0;
    // cyg_bool call_dsr = false;
    // cyg_uint32 tmp1 = 0, tmp2 = 0, tmp3 = 0, tmp4 = 0;
    // cyg_uint16 reg;

    // extra = (cyg_stm32_i2c_extra*)data;
    // base = extra->i2c_base;

    return CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
}

//--------------------------------------------------------------------------
// The error ISR wakes up the driver in case an error occurred. Both status
// registers are stored in the i2c_status field (high and low word for SR2
// and SR1 respectively) of the driver data.
static cyg_uint32 stm32_i2c_err_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    static cyg_stm32_i2c_extra* extra = NULL;
    CYG_ADDRESS base;
    cyg_uint32 isr;

    error_printf("I2C: %s : %u\r\n", __FUNCTION__, __LINE__);
    extra = (cyg_stm32_i2c_extra*)data;
    base = extra->i2c_base;
    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_ISR, isr);
    extra->i2c_status = isr;
    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_ICR, 0xffff);
    // Stop the transfer
    if((isr & CYGHWR_HAL_STM32_I2C_ISR_BUSY) > 0)
    {
        HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, isr);
        isr |= CYGHWR_HAL_STM32_I2C_CR2_STOP;
        HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, isr);
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
    debug_printf("EV dsr!\n");
}

//--------------------------------------------------------------------------
// The error DSR just wakes up the driver code.
static void stm32_i2c_err_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)data;
    error_printf("I2C error: 0x%x\n", extra->i2c_status);
    cyg_drv_cond_signal(&extra->i2c_wait);
    debug_printf("ERR dsr!\n");
}

//--------------------------------------------------------------------------
// Transmits a buffer to a device in interrupt mode
cyg_uint32 cyg_stm32_i2c_tx_int(const cyg_i2c_device *dev,
                            cyg_bool              send_start,
                            const cyg_uint8      *tx_data,
                            cyg_uint32            count,
                            cyg_bool              send_stop)
{
    // cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    // CYG_ADDRESS base = extra->i2c_base;
    // cyg_uint16 reg;
    // extra->i2c_addr  = dev->i2c_address << 1;
    // extra->i2c_txleft = count;
    // extra->i2c_txbuf = tx_data;
    // extra->i2c_stop = send_stop;

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
    // cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    // CYG_ADDRESS base = extra->i2c_base;
    // cyg_uint16 reg;
    // extra->i2c_addr  = (dev->i2c_address << 1) | 1;
    // extra->i2c_rxleft = count;
    // extra->i2c_rxbuf = rx_data;
    // extra->i2c_rxnak = send_nak;
    // extra->i2c_stop = send_stop;

    return count;
}

static void I2C_TransferConfig(CYG_ADDRESS base, cyg_uint16 addr, cyg_uint8 size, cyg_uint32 mode, cyg_uint32 request)
{
    cyg_uint32 reg = 0;

    /* Get the CR2 register value */
    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    /* clear tmpreg specific bits */
    reg &= ~(CYGHWR_HAL_STM32_I2C_CR2_SADD |
             CYGHWR_HAL_STM32_I2C_CR2_NBYTES |
             CYGHWR_HAL_STM32_I2C_CR2_RELOAD |
             CYGHWR_HAL_STM32_I2C_CR2_AUTOEND |
             CYGHWR_HAL_STM32_I2C_CR2_RD_WRN |
             CYGHWR_HAL_STM32_I2C_CR2_START | 
             CYGHWR_HAL_STM32_I2C_CR2_STOP);

    /* update tmpreg */
    reg |= ((addr & CYGHWR_HAL_STM32_I2C_CR2_SADD) |
            ((size << 16) & CYGHWR_HAL_STM32_I2C_CR2_NBYTES) |
            mode |
            request);

    /* update CR2 register */
    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
}

//--------------------------------------------------------------------------
// Transmit a buffer to a device in polling mode
cyg_uint32 cyg_stm32_i2c_tx_poll(const cyg_i2c_device *dev,
                            cyg_bool              send_start,
                            const cyg_uint8      *tx_data,
                            cyg_uint32            count,
                            cyg_bool              send_stop)
/* KNOWN ISSUES: */
/* Currently the send_stop and send_start are ignored */
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint16 reg;
    extra->i2c_addr  = dev->i2c_address << 1;
    extra->i2c_txleft = count;
    extra->i2c_txbuf = tx_data;
    cyg_uint32 sizetmp = 0;
    cyg_uint32 start = 0;
    cyg_uint32 stop = 0;

    cyg_drv_dsr_lock();
    debug_printf("I2C: count = %u\n", count);
    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_ISR, reg);
    if(reg & CYGHWR_HAL_STM32_I2C_ISR_BUSY)
    {
        cyg_drv_dsr_unlock();
        error_printf("I2C: %s return on %u (reg = 0x%x)\r\n", __FUNCTION__, __LINE__, reg);
        return 0;
    }

    // extra->i2c_state = I2C_STATE_BUSY;

    /* Send Slave Address */
    /* Set NBYTES to write and reload if size > 255 and generate RESTART */
    /* Size > 255, need to set RELOAD bit */
    if(count > 255)
    {
        debug_printf("I2C-tx: count > 255\n");
        I2C_TransferConfig(base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, CYGHWR_HAL_STM32_I2C_CR2_START);
        sizetmp = 255;
    }
    else
    {
        debug_printf("I2C-tx: count <= 255\n");
        I2C_TransferConfig(base, extra->i2c_addr, count, CYGHWR_HAL_STM32_I2C_CR2_AUTOEND, CYGHWR_HAL_STM32_I2C_CR2_START);
        sizetmp = count;
    }

    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    debug_printf("I2C (%u): CR2 = 0x%08x\n", __LINE__, reg);
    
    do {
        /* Wait until TXIS flag is set */
        if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_TXIS)) {
            debug_printf("I2C-tx: wait for TXIS failed\n");
            cyg_drv_dsr_unlock();
            return 0;
        }

        /* Write data to TXDR */
        HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_TXDR, *extra->i2c_txbuf++);
        // debug_printf("I2C-tx(%u): data written\n", __LINE__);
        sizetmp--;
        count--;

        if((sizetmp == 0)&&(count!=0))
        {
            /* Wait until TXE flag is set */
            if(!i2c_wait_for_flag_reset(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_TCR))
            {
                debug_printf("I2C-tx: wait for TXIS failed\n");
                cyg_drv_dsr_unlock();
                return 0;
            }

            if(count > 255)
            {
                debug_printf("I2C-tx(%u): count > 255\n", __LINE__);
                I2C_TransferConfig(base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, 0);
                sizetmp = 255;
            }
            else
            {
                debug_printf("I2C-tx(%u): count <= 255\n", __LINE__);
                I2C_TransferConfig(base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_AUTOEND, 0);
                sizetmp = count;
            }
        }
    }while(count > 0);

    /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is set */
    if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_STOPF))
    {
        debug_printf("I2C-tx(%u): wait for flag failed!\n", __LINE__);
        cyg_drv_dsr_unlock();
        return 0;
    }

    /* Clear STOP Flag */
    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_STOPCF);

    /* Clear Configuration Register 2 */
    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg &= ~(CYGHWR_HAL_STM32_I2C_CR2_SADD |
             CYGHWR_HAL_STM32_I2C_CR2_NBYTES |
             CYGHWR_HAL_STM32_I2C_CR2_RELOAD |
             CYGHWR_HAL_STM32_I2C_CR2_HEAD10R |
             CYGHWR_HAL_STM32_I2C_CR2_RD_WRN);
    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    /* Process Unlocked */
    cyg_drv_dsr_unlock();

    count = extra->i2c_txleft;
    extra->i2c_txleft = 0;

    return count;
}

//--------------------------------------------------------------------------
// Receive into a buffer from a device in polling mode
//
cyg_uint32 cyg_stm32_i2c_rx_poll(const cyg_i2c_device *dev,
                            cyg_bool              send_start,
                            cyg_uint8            *rx_data,
                            cyg_uint32            count,
                            cyg_bool              send_nak,
                            cyg_bool              send_stop)
/* KNOWN ISSUES: */
/* Currently the send_stop, send_start and send_nak are ignored */
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint16 reg;
    extra->i2c_addr  = (dev->i2c_address << 1) | 0x01;
    extra->i2c_rxleft = count;
    extra->i2c_rxbuf = rx_data;
    extra->i2c_rxnak = send_nak;
    cyg_uint32 stop = 0;
    cyg_uint32 sizetmp = 0;

    if(count == 0)
        return 0;

    cyg_drv_dsr_lock();
    debug_printf("I2C RX(%u): count: %u addr: 0x%02x\n", __LINE__, count, extra->i2c_addr);

    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_ISR, reg);
    if(reg & CYGHWR_HAL_STM32_I2C_ISR_BUSY)
    {
        cyg_drv_dsr_unlock();
        error_printf("I2C: %s return on %u (reg = 0x%x)\r\n", __FUNCTION__, __LINE__, reg);
        return 0;
    }

    if(count > 255)
    {
        I2C_TransferConfig(base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, (CYGHWR_HAL_STM32_I2C_CR2_START | CYGHWR_HAL_STM32_I2C_CR2_RD_WRN));
        sizetmp = 255;
    }
    else
    {
        I2C_TransferConfig(base, extra->i2c_addr, count, CYGHWR_HAL_STM32_I2C_CR2_AUTOEND, (CYGHWR_HAL_STM32_I2C_CR2_START | CYGHWR_HAL_STM32_I2C_CR2_RD_WRN));
        sizetmp = count;
    }

    do {
        /* Wait until RXNE flag is set */
        if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_RXNE)) {
            cyg_drv_dsr_unlock();
            return 0;
        }
        /* read data from RXDR */
        HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_RXDR, *extra->i2c_rxbuf++);
        sizetmp--;
        count--;

        if((sizetmp == 0)&&(count!=0))
        {
            /* Wait until TCR flag is set */
            if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_TCR))
            {
                cyg_drv_dsr_unlock();
                return 0;
            }

            if(count > 255)
            {
                I2C_TransferConfig(base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, 0);
                sizetmp = 255;
            }
            else
            {
                I2C_TransferConfig(base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_AUTOEND, 0);
                sizetmp = count;
            }
        }
    }while(count > 0);

    /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is set */
    if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_STOPF)) {
        return 0;
    }

    /* Clear STOP Flag */
    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_STOPCF);
    
    /* Clear Configuration Register 2 */
    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg &= ~(CYGHWR_HAL_STM32_I2C_CR2_SADD |
             CYGHWR_HAL_STM32_I2C_CR2_NBYTES |
             CYGHWR_HAL_STM32_I2C_CR2_RELOAD |
             CYGHWR_HAL_STM32_I2C_CR2_HEAD10R |
             CYGHWR_HAL_STM32_I2C_CR2_RD_WRN);
    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    /* Process Unlocked */
    cyg_drv_dsr_unlock();

    return (extra->i2c_rxleft - count);
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
    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR2_STOP;
    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    // Make sure that the STOP bit is cleared by hardware
    if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_STOPF)) {
        error_printf("I2C: Stop condition not generated!\n");
        return;
    }

    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_STOPCF);
    
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
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint16 tmp;
    cyg_uint32 reg;
    cyg_uint32 rcc;

    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg &= ~(CYGHWR_HAL_STM32_I2C_CR1_PE);
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);

    if(extra->i2c_base == CYGHWR_HAL_STM32_I2C1)
    {
#ifdef CYGHWR_HAL_STM32_I2C1_REMAP
        pinspec_scl = stm32_i2c_scl( CYGHWR_HAL_STM32_I2C1_SCL );
        pinspec_sda = stm32_i2c_sda( CYGHWR_HAL_STM32_I2C1_SDA );
#else
        pinspec_scl = CYGHWR_HAL_STM32_GPIO(H, 7, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
        pinspec_sda = CYGHWR_HAL_STM32_GPIO(H, 8, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
#endif
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;

        // Enable peripheral clock
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);

        // Reset peripheral
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
    }
    else if(extra->i2c_base == CYGHWR_HAL_STM32_I2C2)
    {
#ifdef CYGHWR_HAL_STM32_I2C2_REMAP
        pinspec_scl = stm32_i2c_scl( CYGHWR_HAL_STM32_I2C2_SCL );
        pinspec_sda = stm32_i2c_sda( CYGHWR_HAL_STM32_I2C2_SDA );
#else
        pinspec_scl = CYGHWR_HAL_STM32_GPIO(B, 10, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
        pinspec_sda = CYGHWR_HAL_STM32_GPIO(B, 11, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
#endif

        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
        // Enable peripheral clock
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);

        // Reset peripheral
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
    }
    else if(extra->i2c_base == CYGHWR_HAL_STM32_I2C3)
    {
#ifdef CYGHWR_HAL_STM32_I2C3_REMAP
        pinspec_scl = stm32_i2c_scl( CYGHWR_HAL_STM32_I2C3_SCL );
        pinspec_sda = stm32_i2c_sda( CYGHWR_HAL_STM32_I2C3_SDA );
#else
        pinspec_scl = CYGHWR_HAL_STM32_GPIO(H, 7, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
        pinspec_sda = CYGHWR_HAL_STM32_GPIO(H, 8, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
#endif

        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
        // Enable peripheral clock
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C3);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);

        // Reset peripheral
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C3);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C3);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
    }
    else if(extra->i2c_base == CYGHWR_HAL_STM32_I2C4)
    {
#ifdef CYGHWR_HAL_STM32_I2C4_REMAP
        pinspec_scl = stm32_i2c_scl( CYGHWR_HAL_STM32_I2C4_SCL );
        pinspec_sda = stm32_i2c_sda( CYGHWR_HAL_STM32_I2C4_SDA );
#else
        pinspec_scl = CYGHWR_HAL_STM32_GPIO(D, 12, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
        pinspec_sda = CYGHWR_HAL_STM32_GPIO(D, 13, ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ);
#endif

        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
        // Enable peripheral clock
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C4);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, rcc);

        // Reset peripheral
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C4);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
        rcc &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C4);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, rcc);
    }

    CYGHWR_HAL_STM32_GPIO_SET(pinspec_scl);
    CYGHWR_HAL_STM32_GPIO_SET(pinspec_sda);

    /*---------------------------- I2Cx TIMINGR Configuration ------------------*/
    /* Configure I2Cx: Frequency range */
    // value comes from F7Cube, no explanation why is that, currently hardcoded to 100 kHz
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_TIMINGR, 0x40912732);

    /*---------------------------- I2Cx OAR1 Configuration ---------------------*/
    /* Configure I2Cx: Own Address1 and ack own address1 mode */
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_OAR1, reg);
    reg &= ~CYGHWR_HAL_STM32_I2C_OAR1_OA1EN;
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_OAR1, reg);

    /*---------------------------- I2Cx CR2 Configuration ----------------------*/
    /* Enable the AUTOEND by default, and enable NACK (should be disable only during Slave process */
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR2_AUTOEND;
    reg |= CYGHWR_HAL_STM32_I2C_CR2_NACK;
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    /*---------------------------- I2Cx OAR2 Configuration ---------------------*/
    /* Configure I2Cx: Dual mode and Own Address2 */
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_OAR2, reg);
    reg = 0; // &= ~CYGHWR_HAL_STM32_I2C_OAR2_OA2EN;
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_OAR2, reg);
    
    /*---------------------------- I2Cx CR1 Configuration ----------------------*/
    /* Configure I2Cx: Generalcall and NoStretch mode */
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg &= ~(CYGHWR_HAL_STM32_I2C_CR1_NOSTRETCH | CYGHWR_HAL_STM32_I2C_CR1_GCEN);
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);

    /* Enable the selected I2C peripheral */
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR1_PE;
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);

    // Initialize synchronization objects
    cyg_drv_mutex_init(&extra->i2c_lock);
    cyg_drv_cond_init(&extra->i2c_wait, &extra->i2c_lock);

    // Initialize state field
    extra->i2c_state = I2C_STATE_IDLE;

#if CYGPKG_DEVS_I2C_CORTEXM_STM32_DEBUG_LEVEL > 0
    debug_printf("I2C: bus @ 0x%08x\n", extra->i2c_base);

    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    debug_printf("I2C: CR1 = 0x%08x\n", reg);
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    debug_printf("I2C: CR2 = 0x%08x\n", reg);
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_OAR1, reg);
    debug_printf("I2C: OAR1 = 0x%08x\n", reg);
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_OAR2, reg);
    debug_printf("I2C: OAR2 = 0x%08x\n", reg);
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_TIMINGR, reg);
    debug_printf("I2C: TIMINGR = 0x%08x\n", reg);
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ISR, reg);
    debug_printf("I2C: ISR = 0x%08x\n", reg);
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ICR, reg);
    debug_printf("I2C: ICR = 0x%08x\n", reg);

    debug_printf("I2C Bus initialized.\n");
#endif
}
