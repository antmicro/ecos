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
#include <pkgconf/devs_i2c_cortexm_stm32f7.h>

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
#if defined(CYGPKG_DEVS_I2C_CORTEXM_STM32_DEBUG_LEVEL_ERROR) || \
    defined(CYGPKG_DEVS_I2C_CORTEXM_STM32_DEBUG_LEVEL_DEBUG)
#define error_printf(args...) diag_printf(args)
#else
#define error_printf(args...)
#endif

#if defined(CYGPKG_DEVS_I2C_CORTEXM_STM32_DEBUG_LEVEL_DEBUG)
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

#ifdef CYGHWR_DEVS_I2C_CORTEXM_STM32_BUS4
CYG_STM32_I2C_BUS(i2c_bus4, CYGHWR_HAL_STM32_I2C4,
        CYGNUM_HAL_INTERRUPT_I2C4_EV, CYGNUM_HAL_INTERRUPT_I2C4_ER);
#endif

//--------------------------------------------------------------------------
// I2C macros for remap pins
#define stm32_i2c_scl( port_pin ) CYGHWR_HAL_STM32_GPIO( port_pin , ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ)
#define stm32_i2c_sda( port_pin ) CYGHWR_HAL_STM32_GPIO( port_pin , ALTFN, 4, OPENDRAIN, PULLUP, 50MHZ)

//--------------------------------------------------------------------------
static void I2C_TransferConfig(CYG_ADDRESS base, cyg_uint16 addr, cyg_uint8 size, cyg_uint32 mode, cyg_uint32 request);
//--------------------------------------------------------------------------
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
/*
TODO: clean-up EV ISR. There is a lot of duplicated code and some unhandled cases
(eg. when we don't generate stop condition)
*/
static cyg_uint32 stm32_i2c_ev_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    static cyg_stm32_i2c_extra* extra = NULL;
    cyg_uint32 regisr, regcr1;
    cyg_uint32 mask, regval;
    cyg_uint32 retval = CYG_ISR_HANDLED;
    error_printf("I2C: %s : %u\r\n", __FUNCTION__, __LINE__);
    extra = (cyg_stm32_i2c_extra*)data;

    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, regcr1);

    mask = (CYGHWR_HAL_STM32_I2C_CR1_TCIE | \
            CYGHWR_HAL_STM32_I2C_CR1_STOPIE | \
            CYGHWR_HAL_STM32_I2C_CR1_NACKIE | \
            CYGHWR_HAL_STM32_I2C_CR1_TXIE);
    /* all the IE flags must be set -> Master Transmit mode */
    if((regcr1 & mask) == mask) {
        HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ISR, regisr);
        if(regisr & CYGHWR_HAL_STM32_I2C_ISR_TXIS)
        {
            /* Write data to TXDR */
            HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_TXDR, *extra->i2c_txbuf++);
            extra->i2c_txleft--;
            extra->i2c_txtotal--;
            retval = CYG_ISR_HANDLED;
        }
        else if(regisr & CYGHWR_HAL_STM32_I2C_ISR_TCR)
        {
            if((extra->i2c_txleft == 0) && (extra->i2c_txtotal != 0))
            {
                if(extra->i2c_txtotal > 255)
                {
                    I2C_TransferConfig(extra->i2c_base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, 0);
                    extra->i2c_txleft = 255;
                }
                else
                {
                    I2C_TransferConfig(extra->i2c_base, extra->i2c_addr, extra->i2c_txtotal, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, 0);
                    extra->i2c_txleft = extra->i2c_txtotal;
                }
                retval = CYG_ISR_HANDLED;
            }
            else
            {
                //TODO: what else?
                retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
            }
        }
        else if(regisr & CYGHWR_HAL_STM32_I2C_ISR_TC)
        {
            if(extra->i2c_txleft == 0)
            {
                if(extra->i2c_stop) {
                    /* Generate Stop */
                    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);
                    regval |= CYGHWR_HAL_STM32_I2C_CR2_STOP;
                    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);
                    retval = CYG_ISR_HANDLED;
                }
                else {
                    //TODO: stop the transfer (do same as in (regisr & CYGHWR_HAL_STM32_I2C_ISR_STOPF))
                    retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
                }
            }
            else
            {
                //TODO: what else?
                retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
            }
        }
        else if(regisr & CYGHWR_HAL_STM32_I2C_ISR_STOPF)
        {
            /* Disable ERR, TC, STOP, NACK, TXI interrupt */
            HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, regval);
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_ERRIE;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_TCIE;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_STOPIE;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_NACKIE;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_TXIE;
            HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, regval);

            /* Clear STOP Flag */
            HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_STOPCF);

            /* Clear Configuration Register 2 */
            HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_SADD;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_HEAD10R;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_NBYTES;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_RELOAD;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_RD_WRN;
            HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);
            retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
        }
        else if(regisr & CYGHWR_HAL_STM32_I2C_ISR_NACKF)
        {
            /* Clear NACK Flag */
            HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_NACKCF);
            retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
        }
        else
        {
            //TODO: what else?
            retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
        }
    }
    mask = (CYGHWR_HAL_STM32_I2C_CR1_TCIE | \
            CYGHWR_HAL_STM32_I2C_CR1_STOPIE | \
            CYGHWR_HAL_STM32_I2C_CR1_NACKIE | \
            CYGHWR_HAL_STM32_I2C_CR1_RXIE);
    if((regcr1 & mask) == mask) {
        HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ISR, regisr);
        if(regisr & CYGHWR_HAL_STM32_I2C_ISR_RXNE)
        {
            /* Read data from RXDR */
            HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_RXDR, *extra->i2c_rxbuf++);
            extra->i2c_rxleft--;
            extra->i2c_rxtotal--;
            retval = CYG_ISR_HANDLED;
        }
        else if(regisr & CYGHWR_HAL_STM32_I2C_ISR_TCR)
        {
            if((extra->i2c_rxleft == 0) && (extra->i2c_rxtotal != 0))
            {
                if(extra->i2c_rxtotal > 255)
                {
                    I2C_TransferConfig(extra->i2c_base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, 0);
                    extra->i2c_rxleft = 255;
                }
                else
                {
                    I2C_TransferConfig(extra->i2c_base, extra->i2c_addr, extra->i2c_rxtotal, CYGHWR_HAL_STM32_I2C_CR2_AUTOEND, 0);
                    extra->i2c_rxleft = extra->i2c_rxtotal;
                }
                retval = CYG_ISR_HANDLED;
            }
            else
            {
                //TODO: what else?
                retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
            }
        }
        else if(regisr & CYGHWR_HAL_STM32_I2C_ISR_TC)
        {
            if(extra->i2c_rxleft == 0)
            {
                if(extra->i2c_stop) {
                    /* Generate Stop */
                    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);
                    regval |= CYGHWR_HAL_STM32_I2C_CR2_STOP;
                    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);
                    retval = CYG_ISR_HANDLED;
                }
                else {
                    // TODO: stop the transfer (do same as in (regisr & CYGHWR_HAL_STM32_I2C_ISR_STOPF))
                    retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
                }
            }
            else
            {
                retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
                //TODO: what else?
            }
        }
        else if(regisr & CYGHWR_HAL_STM32_I2C_ISR_STOPF)
        {
            /* Disable ERR, TC, STOP, NACK, RXI interrupt */
            HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, regval);
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_ERRIE;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_TCIE;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_STOPIE;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_NACKIE;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR1_RXIE;
            HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, regval);

            /* Clear STOP Flag */
            HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_STOPCF);

            /* Clear Configuration Register 2 */
            HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_SADD;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_HEAD10R;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_NBYTES;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_RELOAD;
            regval &= ~CYGHWR_HAL_STM32_I2C_CR2_RD_WRN;
            HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);
            retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
        }
        else if(regisr & CYGHWR_HAL_STM32_I2C_ISR_NACKF)
        {
            /* Clear NACK Flag */
            HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_NACKCF);
            retval = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
        }
    }
    // Acknowledge interrupt
    cyg_drv_interrupt_acknowledge(vec);
    return retval;
}

//--------------------------------------------------------------------------
// The error ISR wakes up the driver in case an error occurred. Both status
// registers are stored in the i2c_status field (high and low word for SR2
// and SR1 respectively) of the driver data.
static cyg_uint32 stm32_i2c_err_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    // Acknowledge interrupt
    cyg_drv_interrupt_acknowledge(vec);
    return CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
}

//--------------------------------------------------------------------------
// The event DSR just wakes up the driver code.
static void stm32_i2c_ev_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)data;
    error_printf("I2C: ev_dsr\n");
    cyg_drv_cond_signal(&extra->i2c_wait);
}

//--------------------------------------------------------------------------
// The error DSR just wakes up the driver code.
static void stm32_i2c_err_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)data;
    error_printf("I2C err_dsr\n");
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
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    cyg_uint32 reg, request;
    error_printf("I2C (%p): tx int started!\n", extra->i2c_base);

    if((tx_data == NULL) || (count == 0)) {
        error_printf("No data to send!!\n");
        return 0;
    }

    extra->i2c_addr  = dev->i2c_address << 1;
    extra->i2c_txtotal = count;
    extra->i2c_txbuf = tx_data;
    extra->i2c_stop = send_stop;

    if(send_start) {
        request = CYGHWR_HAL_STM32_I2C_CR2_START;
    }
    else {
        request = 0;
    }

    if(count > 255) {
        extra->i2c_txleft = 255;
        error_printf("I2C: %s : %u\r\n", __FUNCTION__, __LINE__);
    }
    else {
        extra->i2c_txleft = count;
        error_printf("I2C: %s : %u\r\n", __FUNCTION__, __LINE__);
    }

    /* Send Slave Address */
    /* Set NBYTES to write and reload if size > 255 and generate RESTART */
    if((extra->i2c_txleft == 255) && (extra->i2c_txleft < count)) {
        I2C_TransferConfig(extra->i2c_base, extra->i2c_addr, extra->i2c_txleft, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, request);
        error_printf("I2C: %s : %u\r\n", __FUNCTION__, __LINE__);
    }
    else {
        I2C_TransferConfig(extra->i2c_base, extra->i2c_addr, extra->i2c_txleft, CYGHWR_HAL_STM32_I2C_CR2_AUTOEND, request);
        error_printf("I2C: %s : %u\r\n", __FUNCTION__, __LINE__);
    }

    cyg_drv_mutex_lock(&extra->i2c_lock);
    cyg_drv_dsr_lock();
    cyg_drv_interrupt_unmask(extra->i2c_ev_vec);
    cyg_drv_interrupt_unmask(extra->i2c_err_vec);

    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR1_ERRIE;
    reg |= CYGHWR_HAL_STM32_I2C_CR1_TCIE;
    reg |= CYGHWR_HAL_STM32_I2C_CR1_STOPIE;
    reg |= CYGHWR_HAL_STM32_I2C_CR1_NACKIE;
    reg |= CYGHWR_HAL_STM32_I2C_CR1_TXIE;
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);

    cyg_drv_cond_wait(&extra->i2c_wait);
    error_printf("I2C: %s : %u\r\n", __FUNCTION__, __LINE__);

    cyg_drv_interrupt_mask(extra->i2c_ev_vec);
    cyg_drv_interrupt_mask(extra->i2c_err_vec);
    cyg_drv_dsr_unlock();
    cyg_drv_mutex_unlock(&extra->i2c_lock);

    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_ERRIE;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_TCIE;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_STOPIE;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_NACKIE;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_TXIE;
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);

    extra->i2c_addr  = 0;
    extra->i2c_txtotal = 0;
    extra->i2c_txbuf = NULL;

    error_printf("I2C (%p): tx int finished!\n", extra->i2c_base);
    return count - extra->i2c_rxleft;
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
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    extra->i2c_addr  = (dev->i2c_address << 1) | 1;
    cyg_uint32 reg, request;
    error_printf("I2C (%p): rx int started!\n", extra->i2c_base);

    if((rx_data == NULL) || (count == 0)) {
        error_printf("No data to receive!!\n");
        return 0;
    }
    extra->i2c_rxtotal = count;
    extra->i2c_rxbuf = rx_data;
    extra->i2c_stop = send_stop;
    extra->i2c_rxnak = send_nak;

    if(send_start) {
        request = CYGHWR_HAL_STM32_I2C_CR2_START | CYGHWR_HAL_STM32_I2C_CR2_RD_WRN;
    }
    else {
        request = CYGHWR_HAL_STM32_I2C_CR2_RD_WRN;
    }

    if(count > 255) {
        extra->i2c_rxleft = 255;
    }
    else {
        extra->i2c_rxleft = count;
    }

    /* Send Slave Address */
    /* Set NBYTES to write and reload if size > 255 and generate RESTART */
    if((extra->i2c_rxleft == 255) && (extra->i2c_rxleft < count)) {
        I2C_TransferConfig(extra->i2c_base, extra->i2c_addr, extra->i2c_rxleft, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, request);
    }
    else {
        I2C_TransferConfig(extra->i2c_base, extra->i2c_addr, extra->i2c_rxleft, CYGHWR_HAL_STM32_I2C_CR2_AUTOEND, request);
    }

    cyg_drv_mutex_lock(&extra->i2c_lock);
    cyg_drv_dsr_lock();
    cyg_drv_interrupt_unmask(extra->i2c_ev_vec);
    cyg_drv_interrupt_unmask(extra->i2c_err_vec);

    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg |= CYGHWR_HAL_STM32_I2C_CR1_ERRIE;
    reg |= CYGHWR_HAL_STM32_I2C_CR1_TCIE;
    reg |= CYGHWR_HAL_STM32_I2C_CR1_STOPIE;
    reg |= CYGHWR_HAL_STM32_I2C_CR1_NACKIE;
    reg |= CYGHWR_HAL_STM32_I2C_CR1_RXIE;
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);

    cyg_drv_cond_wait(&extra->i2c_wait);
    debug_printf("I2C: %s : %u\r\n", __FUNCTION__, __LINE__);

    cyg_drv_interrupt_mask(extra->i2c_ev_vec);
    cyg_drv_interrupt_mask(extra->i2c_err_vec);
    cyg_drv_dsr_unlock();
    cyg_drv_mutex_unlock(&extra->i2c_lock);

    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_ERRIE;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_TCIE;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_STOPIE;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_NACKIE;
    reg &= ~CYGHWR_HAL_STM32_I2C_CR1_RXIE;
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR1, reg);

    extra->i2c_addr  = 0;

    error_printf("I2C (%p): rx int finished!\n", extra->i2c_base);
    return count - extra->i2c_rxleft;
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
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint32 reg, request;
    extra->i2c_addr  = dev->i2c_address << 1;
    extra->i2c_txleft = count;
    extra->i2c_txbuf = tx_data;
    cyg_uint32 sizetmp = 0;

    // error_printf("I2C (%p): tx poll started!\n", extra->i2c_base);
    debug_printf("I2C: count = %u\n", count);
    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_ISR, reg);
    if(reg & CYGHWR_HAL_STM32_I2C_ISR_BUSY)
    {
        error_printf("I2C: %s return on %u (reg = 0x%x)\r\n", __FUNCTION__, __LINE__, reg);
        return 0;
    }

    if(send_start) {
        request = CYGHWR_HAL_STM32_I2C_CR2_START;
    }
    else {
        request = 0;
    }

    /* Send Slave Address */
    /* Set NBYTES to write and reload if size > 255 and generate RESTART */
    /* Size > 255, need to set RELOAD bit */
    if(count > 255)
    {
        debug_printf("I2C-tx: count > 255\n");
        I2C_TransferConfig(base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, request);
        sizetmp = 255;
    }
    else
    {
        debug_printf("I2C-tx: count <= 255\n");
        I2C_TransferConfig(base, extra->i2c_addr, count, (send_stop ? CYGHWR_HAL_STM32_I2C_CR2_AUTOEND : 0), request);
        sizetmp = count;
    }

    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    debug_printf("I2C (%u): CR2 = 0x%08x\n", __LINE__, reg);

    do {
        /* Wait until TXIS flag is set */
        if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_TXIS)) {
            debug_printf("I2C-tx: wait for TXIS failed\n");
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
                I2C_TransferConfig(base, extra->i2c_addr, 255, (send_stop ? CYGHWR_HAL_STM32_I2C_CR2_AUTOEND : 0), 0);
                sizetmp = count;
            }
        }
    }while(count > 0);

    if(send_stop)
    {
        /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
        /* Wait until STOPF flag is set */
        if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_STOPF))
        {
            debug_printf("I2C-tx(%u): wait for flag failed!\n", __LINE__);
            return 0;
        }

        /* Clear STOP Flag */
        HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_STOPCF);
    }

    /* Clear Configuration Register 2 */
    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg &= ~(CYGHWR_HAL_STM32_I2C_CR2_SADD |
             CYGHWR_HAL_STM32_I2C_CR2_NBYTES |
             CYGHWR_HAL_STM32_I2C_CR2_RELOAD |
             CYGHWR_HAL_STM32_I2C_CR2_HEAD10R |
             CYGHWR_HAL_STM32_I2C_CR2_RD_WRN);
    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    count = extra->i2c_txleft;
    extra->i2c_txleft = 0;

    // error_printf("I2C (%p): tx poll finished!\n", extra->i2c_base);
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
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    CYG_ADDRESS base = extra->i2c_base;
    cyg_uint32 reg, request;
    extra->i2c_addr  = (dev->i2c_address << 1) | 0x01;
    extra->i2c_rxleft = count;
    extra->i2c_rxbuf = rx_data;
    extra->i2c_rxnak = send_nak;
    cyg_uint32 sizetmp = 0;

    if(count == 0)
        return 0;

    debug_printf("I2C RX(%u): count: %u addr: 0x%02x\n", __LINE__, count, extra->i2c_addr);

    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_ISR, reg);
    if(reg & CYGHWR_HAL_STM32_I2C_ISR_BUSY)
    {
        error_printf("I2C: %s return on %u (reg = 0x%x)\r\n", __FUNCTION__, __LINE__, reg);
        return 0;
    }

    if(send_start) {
        request = CYGHWR_HAL_STM32_I2C_CR2_START | CYGHWR_HAL_STM32_I2C_CR2_RD_WRN;
    }
    else {
        request = CYGHWR_HAL_STM32_I2C_CR2_RD_WRN;
    }

    if(count > 255)
    {
        I2C_TransferConfig(base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, request);
        sizetmp = 255;
    }
    else
    {
        I2C_TransferConfig(base, extra->i2c_addr, count, (send_stop ? CYGHWR_HAL_STM32_I2C_CR2_AUTOEND : 0), request);
        sizetmp = count;
    }

    do {
        /* Wait until RXNE flag is set */
        if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_RXNE)) {
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
                return 0;
            }

            if(count > 255)
            {
                I2C_TransferConfig(base, extra->i2c_addr, 255, CYGHWR_HAL_STM32_I2C_CR2_RELOAD, 0);
                sizetmp = 255;
            }
            else
            {
                I2C_TransferConfig(base, extra->i2c_addr, 255, (send_stop ? CYGHWR_HAL_STM32_I2C_CR2_AUTOEND : 0), 0);
                sizetmp = count;
            }
        }
    }while(count > 0);

    if(send_stop) {
        /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
        /* Wait until STOPF flag is set */
        if(!i2c_wait_for_flag_set(base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_STOPF)) {
            return 0;
        }

        /* Clear STOP Flag */
        HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_STOPCF);
    }

    /* Clear Configuration Register 2 */
    HAL_READ_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);
    reg &= ~(CYGHWR_HAL_STM32_I2C_CR2_SADD |
             CYGHWR_HAL_STM32_I2C_CR2_NBYTES |
             CYGHWR_HAL_STM32_I2C_CR2_RELOAD |
             CYGHWR_HAL_STM32_I2C_CR2_HEAD10R |
             CYGHWR_HAL_STM32_I2C_CR2_RD_WRN);
    HAL_WRITE_UINT32(base + CYGHWR_HAL_STM32_I2C_CR2, reg);

    /* Process Unlocked */
    return (extra->i2c_rxleft - count);
}

//--------------------------------------------------------------------------
//  Generate a STOP condtition
void cyg_stm32_i2c_stop(const cyg_i2c_device *dev)
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)dev->i2c_bus->i2c_extra;
    cyg_uint32 regval;

    /* Generate Stop */
    HAL_READ_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);
    regval |= CYGHWR_HAL_STM32_I2C_CR2_STOP;
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_CR2, regval);

    /* Wait until STOPF flag is set */
    if(!i2c_wait_for_flag_set(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ISR, CYGHWR_HAL_STM32_I2C_ISR_STOPF)) {
        return;
    }

    /* Clear STOP Flag */
    HAL_WRITE_UINT32(extra->i2c_base + CYGHWR_HAL_STM32_I2C_ICR, CYGHWR_HAL_STM32_I2C_ICR_STOPCF);

    debug_printf("I2C: Stop condition generated.\n");
}

//--------------------------------------------------------------------------
// This function is called from the generic I2C infrastructure to initialize
// a bus device. It should perform any work needed to start up the device.
void cyg_stm32_i2c_init(struct cyg_i2c_bus *bus)
{
    cyg_stm32_i2c_extra* extra = (cyg_stm32_i2c_extra*)bus->i2c_extra;
    cyg_uint32 pinspec_scl, pinspec_sda; // Pin configurations
    cyg_uint32 reg;

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

#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS1_MODE_POLL
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
#else
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
        // TODO: add clock definition to extra and use enable it in more general way
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, reg);
        reg |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, reg);

        // Reset peripheral
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
        reg |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
        reg &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C1);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
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

#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS2_MODE_POLL
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
#else
        // Initialize i2c event interrupt
        cyg_drv_interrupt_create(extra->i2c_ev_vec,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS2_MODE_INT_EV_PRI,
                (cyg_addrword_t) extra,
                &stm32_i2c_ev_isr,
                &stm32_i2c_ev_dsr,
                &(extra->i2c_ev_interrupt_handle),
                &(extra->i2c_ev_interrupt_data));
        cyg_drv_interrupt_attach(extra->i2c_ev_interrupt_handle);

        // Initialize i2c error interrupt
        cyg_drv_interrupt_create(extra->i2c_err_vec,
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
        // TODO: add clock definition to extra and use enable it in more general way
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, reg);
        reg |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, reg);

        // Reset peripheral
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
        reg |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
        reg &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C2);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
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

#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS3_MODE_POLL
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
#else
        // Initialize i2c event interrupt
        cyg_drv_interrupt_create(extra->i2c_ev_vec,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS3_MODE_INT_EV_PRI,
                (cyg_addrword_t) extra,
                &stm32_i2c_ev_isr,
                &stm32_i2c_ev_dsr,
                &(extra->i2c_ev_interrupt_handle),
                &(extra->i2c_ev_interrupt_data));
        cyg_drv_interrupt_attach(extra->i2c_ev_interrupt_handle);

        // Initialize i2c error interrupt
        cyg_drv_interrupt_create(extra->i2c_err_vec,
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
        // TODO: add clock definition to extra and use enable it in more general way
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, reg);
        reg |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C3);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, reg);

        // Reset peripheral
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
        reg |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C3);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
        reg &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C3);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
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

#ifdef CYGSEM_DEVS_I2C_CORTEXM_STM32_BUS4_MODE_POLL
        bus->i2c_tx_fn = &cyg_stm32_i2c_tx_poll;
        bus->i2c_rx_fn = &cyg_stm32_i2c_rx_poll;
#else
        // Initialize i2c event interrupt
        cyg_drv_interrupt_create(extra->i2c_ev_vec,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS4_MODE_INT_EV_PRI,
                (cyg_addrword_t) extra,
                &stm32_i2c_ev_isr,
                &stm32_i2c_ev_dsr,
                &(extra->i2c_ev_interrupt_handle),
                &(extra->i2c_ev_interrupt_data));
        cyg_drv_interrupt_attach(extra->i2c_ev_interrupt_handle);

        // Initialize i2c error interrupt
        cyg_drv_interrupt_create(extra->i2c_err_vec,
                CYGINT_DEVS_I2C_CORTEXM_STM32_BUS4_MODE_INT_EE_PRI,
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
        // TODO: add clock definition to extra and use enable it in more general way
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, reg);
        reg |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C4);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1ENR, reg);

        // Reset peripheral
        HAL_READ_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
        reg |= BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C4);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
        reg &= ~BIT_(CYGHWR_HAL_STM32_RCC_APB1ENR_I2C4);
        HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB1RSTR, reg);
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
