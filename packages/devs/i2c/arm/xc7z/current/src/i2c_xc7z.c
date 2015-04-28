//==========================================================================
//
//      i2c_xc7z.c
//
//      I2C driver for Xilinx Zynq
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Ant Micro <www.antmicro.com>
// Date:         2012-07-23
// Purpose:
// Description:  I2C driver for Xilinx Zynq
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/devs_i2c_arm_xc7z.h>
#include <pkgconf/infra.h>

#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/cyg_trac.h>
#include <cyg/infra/diag.h>

#include <cyg/io/i2c.h>
#include <cyg/io/i2c_xc7z.h>

#include <cyg/hal/hal_io.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/xc7z020.h>
#include <cyg/hal/var_io.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>

/*
 * Configuration of the base addres of the registers
 */
#if CYGPKG_DEVS_I2C_ARM_XC7Z_INTERFACE == 0
    #define XI2CPS_BASE 0xE0004000
    #define XI2CPS_ISR_VECT CYGNUM_HAL_INTERRUPT_I2C0
#elif CYGPKG_DEVS_I2C_ARM_XC7Z_INTERFACE == 1
    #define XI2CPS_BASE 0xE0005000
    #define XI2CPS_ISR_VECT CYGNUM_HAL_INTERRUPT_I2C1
#else
    #error Bad interface!
#endif

/*
 * I2C Registers (offsets)
 */
#define XI2CPS_CR_OFFSET        0x00 /* Control Register, RW, 16b */
#define XI2CPS_SR_OFFSET        0x04 /* Status Register, RO, 16b */
#define XI2CPS_ADDR_OFFSET      0x08 /* I2C Address Register, RW, 16b */
#define XI2CPS_DATA_OFFSET      0x0C /* I2C Data Register, RW, 16b */
#define XI2CPS_ISR_OFFSET       0x10 /* Interrupt Status Register, RW, 16b */
#define XI2CPS_XFER_SIZE_OFFSET 0x14 /* Transfer Size Register, RW, 8b */
#define XI2CPS_SLV_PAUSE_OFFSET 0x18 /* Slave monitor pause Register, RW, 8b */
#define XI2CPS_TIME_OUT_OFFSET  0x1C /* Time Out Register, RW, 8b */
#define XI2CPS_IMR_OFFSET       0x20 /* Interrupt Mask Register, RO, 16b */
#define XI2CPS_IER_OFFSET       0x24 /* Interrupt Enable Register, WO, 16b */
#define XI2CPS_IDR_OFFSET       0x28 /* Interrupt Disable Register, WO, 16b */

/*
 * Control Register Bit mask definitions
 * This register contains various control bits that affect the operation of the
 * I2C controller.
 */
#define XI2CPS_CR_HOLD_BUS_MASK 0x00000010 /* Hold Bus bit */
#define XI2CPS_CR_RW_MASK       0x00000001 /* Read or Write Master transfer
                                            * 0= Transmitter, 1= Receiver */
#define XI2CPS_CR_MS_MASK       0x00000002 /* Master=1/Slave=0 */
#define XI2CPS_CR_CLR_FIFO_MASK 0x00000040 /* 1 = Auto init FIFO to zeroes */

/*
 * Control Register Bits
 */
#define CR_CLR_FIFO     6 /* 1=Clear FIFO */
#define CR_SLVMON       5 /* 1=Monitor mode */
#define CR_HOLD         4 /* 1=hold SCL low after all operations */
#define CR_ACKEN        3 /* 1=ACK enabled, must always be set */
#define CR_NEA          2 /* 1=Normal address (7 bit) 0=Reserved */
#define CR_MS           1 /* 1=Master 0=Slave */
#define CR_RW           0 /* 0=Master Transmitter 1=Master Receiver */

/*
 * I2C Address Register Bit mask definitions
 * Normal addressing mode uses [6:0] bits. Extended addressing mode uses [9:0]
 * bits. A write access to this register always initiates a transfer if the I2C
 * is in master mode.
 */
#define XI2CPS_ADDR_MASK        0x000003FF /* I2C Address Mask */


/*
 * I2C Interrupt Status Register Bits definitions
 */
#define XI2CPS_ISR_COMP         0x0001
#define XI2CPS_ISR_DATA         0x0002
#define XI2CPS_ISR_NACK         0x0004
#define XI2CPS_ISR_TO           0x0008
#define XI2CPS_ISR_SLV_RDY      0x0010
#define XI2CPS_ISR_RX_OVF       0x0020
#define XI2CPS_ISR_TX_OVF       0x0040
#define XI2CPS_ISR_RX_UNF       0x0080
#define XI2CPS_ISR_ARB_LOST     0x0200

/*
 * I2C Status Register Bits definitions
 */
#define XI2CPS_SR_RXRW  0x0008
#define XI2CPS_SR_RXDV  0x0020
#define XI2CPS_SR_TXDV  0x0040
#define XI2CPS_SR_RXOVF 0x0080
#define XI2CPS_SR_BA    0x0100

/*
 * I2C Interrupt Registers Bit mask definitions
 * All the four interrupt registers (Status/Mask/Enable/Disable) have the same
 * bit definitions.
 */
#define XI2CPS_IXR_ALL_INTR_MASK 0x000002FF /* All ISR Mask */
#define XI2CPS_FIFO_DEPTH       16              /* FIFO Depth */
#define XI2CPS_TIMEOUT          (50 * HZ)       /* Timeout for bus busy check */
#define XI2CPS_ENABLED_INTR     0x2EF           /* Enabled Interrupts */

#define XI2CPS_DATA_INTR_DEPTH (XI2CPS_FIFO_DEPTH - 2)/* FIFO depth at which
                                                         * the DATA interrupt
                                                         * occurs
                                                         */


#define I2C_FLAG_FINISH  1       // transfer finished                       
#define I2C_FLAG_ACT     2       // bus still active, no STOP condition send
#define I2C_FLAG_ERROR  (1<<31)  // one of the following errors occured:    
#define I2C_FLAG_ADDR   (1<<30)  // - address was not ACKed                 
#define I2C_FLAG_DATA   (1<<29)  // - data was not ACKed                    
#define I2C_FLAG_LOST   (1<<28)  // - bus arbitration was lost              
#define I2C_FLAG_BUF    (1<<27)  // - no buffer for reading or writing      
#define I2C_FLAG_UNK    (1<<26)  // - unknown I2C status                   
#define I2C_FLAG_BUS    (1<<25)  // - bus error

static void zynq_i2c_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data);
static cyg_uint32 zynq_i2c_isr(cyg_vector_t vec, cyg_addrword_t data);
void zynq_i2c_feed_data(cyg_zynq_i2c_extra* ex);


#define ZYNQ_I2C_DEBUG
#define DPRINTF(s) { diag_printf("[I2C driver function \"%s\", line %4d] %s", __FUNCTION__, __LINE__,s); } ;

/*
 * Initialize driver and hardware
 */
void zynq_i2c_init(struct cyg_i2c_bus* bus)
{
    cyg_zynq_i2c_extra* extra = (cyg_zynq_i2c_extra*)bus->i2c_extra;
    cyg_uint16 div_a, div_b;
    cyg_uint16 temp_div_a, temp_div_b;
    cyg_uint32 error, last_error;
    cyg_uint32 temp;
    cyg_uint16 ctrl_reg;

    extra->i2c_isr_vector = XI2CPS_ISR_VECT;
    extra->i2c_isr_priority = 0;
    extra->i2c_hold_flag = 0;
    
    // register ISR
    cyg_drv_mutex_init(&extra->i2c_lock);
    cyg_drv_cond_init(&extra->i2c_wait, &extra->i2c_lock);
    cyg_drv_interrupt_create(extra->i2c_isr_vector,
                             extra->i2c_isr_priority,
                             (cyg_addrword_t) extra,
                             &zynq_i2c_isr,
                             &zynq_i2c_dsr,
                             &(extra->i2c_int_handle),
                             &(extra->i2c_int_data));
    cyg_drv_interrupt_attach(extra->i2c_int_handle);
    
    // Calculate values of diva and divb based o the PCLK and FSCL
    for(temp_div_a = 0; temp_div_a < 4; temp_div_a++)
    {
        //calculate div_b for assumed div_a
        temp = 22 * extra->i2c_bus_freq * (temp_div_a + 1);
        temp_div_b = (CYGPKG_DEVS_I2C_ARM_XC7Z_PCLK / temp) - 1;
        //error = assumed_fscl - calculated_fscl
        error = extra->i2c_bus_freq - (CYGPKG_DEVS_I2C_ARM_XC7Z_PCLK / (22 * (temp_div_a + 1) * (temp_div_b + 1)));
        //don't compare errors for first loop iteration
        if(temp_div_a == 0)
        {
            last_error = error;
            div_a = temp_div_a;
            div_b = temp_div_b;
        }
        else if(last_error < error) //save the best divisors values
        {
            div_a = temp_div_a;
            div_b = temp_div_b;
        }
    }

#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("divisors calculated: ");
    diag_printf("div_a = %d,  div_b = %d\n", div_a, div_b);
#endif
    // read actual config
    HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
    // clear divisor_a and divisor_b fields
    ctrl_reg &= ~(0x0000C000 | 0x00003F00);
    //set new values of divisor_a and divisor_b
    ctrl_reg |= ((div_a << 14) | (div_b << 8));
    // write config
    HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
    // enable interrupts
    HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_IER_OFFSET, XI2CPS_ENABLED_INTR);
    // set time-out as long as possible
    HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_TIME_OUT_OFFSET, 0xFF);
#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("init OK\n");
#endif
}

/*
 * dev - device structure (see: io/i2c/i2c.h)
 * send_start - true if we must send start condition (only on first part of data)
 * tx_data - no comments
 * count - number of data in tx_data
 * send_stop - true if there will be no more data to send at this session, we must send stop
 */
cyg_uint32 zynq_i2c_tx(const cyg_i2c_device* dev,
            cyg_bool send_start,
            const cyg_uint8* tx_data, 
            cyg_uint32 count, 
            cyg_bool send_stop) 
{
    //get driver private data
    cyg_zynq_i2c_extra* extra = (cyg_zynq_i2c_extra*)dev->i2c_bus->i2c_extra;
    cyg_uint16 ctrl_reg;
    cyg_uint8 bytes_to_send;
    cyg_uint16 isr_value;

    extra->i2c_addr = dev->i2c_address;
    extra->i2c_bytes_left = count;
    extra->i2c_tx_buf = tx_data;

#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("tx start\n");
#endif

    if(send_start) //init transmission
    {
        HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
        // Clear all except div fields
        ctrl_reg &= 0xff00;
        // clear FIFO, master transmit mode
        ctrl_reg |= ((1 << CR_CLR_FIFO) | (1 << CR_ACKEN) | (1 << CR_NEA) | (1 << CR_MS));
        // if there are more data to send than fifo length
        // or we don't want to send stop
        // or we continue proevious transmission: set HOLD
        if(extra->i2c_bytes_left > XI2CPS_FIFO_DEPTH || !send_stop || (extra->i2c_flag & I2C_FLAG_ACT))
        {
            ctrl_reg |= (1 << CR_HOLD);
            extra->i2c_hold_flag = 1;
        }
        else
        {
            extra->i2c_hold_flag = 0;
        }
        // Write config bits
        HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
        // Feed data to FIFO
        zynq_i2c_feed_data(extra);
        // Write Slave address - generate start condition, tx start
        HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_ADDR_OFFSET, (dev->i2c_address & XI2CPS_ADDR_MASK));	
    }
    else //transmission in progress
    {
        // Feed data to FIFO
        zynq_i2c_feed_data(extra);
    }

#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("isr starting\n");
#endif
    //TODO: mutex_lock on bus level performed in io/i2c.cxx
    //TODO: line below to remove?
    cyg_drv_mutex_lock(&extra->i2c_lock);
    cyg_drv_dsr_lock();
    cyg_drv_interrupt_unmask(extra->i2c_isr_vector);
#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("waiting for data transmission...\n");
#endif
    while(!(extra->i2c_flag & (I2C_FLAG_FINISH | I2C_FLAG_ERROR)))
    {
#ifdef ZYNQ_I2C_DEBUG
        HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, isr_value);
        diag_printf("CR: %x\n", isr_value);
        HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_SR_OFFSET, isr_value);
        diag_printf("SR: %x\n", isr_value);
        HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_ADDR_OFFSET, isr_value);
        diag_printf("ADDR: %x\n", isr_value);
        HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_ISR_OFFSET, isr_value);
        diag_printf("ISR: %x\n", isr_value);
        HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_IMR_OFFSET, isr_value);
        diag_printf("IMR: %x\n", isr_value);
#endif
        cyg_drv_cond_wait(&extra->i2c_wait);
    }
#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("data transmitted!?\n");
#endif
    cyg_drv_interrupt_mask(extra->i2c_isr_vector);
    cyg_drv_dsr_unlock();
    cyg_drv_mutex_unlock(&extra->i2c_lock);

    if(extra->i2c_flag & I2C_FLAG_ERROR)
    {
#ifdef ZYNQ_I2C_DEBUG
        DPRINTF("TX error extra->i2c_flag = ");
        diag_printf("%x\n", extra->i2c_flag);
#endif
        extra->i2c_flag = 0;
        //TODO: condition for hold_flag?
        HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
        ctrl_reg &= ~(1 << CR_HOLD);
        HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
    }
    else
    {
        if(send_stop)
        {
            //TODO: condition for hold_flag?
            HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
            ctrl_reg &= ~(1 << CR_HOLD);
            HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
            extra->i2c_flag = 0;
        }
        else
        {
            extra->i2c_flag = I2C_FLAG_ACT;
        }
    }

    count -= extra->i2c_bytes_left;
    extra->i2c_addr  = 0;
    extra->i2c_bytes_left = 0;
    extra->i2c_tx_buf = NULL;
#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("tx finished\n");
#endif
    return count;        
}

cyg_uint32
zynq_i2c_rx(const cyg_i2c_device* dev,
             cyg_bool send_start,
             cyg_uint8* rx_data, cyg_uint32 count,
             cyg_bool send_nack, cyg_bool send_stop)
{
    //get driver private data
    cyg_zynq_i2c_extra* extra = (cyg_zynq_i2c_extra*)dev->i2c_bus->i2c_extra;
    cyg_uint16 ctrl_reg;
    cyg_uint8 bytes_to_send;
    cyg_uint16 isr_status;

#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("RX start\n");
#endif

    extra->i2c_addr = dev->i2c_address;
    extra->i2c_bytes_left = count;
    extra->i2c_rx_buf = rx_data;

    if(send_start)
    {
        HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
        // Clear all except div fields
        ctrl_reg &= 0xff00;
        // clear FIFO, master receive mode
        ctrl_reg |= ((1 << CR_RW) | (1 << CR_CLR_FIFO) | (1 << CR_ACKEN) | (1 << CR_NEA) | (1 << CR_MS));
        if(extra->i2c_bytes_left > XI2CPS_FIFO_DEPTH || !send_stop || (extra->i2c_flag & I2C_FLAG_ACT))
        {
            ctrl_reg |= (1 << CR_HOLD);
            extra->i2c_hold_flag = 1;
        }
        // Write config bits
        HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
        // write number of data to receive from slave
        if(extra->i2c_bytes_left > XI2CPS_FIFO_DEPTH)
            HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_XFER_SIZE_OFFSET, XI2CPS_FIFO_DEPTH + 1);
        else
            HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_XFER_SIZE_OFFSET, extra->i2c_bytes_left);
        // Write Slave address - generate start condition, tx start
        HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_ADDR_OFFSET, (dev->i2c_address & XI2CPS_ADDR_MASK));
    }
    else //transmission in progress
    {
        // write number of data to receive from slave
        if(extra->i2c_bytes_left > XI2CPS_FIFO_DEPTH)
            HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_XFER_SIZE_OFFSET, XI2CPS_FIFO_DEPTH + 1);
        else
            HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_XFER_SIZE_OFFSET, extra->i2c_bytes_left);
    }

#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("isr starting\n");
#endif
    cyg_drv_mutex_lock(&extra->i2c_lock);
    cyg_drv_dsr_lock();
    cyg_drv_interrupt_unmask(extra->i2c_isr_vector);
#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("waiting for data reception...\n");
#endif
    while(!(extra->i2c_flag & (I2C_FLAG_FINISH | I2C_FLAG_ERROR)))
    {
        cyg_drv_cond_wait(&extra->i2c_wait);
    }
#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("data received!?\n");
#endif
    cyg_drv_interrupt_mask(extra->i2c_isr_vector);
    cyg_drv_dsr_unlock();
    cyg_drv_mutex_unlock(&extra->i2c_lock);

    if(extra->i2c_flag & I2C_FLAG_ERROR)
    {
#ifdef ZYNQ_I2C_DEBUG
        DPRINTF("RX error extra->i2c_flag = ");
        diag_printf("%x\n", extra->i2c_flag);
#endif
        extra->i2c_flag = 0;
        //TODO: condition for hold_flag?
        HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
        ctrl_reg &= ~(1 << CR_HOLD);
        HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
    }
    else
    {
        if(send_stop)
        {
            //TODO: condition for hold_flag?
            HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
            ctrl_reg &= ~(1 << CR_HOLD);
            HAL_WRITE_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
            extra->i2c_flag = 0;
        }
        else
        {
            extra->i2c_flag = I2C_FLAG_ACT;
        }
    }

    count -= extra->i2c_bytes_left;
    extra->i2c_addr = 0;
    extra->i2c_bytes_left = 0;
    extra->i2c_rx_buf = NULL;
#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("rx finished\n")
#endif
    return count;
}

void zynq_i2c_stop(const cyg_i2c_device* dev)
{
    //get driver private data
    cyg_zynq_i2c_extra* extra = (cyg_zynq_i2c_extra*)dev->i2c_bus->i2c_extra;
    cyg_uint16 ctrl_reg;
        
    // setting a stop condition : Clear the HOLD bus bit
    HAL_READ_UINT16(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
    if ((ctrl_reg & XI2CPS_CR_HOLD_BUS_MASK) == XI2CPS_CR_HOLD_BUS_MASK)
    {
        ctrl_reg &= ~XI2CPS_CR_HOLD_BUS_MASK;
        HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_CR_OFFSET, ctrl_reg);
    }
        
    extra->i2c_bytes_left = 0;
    extra->i2c_rx_buf = NULL;
}

// DSR signals data
static void zynq_i2c_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("DSR executed\n");
#endif
    cyg_zynq_i2c_extra* extra = (cyg_zynq_i2c_extra*)data;
    if(extra->i2c_flag)
    {
        cyg_drv_cond_signal(&extra->i2c_wait);
    }
    cyg_drv_interrupt_unmask(vec);
}

//
static cyg_uint32 zynq_i2c_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    cyg_drv_interrupt_mask(vec);

    cyg_zynq_i2c_extra* extra = (cyg_zynq_i2c_extra*)data;
    cyg_uint16 isr_value;
    cyg_uint16 sr_value;
    cyg_uint8 avail_bytes;
    cyg_uint8 trans_size;
    
    HAL_READ_UINT16(XI2CPS_BASE+XI2CPS_ISR_OFFSET, isr_value);
    HAL_READ_UINT16(XI2CPS_BASE+XI2CPS_SR_OFFSET, sr_value);

#ifdef ZYNQ_I2C_DEBUG
    DPRINTF("registers ");
    diag_printf("ISR: %x SR: %x\n", isr_value, sr_value);
#endif
    
    if (isr_value & XI2CPS_ISR_NACK) //NACK
    {
        extra->i2c_flag = I2C_FLAG_ERROR | I2C_FLAG_ADDR | I2C_FLAG_DATA;
    } 
    else if (isr_value & XI2CPS_ISR_ARB_LOST) // Arbitration Lost
    {
        extra->i2c_flag = I2C_FLAG_ERROR | I2C_FLAG_LOST;
    }
    // process data reception, 2 bytes of FIFO remain free
    else if ((isr_value & XI2CPS_ISR_DATA) && (sr_value & XI2CPS_SR_RXDV))
    {
        if(extra->i2c_bytes_left > XI2CPS_FIFO_DEPTH)
        {
            //calculate amount of data to read from fifo
            HAL_READ_UINT8(XI2CPS_BASE + XI2CPS_XFER_SIZE_OFFSET, trans_size);
            avail_bytes = (XI2CPS_FIFO_DEPTH + 1) - trans_size;

            while(avail_bytes--) 
            {
                HAL_READ_UINT8(XI2CPS_BASE + XI2CPS_DATA_OFFSET, *extra->i2c_rx_buf);
                extra->i2c_rx_buf++;
                extra->i2c_bytes_left--;
            }
            
            // write number of data to receive from slave
            // more data than fifo length
            if(extra->i2c_bytes_left > XI2CPS_FIFO_DEPTH)
            {
                HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_XFER_SIZE_OFFSET, XI2CPS_FIFO_DEPTH + 1);
            }
            // no more data, signal end of reception
            /*else if(extra->i2c_bytes_left == 0)
            {
                extra->i2c_flag = I2C_FLAG_FINISH;
                cyg_drv_interrupt_mask_intunsafe(vec);
            }*/
            else
            {
                HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_XFER_SIZE_OFFSET, extra->i2c_bytes_left);
            }
        }
    }
    else if (isr_value & XI2CPS_ISR_COMP) // transfer complete
    {
        // Master Transmit
        if(extra->i2c_rx_buf == NULL)
        {
            if(extra->i2c_bytes_left > 0)
            {
                cyg_uint32 avail_bytes;
                cyg_uint8 fifo_free;
          
                HAL_READ_UINT8(XI2CPS_BASE + XI2CPS_XFER_SIZE_OFFSET, fifo_free);
                avail_bytes = XI2CPS_FIFO_DEPTH - fifo_free;
                if(avail_bytes > extra->i2c_bytes_left) avail_bytes = extra->i2c_bytes_left;
                
                while(avail_bytes--) 
                {
                    HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_DATA_OFFSET, *extra->i2c_tx_buf);
                    extra->i2c_tx_buf++;
                    extra->i2c_bytes_left--;
                }
            }
            else //extra->i2c_bytes_left == 0
            {
                extra->i2c_flag = I2C_FLAG_FINISH;
                cyg_drv_interrupt_mask_intunsafe(vec);
            }
        }
        // Master Receive
        else
        {
            avail_bytes = extra->i2c_bytes_left;
            while(avail_bytes--) 
            {
                HAL_READ_UINT8(XI2CPS_BASE + XI2CPS_DATA_OFFSET, *extra->i2c_rx_buf);
                extra->i2c_rx_buf++;
                extra->i2c_bytes_left--;
            }
            extra->i2c_flag = I2C_FLAG_FINISH;
            cyg_drv_interrupt_mask_intunsafe(vec);
        }
    }
    
    cyg_drv_interrupt_acknowledge(vec);

    // Clear ststus bits
    HAL_WRITE_UINT16(XI2CPS_BASE+XI2CPS_ISR_OFFSET, isr_value);
    
    // We need to call the DSR only if there is really something to signal,
    // that means only if extra->i2c_flag != 0
    if (extra->i2c_flag)
    {
        return CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
    }
    else
    {
        cyg_drv_interrupt_unmask(vec);
        return CYG_ISR_HANDLED; 
    }
}

void zynq_i2c_feed_data(cyg_zynq_i2c_extra* ex)
{
    cyg_uint32 avail_bytes;
    cyg_uint8 fifo_free;

    HAL_READ_UINT8(XI2CPS_BASE + XI2CPS_XFER_SIZE_OFFSET, fifo_free);
    avail_bytes = XI2CPS_FIFO_DEPTH - fifo_free;
    if (avail_bytes > ex->i2c_bytes_left) avail_bytes = ex->i2c_bytes_left;    

    while(avail_bytes--)
    {
        HAL_WRITE_UINT8(XI2CPS_BASE + XI2CPS_DATA_OFFSET, *ex->i2c_tx_buf);
        ex->i2c_tx_buf++;
        ex->i2c_bytes_left--;
    }
}

