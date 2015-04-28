/**************************************************************************/
/**
*
* @file     qspi_xc7z.c
*
* @brief    Xilinx XC7Z (ARM) QSPI driver
*
***************************************************************************/
//==========================================================================
//
//      qspi_xc7z.c
//
//      Xilinx XC7Z (ARM) QSPI driver
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2009, 2012 Free Software Foundation, Inc.
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
// Author(s):     ITR-GmbH
// Date:          2012-07-11
//
// Contributor(s): Ant Micro <www.antmicro.com>
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_spi.h>
#include <pkgconf/devs_qspi_arm_xc7z.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/spi.h>
#include <cyg/io/qspi_xc7z.h>
#include <cyg/error/codes.h>

#define XQSPIPS_CR_OFFSET       0x00000000
#define XQSPIPS_SR_OFFSET       0x00000004
#define XQSPIPS_IER_OFFSET      0x00000008
#define XQSPIPS_IDR_OFFSET      0x0000000C
#define XQSPIPS_ER_OFFSET       0x00000014 // SPI_enable register
#define XQSPIPS_TXD_00_OFFSET   0x0000001C
#define XQSPIPS_RXD_OFFSET      0x00000020
#define XQSPIPS_TXD_01_OFFSET   0x00000080
#define XQSPIPS_TXD_10_OFFSET   0x00000084
#define XQSPIPS_TXD_11_OFFSET   0x00000088
// TODO datasheet name for the next one is LQSPI_CFG(_OFFSET)
#define XQSPIPS_LQSPI_CR_OFFSET 0x000000A0 // Linear QSPI controller control register

#define XQSPIPS_CR_MSTREN_MASK    (1 <<  0) // MODE_SEL
#define XQSPIPS_CR_CPOL_MASK      (1 <<  1) // CLK_POL
#define XQSPIPS_CR_CPHA_MASK      (1 <<  2) // CLK_PH
// TODO datasheet name for the next one is CR_BAUD_RATE_DIV(_SHIFT)
#define XQSPIPS_CR_PRESC_SHIFT           3
// TODO datasheet name for the next two is CR_PCS(_SHIFT/MASK)
#define XQSPIPS_CR_SSCTRL_SHIFT         10
#define XQSPIPS_CR_SSCTRL_MASK    (1 << 10)

// TODO unsure if this is FIFO_WIDTH? - order suggests this
#define XQSPIPS_CR_DATA_SZ_MASK   (3 <<  6)

#define XQSPIPS_CR_SSFORCE_MASK   (1 << 14) // Manual_CS
#define XQSPIPS_CR_MANSTRTEN_MASK (1 << 15) // Man_start_en
#define XQSPIPS_CR_MANSTRT_MASK   (1 << 16) // Man_start_com
#define XQSPIPS_CR_IFMODE_MASK    (1 << 31) // leg_flash

// shared defines between all interrupt registers (SR, IMR, IER, IDR)
#define XQSPIPS_IXR_MODF_MASK     (1 << 1) // MODE_FAIL
#define XQSPIPS_IXR_TXOW_MASK     (1 << 2) // TX_FIFO_not_full
#define XQSPIPS_IXR_TXFULL_MASK   (1 << 3) // TX_FIFO_full
#define XQSPIPS_IXR_RXNEMPTY_MASK (1 << 4) // RX_FIFO_not_empty

#define XQSPIPS_ER_ENABLE_MASK    (1 << 0) // SPI_EN

#define XQSPIPS_FLASH_OPCODE_WRSR       0x01    /* Write status register */
#define XQSPIPS_FLASH_OPCODE_PP         0x02    /* Page program */
#define XQSPIPS_FLASH_OPCODE_PPQ        0x32    /* Page program Quad (W25Q) */
#define XQSPIPS_FLASH_OPCODE_NORM_READ  0x03    /* Normal read data bytes */
#define XQSPIPS_FLASH_OPCODE_WRDS       0x04    /* Write disable */
#define XQSPIPS_FLASH_OPCODE_RDSR1      0x05    /* Read status register 1 */
#define XQSPIPS_FLASH_OPCODE_WREN       0x06    /* Write enable */
#define XQSPIPS_FLASH_OPCODE_FAST_READ  0x0B    /* Fast read data bytes */
#define XQSPIPS_FLASH_OPCODE_BE_4K      0x20    /* Erase 4KiB block */
#define XQSPIPS_FLASH_OPCODE_RDSR2      0x35    /* Read status register 2 */
#define XQSPIPS_FLASH_OPCODE_DUAL_READ  0x3B    /* Dual read data bytes */
#define XQSPIPS_FLASH_OPCODE_BE_32K     0x52    /* Erase 32KiB block */
#define XQSPIPS_FLASH_OPCODE_QUAD_READ  0x6B    /* Quad read data bytes */
#define XQSPIPS_FLASH_OPCODE_ERASE_SUS  0x75    /* Erase suspend */
#define XQSPIPS_FLASH_OPCODE_ERASE_RES  0x7A    /* Erase resume */
#define XQSPIPS_FLASH_OPCODE_RDID       0x9F    /* Read JEDEC ID */
#define XQSPIPS_FLASH_OPCODE_BE         0xC7    /* Erase whole flash block */
#define XQSPIPS_FLASH_OPCODE_SE         0xD8    /* Sector erase (usually 64KB)*/

/****************************************************************************/
/**
*
* Functions defenitions
*
*****************************************************************************/

#if 0
#define entry_debug() diag_printf("Entering %s\n", __func__);
#define exit_debug() diag_printf("Leaving %s\n", __func__);
#endif

#define entry_debug()
#define exit_debug()
//#define diag_printf(...)

static void qspi_xc7z_init_bus(cyg_qspi_xc7z_bus_t * bus);

static cyg_uint32 qspi_xc7z_ISR(cyg_vector_t vector, cyg_addrword_t data);

static void qspi_xc7z_DSR(cyg_vector_t   vector,
                         cyg_ucount32   count,
                         cyg_addrword_t data);

static void qspi_xc7z_transaction_begin(cyg_spi_device *dev);

static void qspi_xc7z_transaction_transfer(cyg_spi_device  *dev,
                                          cyg_bool         polled,
                                          cyg_uint32       count,
                                          const cyg_uint8 *tx_data,
                                          cyg_uint8       *rx_data,
                                          cyg_bool         drop_cs);

static void qspi_xc7z_transaction_tick(cyg_spi_device *dev,
                                      cyg_bool        polled,
                                      cyg_uint32      count);

static void qspi_xc7z_transaction_end(cyg_spi_device* dev);

static int qspi_xc7z_get_config(cyg_spi_device *dev,
                               cyg_uint32      key,
                               void           *buf,
                               cyg_uint32     *len);

static int qspi_xc7z_set_config(cyg_spi_device *dev,
                               cyg_uint32      key,
                               const void     *buf,
                               cyg_uint32     *len);
                               
static void qspi_xc7z_copy_read_data(cyg_qspi_xc7z_bus_t    *qspi_bus,
                                cyg_uint32             data,
                                cyg_uint8              len_burst
                                );
                         
static void qspi_xc7z_copy_write_data(cyg_qspi_xc7z_bus_t    *qspi_bus,
                                cyg_uint32              *data,
                                cyg_uint8                len_burst
                                );
                         
static void qspi_xc7z_fill_tx_fifo(cyg_qspi_xc7z_bus_t    *qspi_bus, int max);

static void qspi_xc7z_send_instruction(cyg_qspi_xc7z_bus_t    *qspi_bus);


/****************************************************************************/
/**
*
* Write pointers to process functions
*
*****************************************************************************/
#ifdef CYGHWR_DEVS_QSPI_ARM_XC7Z
cyg_qspi_xc7z_bus_t cyg_qspi_xc7z_bus0 = {
    .qspi_bus.spi_transaction_begin    = qspi_xc7z_transaction_begin,
    .qspi_bus.spi_transaction_transfer = qspi_xc7z_transaction_transfer,
    .qspi_bus.spi_transaction_tick     = qspi_xc7z_transaction_tick,
    .qspi_bus.spi_transaction_end      = qspi_xc7z_transaction_end,
    .qspi_bus.spi_get_config           = qspi_xc7z_get_config,
    .qspi_bus.spi_set_config           = qspi_xc7z_set_config,
    .interrupt_number                  = CYGNUM_HAL_INTERRUPT_QSPI,
    .base                              = XC7Z_QSPI_BASEADDR,
    .cs_en[0]                          = true,
};

CYG_SPI_DEFINE_BUS_TABLE(cyg_qspi_xc7z_device_t, 0);
#endif

/****************************************************************************/
/**
*
* A buffer of 64 zeros for dummy transmits
*
*****************************************************************************/
static const cyg_uint32 zeros[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

/****************************************************************************/
/**
*
* This typedef defines qspi flash instruction format
*
*****************************************************************************/
typedef struct {
    cyg_uint8 OpCode;      /**< Operational code of the instruction */
    cyg_uint8 InstSize;    /**< Size of the instruction including address bytes */
    cyg_uint8 TxOffset;    /**< Register address where instruction has to be written */
} tXQspiPsInstFormat;

/****************************************************************************/
/**
*
* List of all the QSPI instructions and its format
*
*****************************************************************************/
static tXQspiPsInstFormat qpifFlashInst[] = {
    { XQSPIPS_FLASH_OPCODE_WREN,      1, XQSPIPS_TXD_01_OFFSET },
    { XQSPIPS_FLASH_OPCODE_WRDS,      1, XQSPIPS_TXD_01_OFFSET },
    { XQSPIPS_FLASH_OPCODE_RDSR1,     1, XQSPIPS_TXD_01_OFFSET },
    { XQSPIPS_FLASH_OPCODE_RDSR2,     1, XQSPIPS_TXD_01_OFFSET },
    { XQSPIPS_FLASH_OPCODE_WRSR,      1, XQSPIPS_TXD_01_OFFSET },
    { XQSPIPS_FLASH_OPCODE_PP,        4, XQSPIPS_TXD_00_OFFSET },
    { XQSPIPS_FLASH_OPCODE_PPQ,       4, XQSPIPS_TXD_00_OFFSET },
    { XQSPIPS_FLASH_OPCODE_SE,        4, XQSPIPS_TXD_00_OFFSET },
    { XQSPIPS_FLASH_OPCODE_BE_32K,    4, XQSPIPS_TXD_00_OFFSET },
    { XQSPIPS_FLASH_OPCODE_BE_4K,     4, XQSPIPS_TXD_00_OFFSET },
    { XQSPIPS_FLASH_OPCODE_BE,        1, XQSPIPS_TXD_01_OFFSET },
    { XQSPIPS_FLASH_OPCODE_ERASE_SUS, 1, XQSPIPS_TXD_01_OFFSET },
    { XQSPIPS_FLASH_OPCODE_ERASE_RES, 1, XQSPIPS_TXD_01_OFFSET },
    { XQSPIPS_FLASH_OPCODE_RDID,      4, XQSPIPS_TXD_00_OFFSET },
    { XQSPIPS_FLASH_OPCODE_NORM_READ, 4, XQSPIPS_TXD_00_OFFSET },
    { XQSPIPS_FLASH_OPCODE_FAST_READ, 4, XQSPIPS_TXD_00_OFFSET },
    { XQSPIPS_FLASH_OPCODE_DUAL_READ, 4, XQSPIPS_TXD_00_OFFSET },
    { XQSPIPS_FLASH_OPCODE_QUAD_READ, 4, XQSPIPS_TXD_00_OFFSET },
    /* Add all the instructions supported by the flash device */
}; 

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))

/****************************************************************************/
/**
*
* QSPI bus initialization function from eCOS
*
* @return   none
*
*****************************************************************************/
void cyg_qspi_xc7z_bus_init(void)
{
  entry_debug();
#ifdef CYGHWR_DEVS_QSPI_ARM_XC7Z
   // NOTE: here we let the SPI controller control
   //       the data in, out and clock signals, but
   //       we need to handle the chip selects manually
   //       in order to achieve better chip select control
   //       in between transactions.

   // Put SPI MISO, MOSI and SPCK pins into peripheral mode
   
   qspi_xc7z_init_bus(&cyg_qspi_xc7z_bus0);
#endif

}

/****************************************************************************/
/**
*
* QSPI bus initialization function
*
* @param    qspi_bus     - Driver handle
*
* @return   none
*
*****************************************************************************/
static void qspi_xc7z_init_bus(cyg_qspi_xc7z_bus_t * qspi_bus)
{
    entry_debug();
    volatile cyg_uint32 reg;
    
    // Create and attach SPI interrupt object
    cyg_drv_interrupt_create(qspi_bus->interrupt_number,
                             4,
                             (cyg_addrword_t)qspi_bus,
                             qspi_xc7z_ISR,
                             qspi_xc7z_DSR,
                             &qspi_bus->qspi_interrupt_handle,
                             &qspi_bus->qspi_interrupt);

    cyg_drv_interrupt_attach(qspi_bus->qspi_interrupt_handle);
    cyg_drv_interrupt_mask(qspi_bus->interrupt_number);  

    // Init transfer mutex and condition
    cyg_drv_mutex_init(&qspi_bus->transfer_mx);
    cyg_drv_cond_init(&qspi_bus->transfer_cond,
                      &qspi_bus->transfer_mx);

    // Init flags
    qspi_bus->transfer_end = true;
    qspi_bus->cs_up        = false;
    
    // Unlock SLCR regs
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCR_UNLOCK_OFFSET, XSLCR_UNLOCK_KEY);
    
    // Enable clock to QSPI module
    HAL_READ_UINT32( XC7Z_SYS_CTRL_BASEADDR + XSLCRAPER_CLK_CTRL_OFFSET, reg);
    reg |= XSLCRAPER_CLK_CTRL_QSPI_EN;
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRAPER_CLK_CTRL_OFFSET, reg);
    
    // Lock SLCR regs
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCR_LOCK_OFFSET, XSLCR_LOCK_KEY);
    
    // Soft reset the SPI controller
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_ER_OFFSET,        0x00);
   
    //TODO changed, originally was just setting a value without reading
    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET, reg);
    reg &= (1 << 17); // preserve the reserved bit which is described as "do not modify"
    reg |= (1 << 31); 
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET, reg);
    
    // Disable linear mode
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_LQSPI_CR_OFFSET,  0x00);
    
    // Clear status
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET,        0x7F);
    
    // Clear the RX FIFO
    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET, reg);
    while (reg & XQSPIPS_IXR_RXNEMPTY_MASK)
    {
        HAL_READ_UINT32(qspi_bus->base + XQSPIPS_RXD_OFFSET, reg);
        HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET,  reg);
    }
    
    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,         reg);
    reg &= 0xFBFFFFFF; /* Set little endian mode of TX FIFO */
    reg |= (XQSPIPS_CR_IFMODE_MASK | XQSPIPS_CR_MANSTRTEN_MASK | XQSPIPS_CR_SSFORCE_MASK |
            XQSPIPS_CR_SSCTRL_MASK | XQSPIPS_CR_DATA_SZ_MASK | XQSPIPS_CR_MSTREN_MASK);
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,         reg);
    
    // Configure SPI pins
    // All pins was configured in HAL

    // Call upper layer bus init
    CYG_SPI_BUS_COMMON_INIT(&qspi_bus->qspi_bus);
}

/****************************************************************************/
/**
*
* QSPI bus interrupt service routine
*
* @param    vector     - Number of ISR
* @param    data       - Pointer to data structure (with driver handle)
*
* @return   ISR return condition
*
*****************************************************************************/
static cyg_uint32
qspi_xc7z_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    cyg_uint32 stat;
    cyg_uint32 val;
    cyg_qspi_xc7z_bus_t * qspi_bus = (cyg_qspi_xc7z_bus_t *)data;
    cyg_uint32 return_cond = CYG_ISR_HANDLED;
    
    // Read the status register and clear interrupt immediately
    // the SPI int events that have occurred

    
    HAL_READ_UINT32(qspi_bus->base  + XQSPIPS_SR_OFFSET,   stat);
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET,   stat);
    
    // Read the status register and
    // check for transfer completion
    

    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET, stat);
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET, stat); //TODO: is it necessary 
    
    if(stat & XQSPIPS_IXR_MODF_MASK)
    {
        cyg_drv_interrupt_mask(vector);
       
        return_cond  |= CYG_ISR_CALL_DSR;
        
    }
    else
    {
        if ((stat & XQSPIPS_IXR_TXOW_MASK) || (stat & XQSPIPS_IXR_RXNEMPTY_MASK)) {
        
            /* Read out the data from the RX FIFO */
            HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET,     val);
            while (val & XQSPIPS_IXR_RXNEMPTY_MASK) {
                
		HAL_READ_UINT32(qspi_bus->base + XQSPIPS_RXD_OFFSET,    val);

                if (qspi_bus->us_rx_bytes < 4)
                    qspi_xc7z_copy_read_data(qspi_bus, val, qspi_bus->us_rx_bytes);
                else
                    qspi_xc7z_copy_read_data(qspi_bus, val, 4);
                
                HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET,     val);
            }
            
            if (qspi_bus->us_tx_bytes){
                
                qspi_xc7z_fill_tx_fifo(qspi_bus,8);
                
                HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_IER_OFFSET,
                        XQSPIPS_IXR_TXOW_MASK | XQSPIPS_IXR_MODF_MASK);
                
                // Transfer still in progress - unmask the SPI
                // int so we can get more SPI int events
                cyg_drv_interrupt_unmask(vector);                        
        
                HAL_READ_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,     val);
                val |= XQSPIPS_CR_MANSTRT_MASK;
                HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,    val);
                
            } else         
            if (qspi_bus->us_rx_bytes){
                
                /* Read out the data from the RX FIFO */
                HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET,     val);

                while (val & XQSPIPS_IXR_RXNEMPTY_MASK) {
                    
                    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_RXD_OFFSET,    val);

                    if (qspi_bus->us_rx_bytes < 4)
                        qspi_xc7z_copy_read_data(qspi_bus, val, qspi_bus->us_rx_bytes);
                    else
                        qspi_xc7z_copy_read_data(qspi_bus, val, 4);
                    
                    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET,     val);
                }
                
            } else {
            
                cyg_drv_interrupt_mask(vector);
                return_cond  |= CYG_ISR_CALL_DSR;
            }
            
        }

    }
    cyg_drv_interrupt_acknowledge(vector);
    
    return return_cond;
}

/****************************************************************************/
/**
*
* QSPI bus DSR handler
*
* @param    vector     - Number of ISR
* @param    data       - Pointer to data structure (with driver handle)
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    cyg_qspi_xc7z_bus_t * qspi_bus = (cyg_qspi_xc7z_bus_t *)data;
    
    // Transfer ended
    qspi_bus->transfer_end = true;
    cyg_drv_cond_signal(&qspi_bus->transfer_cond);
}

/****************************************************************************/
/**
*
* QSPI bus baud rate calculator and BR upgrade function
*
* @param    dev     - QSPI device handle
*
* @return   TRUE - if BR set properly, FALSE - if fail to set BR
*
*****************************************************************************/
static cyg_bool
qspi_xc7z_calc_bratediv(cyg_qspi_xc7z_device_t *dev)
{
    entry_debug();
    cyg_qspi_xc7z_bus_t *qspi_bus = (cyg_qspi_xc7z_bus_t *)dev->qspi_device.spi_bus;
    volatile cyg_uint32 baud_rate_val;
    volatile cyg_uint32 reg;
    cyg_bool   res = true;

    // Calculate baud_rate_val from baud rate
    baud_rate_val = 0;
    while ((baud_rate_val < 8)  &&
        ((CYGHWR_HAL_ARM_SOC_PROCESSOR_CLOCK / (2 << baud_rate_val)) > dev->cl_brate)) {
            baud_rate_val++;
    }
    
    baud_rate_val &= 0x07;
    
    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,     reg);
    reg &= 0xFFFFFFC7;
    reg |= (baud_rate_val << XQSPIPS_CR_PRESC_SHIFT);
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,    reg);

    return res;
}

/****************************************************************************/
/**
*
* QSPI bus control chip select signal
*
* @param    qspi_bus     - QSPI bus handle
* @param    val          - 1-CS is High, 0-CS is Low.
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_set_npcs(cyg_qspi_xc7z_bus_t *qspi_bus,int val)
{
    entry_debug();
    cyg_uint32 reg;
    
    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,     reg);
    
    if (!val)
        reg &= ~(1 << XQSPIPS_CR_SSCTRL_SHIFT);
    else
        reg |= (1 << XQSPIPS_CR_SSCTRL_SHIFT);
    
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,    reg);
    
}

/****************************************************************************/
/**
*
* QSPI bus start transfer function
*
* @param    dev     - QSPI device handle
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_start_transfer(cyg_qspi_xc7z_device_t *dev)
{
    entry_debug();

    cyg_qspi_xc7z_bus_t *qspi_bus = (cyg_qspi_xc7z_bus_t *)dev->qspi_device.spi_bus;

    if (qspi_bus->cs_up)
        return;
    
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_ER_OFFSET,    XQSPIPS_ER_ENABLE_MASK);

    // Force minimal delay between two transfers - in case two transfers
    // follow each other w/o delay, then we have to wait here in order for
    // the peripheral device to detect cs transition from inactive to active.
    CYGACC_CALL_IF_DELAY_US(dev->tr_bt_udly);

    // Raise CS
    qspi_xc7z_set_npcs(qspi_bus, 0);
    CYGACC_CALL_IF_DELAY_US(dev->cs_up_udly);

    qspi_bus->cs_up = true;
    qspi_bus->uc_tx_instr = 0;
}

/****************************************************************************/
/**
*
* QSPI bus drop CS function
*
* @param    dev     - QSPI device handle
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_drop_cs(cyg_qspi_xc7z_device_t *dev)
{
	entry_debug();
    cyg_qspi_xc7z_bus_t *qspi_bus = (cyg_qspi_xc7z_bus_t *)dev->qspi_device.spi_bus;

    if (!qspi_bus->cs_up)
       return;

    // Drop CS

    CYGACC_CALL_IF_DELAY_US(dev->cs_dw_udly);
    qspi_xc7z_set_npcs(qspi_bus, 1);
    qspi_bus->cs_up = false;
}

/****************************************************************************/
/**
*
* QSPI bus RX prepare data (for Zynq specific FIFO).
* For correct working with unaligned data, used 1-byte transactions.
*
* @param    qspi_bus     - QSPI bus handle
* @param    data         - 4-byte variable which RX data has.
* @param    len_burst    - Size of data in bytes [1..4].
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_copy_read_data(cyg_qspi_xc7z_bus_t    *qspi_bus,
                         cyg_uint32             data,
                         cyg_uint8              len_burst
                         )
{
    entry_debug();
    cyg_uint8 btemp;

    if (qspi_bus->lp_rx_buf) {
        switch (len_burst) {
        case 1:
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = (data >> 24) & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            break;
        case 2:
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = (data >> 16) & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = (data >> 24) & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            break;
        case 3:
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = (data >> 8) & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = (data >> 16) & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = (data >> 24) & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            break;
        case 4:
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = data & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = (data >> 8) & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = (data >> 16) & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            *((cyg_uint8 *)qspi_bus->lp_rx_buf) = (data >> 24) & 0xFF;
            qspi_bus->lp_rx_buf += 1;
            break;
        default:
            /* This will never execute */
            break;
        }
    }
    if (qspi_bus->us_rx_bytes < len_burst)
        qspi_bus->us_rx_bytes = 0;
    else 
        qspi_bus->us_rx_bytes -= len_burst;
}

/****************************************************************************/
/**
*
* QSPI bus TX prepare data function (for Zynq specific FIFO). 
*
* @param    qspi_bus     - QSPI bus handle
* @param    data         - 4-byte variable which TX data has.
* @param    len_burst    - Size of data in bytes [1..4].
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_copy_write_data(cyg_qspi_xc7z_bus_t    *qspi_bus,
                         cyg_uint32              *data,
                         cyg_uint8                len_burst
                         )
{
	entry_debug();
	cyg_uint8 *d = (cyg_uint8*)qspi_bus->lp_tx_buf;
	int i;
    if (qspi_bus->lp_tx_buf) {
        switch (len_burst) {
        case 1:
            *data = *((cyg_uint8 *)qspi_bus->lp_tx_buf);
            qspi_bus->lp_tx_buf += 1;
            *data |= 0xFFFFFF00;
            break;
        case 2:
            *data = d[0] | (d[1] << 8);
            qspi_bus->lp_tx_buf += 2;
            *data |= 0xFFFF0000;
            break;
        case 3:
            *data = *((cyg_uint16 *)qspi_bus->lp_tx_buf);
            qspi_bus->lp_tx_buf += 2;
            *data |= (*((cyg_uint8 *)qspi_bus->lp_tx_buf) << 16);
            qspi_bus->lp_tx_buf += 1;
            *data |= 0xFF000000;
            break;
        case 4:
            *data = *((cyg_uint32 *)qspi_bus->lp_tx_buf);
            qspi_bus->lp_tx_buf += 4;
            break;
        default:
            /* This will never execute */
            break;
        }
    } else {
        switch (len_burst) {
        case 1:
            *data = 0;
            *data |= 0xFFFFFF00;
            break;
        case 2:
            *data = 0;
            *data |= 0xFFFF0000;
            break;
        case 3:
            *data = 0;
            *data |= 0xFF000000;
            break;
        case 4:
            *data = 0;
            break;
        default:
            /* This will never execute */
            break;
        }    
    }
    if (qspi_bus->us_tx_bytes < len_burst)
        qspi_bus->us_tx_bytes = 0;
    else 
        qspi_bus->us_tx_bytes -= len_burst;
}

/****************************************************************************/
/**
*
* QSPI bus fill TX FIFO function. 
*
* @param    qspi_bus     - QSPI bus handle
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_fill_tx_fifo(cyg_qspi_xc7z_bus_t    *qspi_bus, int max)
{
    entry_debug();
    cyg_uint32 data = 0;
    cyg_uint32 val  = 0;
    int count = 0;
    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET, val);
    while ((!(val & XQSPIPS_IXR_TXFULL_MASK)) && (qspi_bus->us_tx_bytes > 0) && (count < max)) {
    	count++;
	if (qspi_bus->us_tx_bytes < 4) {
	    int tp = qspi_bus->us_tx_bytes;
            qspi_xc7z_copy_write_data(qspi_bus, &data, qspi_bus->us_tx_bytes);
	    if (tp == 1) {
		    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_TXD_01_OFFSET, data);
	    } else if (tp == 2) {
		    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_TXD_10_OFFSET, data);
	    } else {
		    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_TXD_11_OFFSET, data);
	    }
        } else {
            qspi_xc7z_copy_write_data(qspi_bus, &data, 4);
	    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_TXD_00_OFFSET, data);

        }
        HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET,      val);
   }
}

/****************************************************************************/
/**
*
* QSPI bus send flash memory instruction function (for Zynq specific QSPI HW). 
*
* @param    qspi_bus     - QSPI bus handle
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_send_instruction(cyg_qspi_xc7z_bus_t    *qspi_bus)
{
    entry_debug();
    cyg_uint32 index;
    cyg_uint8  instruction;
    cyg_uint32 data = 0;
    
    qspi_bus->uc_tx_instr = 0;
    
    if ((!qspi_bus->lp_tx_buf) || (!qspi_bus->us_tx_bytes))
        return;
        
    if (*((cyg_uint8 *)qspi_bus->lp_tx_buf) == 0)
        return;
        
    instruction  = *((cyg_uint8 *)qspi_bus->lp_tx_buf);

    for (index = 0; index < ARRAY_SIZE(qpifFlashInst); index++)
        if (instruction == qpifFlashInst[index].OpCode)
            break;    
    
    if (index == ARRAY_SIZE(qpifFlashInst)) {
        diag_printf("illegal instruction %02X\n",instruction);
        return;
    }
        
    qspi_bus->uc_tx_instr = qpifFlashInst[index].OpCode;
    
    /* Get the instruction */
    qspi_xc7z_copy_write_data(qspi_bus, &data, qpifFlashInst[index].InstSize);

    /* Write the instruction to LSB of the FIFO. The core is
     * designed such that it is not necessary to check whether the
     * write FIFO is full before writing. However, write would be
     * delayed if the user tries to write when write FIFO is full
     */
    HAL_WRITE_UINT32(qspi_bus->base + qpifFlashInst[index].TxOffset, data);
}

/****************************************************************************/
/**
*
* QSPI bus transfer with IRQ function. 
*
* @param    qspi_bus     - QSPI bus handle
* @param    count        - Number of bytes to transmit.
* @param    tx_data      - Pointer to TX buffer.
* @param    rx_data      - Pointer to RX buffer.
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_transfer(cyg_qspi_xc7z_device_t *dev,
                  cyg_uint32             count,
                  const cyg_uint8       *tx_data,
                  cyg_uint8             *rx_data)
{	
    entry_debug();
    cyg_qspi_xc7z_bus_t *qspi_bus = (cyg_qspi_xc7z_bus_t *)dev->qspi_device.spi_bus;
    cyg_uint32 val;
    
    // Enable device
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_ER_OFFSET, XQSPIPS_ER_ENABLE_MASK);
    
    // Enable manual start
    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,  val);
    val |= XQSPIPS_CR_MANSTRTEN_MASK | XQSPIPS_CR_SSFORCE_MASK;
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET, val);

    // Set tx buf pointer and counter
    if (NULL != tx_data)
        HAL_DCACHE_STORE(tx_data, count);

    // Set rx buf pointer and counter
    if (NULL != rx_data)
        HAL_DCACHE_FLUSH(rx_data, count);
        
    // Send first instruction
    if(qspi_bus->uc_tx_instr == 0) 	 
	    qspi_xc7z_send_instruction(qspi_bus);
    {
        
       if ((qspi_bus->us_tx_bytes) && 
        ((qspi_bus->uc_tx_instr != XQSPIPS_FLASH_OPCODE_FAST_READ) ||
         (qspi_bus->uc_tx_instr != XQSPIPS_FLASH_OPCODE_DUAL_READ) ||
         (qspi_bus->uc_tx_instr != XQSPIPS_FLASH_OPCODE_QUAD_READ) )) 
            qspi_xc7z_fill_tx_fifo(qspi_bus,count);
            
        // Enable the QSPI int events we are interested in
        HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_IER_OFFSET,
                        XQSPIPS_IXR_TXOW_MASK | XQSPIPS_IXR_MODF_MASK); 
                        
        HAL_READ_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,     val);
        val |= XQSPIPS_CR_MANSTRT_MASK;
        HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,    val);
        
        cyg_drv_mutex_lock(&qspi_bus->transfer_mx);
        {
            qspi_bus->transfer_end = false;

            // Unmask the SPI int
            cyg_drv_interrupt_unmask(qspi_bus->interrupt_number);
            
            // Wait for its completion
            cyg_drv_dsr_lock();
            {
                while (!qspi_bus->transfer_end)
                    cyg_drv_cond_wait(&qspi_bus->transfer_cond);
                
            }
            cyg_drv_dsr_unlock();
        }
        
        cyg_drv_mutex_unlock(&qspi_bus->transfer_mx);

    }
}

/****************************************************************************/
/**
*
* QSPI bus transfer function in poll mode without IRQ. 
*
* @param    qspi_bus     - QSPI bus handle
* @param    count        - Number of bytes to transmit.
* @param    tx_data      - Pointer to TX buffer.
* @param    rx_data      - Pointer to RX buffer.
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_transfer_polled(cyg_qspi_xc7z_device_t *dev,
                         cyg_uint32             count,
                         const cyg_uint8       *tx_data,
                         cyg_uint8             *rx_data)
{
    entry_debug();
    cyg_uint32 val;
    cyg_uint32 data;
    cyg_qspi_xc7z_bus_t *qspi_bus = (cyg_qspi_xc7z_bus_t *)dev->qspi_device.spi_bus;
   
    // Set tx buf pointer and counter
    if (NULL != tx_data)
      HAL_DCACHE_STORE(tx_data, count);
    // Set rx buf pointer and counter
    if (NULL != rx_data)
      HAL_DCACHE_FLUSH(rx_data, count);

    while(qspi_bus->us_rx_bytes)
    {
/*  if ((qspi_bus->us_tx_bytes) &&
        ((qspi_bus->uc_tx_instr != XQSPIPS_FLASH_OPCODE_FAST_READ) ||
         (qspi_bus->uc_tx_instr != XQSPIPS_FLASH_OPCODE_DUAL_READ) ||
         (qspi_bus->uc_tx_instr != XQSPIPS_FLASH_OPCODE_QUAD_READ))) {*/
	qspi_xc7z_fill_tx_fifo(qspi_bus, count);
//    }
        
        HAL_READ_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,     val);

	val |= XQSPIPS_CR_MANSTRT_MASK | XQSPIPS_CR_SSFORCE_MASK;
        HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET,    val);
        
        /* Read out the data from the RX FIFO */

        HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET, val);

        while ((val & XQSPIPS_IXR_RXNEMPTY_MASK) && (qspi_bus->us_rx_bytes > 0) ) {
            
            HAL_READ_UINT32(qspi_bus->base + XQSPIPS_RXD_OFFSET,    data);
		if (qspi_bus->lp_rx_buf != NULL) {

            if (qspi_bus->us_rx_bytes < 4)
                qspi_xc7z_copy_read_data(qspi_bus, data, qspi_bus->us_rx_bytes);
            else
                qspi_xc7z_copy_read_data(qspi_bus, data, 4);
            } else {
	    	qspi_bus->us_rx_bytes -=  (qspi_bus->us_rx_bytes < 4) ? qspi_bus->us_rx_bytes : 4;
	    }
            HAL_READ_UINT32(qspi_bus->base + XQSPIPS_SR_OFFSET,     val);
    }
    }

    
}

/****************************************************************************/
/**
*
* QSPI bus begin transaction function. 
*
* @param    dev          - QSPI device handle
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_transaction_begin(cyg_spi_device *dev)
{
    entry_debug();
    cyg_qspi_xc7z_device_t *xc7z_qspi_dev = (cyg_qspi_xc7z_device_t *) dev;
    cyg_qspi_xc7z_bus_t *qspi_bus =
      (cyg_qspi_xc7z_bus_t *)xc7z_qspi_dev->qspi_device.spi_bus;
    cyg_uint32 val;

    if (!xc7z_qspi_dev->init)
    {
        xc7z_qspi_dev->init = true;
        qspi_xc7z_calc_bratediv(xc7z_qspi_dev);
    }

    // Configure SPI channel 0 - this is the only channel we
    // use for all devices since we drive chip selects manually
    HAL_READ_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET, val);
    
    val &= (~XQSPIPS_CR_CPOL_MASK) & (~XQSPIPS_CR_CPHA_MASK);

    if (1 == xc7z_qspi_dev->cl_pol)
        val |= XQSPIPS_CR_CPOL_MASK;

    if (1 == xc7z_qspi_dev->cl_pha)
        val |= XQSPIPS_CR_CPHA_MASK;

    // Write new settings
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_CR_OFFSET, val);
}

/****************************************************************************/
/**
*
* QSPI bus transaction transfer function. 
*
* @param    dev          - QSPI device handle
* @param    polled       - Poll mode flash: 1-Polled, 0-IRQ
* @param    count        - Number of bytes to transmit.
* @param    tx_data      - Pointer to TX buffer.
* @param    rx_data      - Pointer to RX buffer.
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_transaction_transfer(cyg_spi_device  *dev,
                              cyg_bool         polled,
                              cyg_uint32       count,
                              const cyg_uint8 *tx_data,
                              cyg_uint8       *rx_data,
                              cyg_bool         drop_cs)
{
    entry_debug();
    cyg_qspi_xc7z_device_t *xc7z_qspi_dev = (cyg_qspi_xc7z_device_t *) dev;
    cyg_qspi_xc7z_bus_t *qspi_bus =
      (cyg_qspi_xc7z_bus_t *)xc7z_qspi_dev->qspi_device.spi_bus;

    // Setup poiner to buffers
    qspi_bus->lp_tx_buf = tx_data;
    qspi_bus->lp_rx_buf = rx_data;
    qspi_bus->us_tx_bytes = count;
    qspi_bus->us_rx_bytes = count;

    // Select the device if not already selected
    qspi_xc7z_start_transfer(xc7z_qspi_dev);
    
    // Perform the transfer
    if (polled)
        qspi_xc7z_transfer_polled(xc7z_qspi_dev, count, tx_data, rx_data);
    else
        qspi_xc7z_transfer(xc7z_qspi_dev, count, tx_data, rx_data);

    // Deselect the device if requested
    if (drop_cs)
        qspi_xc7z_drop_cs(xc7z_qspi_dev);
        
    // Clear pointers
    qspi_bus->lp_tx_buf = 0;
    qspi_bus->lp_rx_buf = 0;
    qspi_bus->us_tx_bytes = 0;
    qspi_bus->us_rx_bytes = 0;
}

/****************************************************************************/
/**
*
* QSPI bus adding zeros function. 
*
* @param    dev          - QSPI device handle
* @param    polled       - Poll mode flash: 1-Polled, 0-IRQ
* @param    count        - Number of bytes to transmit (Number of Ticks = count * PerByteClocks).
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_transaction_tick(cyg_spi_device *dev,
                          cyg_bool        polled,
                          cyg_uint32      count)
{
    entry_debug();
    cyg_qspi_xc7z_device_t *xc7z_qspi_dev = (cyg_qspi_xc7z_device_t *) dev;

    // Transfer count zeros to the device - we don't touch the
    // chip select, the device could be selected or deselected.
    // It is up to the device driver to decide in which state the
    // device will be ticked.

    while (count > 0)
    {
        int tcnt = count > sizeof(zeros) ? sizeof(zeros) : count;

        if (polled)
            qspi_xc7z_transfer_polled(xc7z_qspi_dev, tcnt,
                                     (const cyg_uint8 *) zeros, NULL);
        else
            qspi_xc7z_transfer(xc7z_qspi_dev, tcnt,
                              (const cyg_uint8 *) zeros, NULL);

        count -= tcnt;
    }
}

/****************************************************************************/
/**
*
* QSPI bus finalize transaction function. 
*
* @param    dev          - QSPI device handle
*
* @return   none
*
*****************************************************************************/
static void
qspi_xc7z_transaction_end(cyg_spi_device* dev)
{
    entry_debug();
    cyg_qspi_xc7z_device_t * xc7z_qspi_dev = (cyg_qspi_xc7z_device_t *)dev;
    cyg_qspi_xc7z_bus_t *qspi_bus =
      (cyg_qspi_xc7z_bus_t *)xc7z_qspi_dev->qspi_device.spi_bus;

    qspi_xc7z_drop_cs((cyg_qspi_xc7z_device_t *) dev);

    // Disable device
    HAL_WRITE_UINT32(qspi_bus->base + XQSPIPS_ER_OFFSET, 0x00);

}

/****************************************************************************/
/**
*
* QSPI bus get config function. 
*
* @param    dev          - QSPI device handle
* @param    key          - IO control number of key
* @param    buf          - Pointer to BUF
* @param    len          - Pointer to length
*
* @return   Status of operation: ENOERR - all good, -EINVAL - fail to find IO control number
*
*****************************************************************************/
static int
qspi_xc7z_get_config(cyg_spi_device *dev,
                    cyg_uint32      key,
                    void           *buf,
                    cyg_uint32     *len)
{
    entry_debug();
    cyg_qspi_xc7z_device_t *xc7z_qspi_dev = (cyg_qspi_xc7z_device_t *) dev;

    switch (key)
    {
        case CYG_IO_GET_CONFIG_SPI_CLOCKRATE:
        {
            if (*len != sizeof(cyg_uint32))
                return -EINVAL;
            else
            {
                cyg_uint32 *cl_brate = (cyg_uint32 *)buf;
                *cl_brate = xc7z_qspi_dev->cl_brate;
            }
        }
        break;
        default:
            return -EINVAL;
    }
    return ENOERR;
}

/****************************************************************************/
/**
*
* QSPI bus set config function. 
*
* @param    dev          - QSPI device handle
* @param    key          - IO control number of key
* @param    buf          - Pointer to BUF
* @param    len          - Pointer to length
*
* @return   Status of operation: ENOERR - all good, -EINVAL - fail to find IO control number
*
*****************************************************************************/
static int
qspi_xc7z_set_config(cyg_spi_device *dev,
                    cyg_uint32      key,
                    const void     *buf,
                    cyg_uint32     *len)
{
    entry_debug();
    cyg_qspi_xc7z_device_t *xc7z_qspi_dev = (cyg_qspi_xc7z_device_t *) dev;

    switch (key)
    {
        case CYG_IO_SET_CONFIG_SPI_CLOCKRATE:
        {
            if (*len != sizeof(cyg_uint32))
                return -EINVAL;
            else
            {
                cyg_uint32 cl_brate     = *((cyg_uint32 *)buf);
                cyg_uint32 old_cl_brate = xc7z_qspi_dev->cl_brate;

                xc7z_qspi_dev->cl_brate = cl_brate;

                if (!qspi_xc7z_calc_bratediv(xc7z_qspi_dev))
                {
                    xc7z_qspi_dev->cl_brate = old_cl_brate;
                    qspi_xc7z_calc_bratediv(xc7z_qspi_dev);
                    return -EINVAL;
                }
            }
        }
        break;
        default:
            return -EINVAL;
    }
    return ENOERR;
}

// -------------------------------------------------------------------------
// EOF qspi_xc7z.c
