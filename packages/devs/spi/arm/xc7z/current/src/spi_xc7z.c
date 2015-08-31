//==========================================================================
//
//      spi_xc7z.c
//
//      SPI driver for Xilinx Zynq
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
// Author(s):    Antmicro Ltd <www.antmicro.com>, Artur Łącki <alacki93@gmail.com>
// Date:         2015-08-31
// Purpose:
// Description:  SPI driver for Xilinx Zynq
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <stdio.h>
#include <cyg/io/spi_xc7z.h>
#include <pkgconf/devs_spi_arm_xc7z.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/kernel/kapi.h>

//SPI registers
#define XSPI_REG_BASE_0         0xE0006000
#define XSPI_REG_BASE_1         0xE0007000
#define XSPI_INTR_STATUS_REG    0x00000004
#define XSPI_INTRPT_EN_REG      0x00000008
#define XSPI_INTRPT_DIS_REG     0x0000000C
#define XSPI_EN_REG             0x00000014
#define XSPI_TX_DATA_REG        0x0000001C
#define XSPI_RX_DATA_REG        0x00000020
#define XSPI_RX_THRES_REG       0x0000002C

//SLCR registers
#define XSLR_BASE               0xF8000000
#define XSLR_SLCR_LOCK          0x00000004
#define XSLR_SLCR_UNLOCK        0x00000008
#define XSLR_APER_CLK_CTRL      0x0000012C
#define XSLR_SPI_CLK_CTRL       0x00000158
#define XSLR_SPI_RST_CTRL       0x0000021C

//Interrupt masks
#define SPI_IXR_TXOW_MASK	    0x00000004 /* SPI TX FIFO Overwater */
#define SPI_IXR_TXFULL_MASK	    0x00000008
#define SPI_IXR_MODF_MASK	    0x00000002 /* SPI Mode Fault */
#define SPI_IXR_RXNEMTY_MASK    0x00000010 /* SPI RX FIFO Not Empty */
#define SPI_IXR_TXFULL_MASK	    0x00000008 /* SPI TX Full */
#define SPI_IXR_ALL_MASK	    0x0000007F /* SPI all interrupts */

static void spi_xc7z_init_bus(cyg_spi_xc7z_bus_t *spi_bus);

static void spi_xc7z_transaction_begin(cyg_spi_device *dev);

static void spi_xc7z_transaction_transfer(  cyg_spi_device  *dev,
                                            cyg_bool         polled,
                                            cyg_uint32       count,
                                            cyg_uint8       *tx_data,
                                            cyg_uint8       *rx_data,
                                            cyg_bool         drop_cs);

static  void spi_xc7z_transaction_tick( cyg_spi_device *dev,
                                        cyg_bool        polled,
                                        cyg_uint32      count);

static void spi_xc7z_transaction_end(cyg_spi_device* dev);

static int spi_xc7z_get_config(cyg_spi_device *dev,
                                cyg_uint32      key,
                                void           *buf,
                                cyg_uint32     *len);

static int spi_xc7z_set_config(cyg_spi_device *dev,
                                cyg_uint32      key,
                                const void     *buf,
                                cyg_uint32     *len);

static void spi_xc7z_start_transfer(cyg_spi_xc7z_device_t *dev);

static void spi_xc7z_set_cs(cyg_spi_xc7z_bus_t *spi_bus,int val);

static void spi_xc7z_transfer_polled(  cyg_spi_xc7z_device_t *dev,
                                        cyg_uint32             count,
                                        const cyg_uint8       *tx_data,
                                        cyg_uint8             *rx_data);

static void spi_xc7z_transfer(  cyg_spi_xc7z_device_t *dev,
                                cyg_uint32             count,
                                const cyg_uint8       *tx_data,
                                cyg_uint8             *rx_data);

static void spi_xc7z_drop_cs(cyg_spi_xc7z_device_t *dev);

static cyg_uint32 spi_xc7z_ISR(cyg_vector_t vector, cyg_addrword_t data);

static void spi_xc7z_DSR(cyg_vector_t vector, cyg_ucount32 count,
                         cyg_addrword_t data);

static cyg_bool spi_xc7z_calc_bratediv(cyg_spi_xc7z_device_t *dev);

static void zynq_slcr_unlock(void);
static void zynq_slcr_lock(void);
static void zynq_spi_reset(cyg_uint8 spi_number);
static void zynq_enable_spi_ref_clk(cyg_uint8 spi_number);
static void zynq_spi_disable(cyg_uint32 spi_base_reg);
static void zynq_spi_enable(cyg_uint32 spi_base_reg);
static cyg_uint32 zynq_read_and_clear_intr_status_reg(cyg_uint32 spi_base_reg);
static void zynq_intr_disable(cyg_uint32 spi_base_reg);
static void zynq_intr_enable(cyg_uint32 spi_base_reg);
static void zynq_spi_set_conf(cyg_uint32 spi_base_reg, cyg_uint8 spi_mode);
static void zynq_set_src_clk_divisor(cyg_uint32 divisor);
static void zynq_set_cs_clk_divisor(cyg_uint32 reg_base, cyg_uint32 divisor);
static void zynq_intr_setup(cyg_spi_xc7z_bus_t *spi_bus);
static void zynq_fill_tx_fifo(cyg_spi_xc7z_bus_t *spi_bus, cyg_uint32 max);
static void zynq_read_rx_fifo(cyg_spi_xc7z_bus_t *spi_bus, cyg_uint32 max);
static void zynq_set_spi_mode(cyg_spi_xc7z_device_t *spi_device);
static void zynq_set_clk_ph(cyg_spi_xc7z_device_t *spi_device);
static void zynq_set_clk_pol(cyg_spi_xc7z_device_t *spi_device);
static void zynq_set_cs_num(cyg_spi_xc7z_device_t *spi_device);
static void zynq_clear_rx_fifo(cyg_spi_xc7z_bus_t *spi_bus);
static void zynq_set_rx_thres(cyg_uint32 reg_base, cyg_uint32 size);

#ifdef CYGHWR_DEVS_SPI_ARM_XC7Z_BUS0
cyg_spi_xc7z_bus_t cyg_spi_xc7z_bus0  = {
    .spi_bus.spi_transaction_begin    = spi_xc7z_transaction_begin,
    .spi_bus.spi_transaction_transfer = spi_xc7z_transaction_transfer,
    .spi_bus.spi_transaction_tick     = spi_xc7z_transaction_tick,
    .spi_bus.spi_transaction_end      = spi_xc7z_transaction_end,
    .spi_bus.spi_get_config           = spi_xc7z_get_config,
    .spi_bus.spi_set_config           = spi_xc7z_set_config,
    .interrupt_number                 = CYGNUM_HAL_INTERRUPT_SPI0,
    .base                             = XSPI_REG_BASE_0,
};
CYG_SPI_DEFINE_BUS_TABLE(cyg_spi_xc7z_device_t, 1);
#endif

#ifdef CYGHWR_DEVS_SPI_ARM_XC7Z_BUS1
cyg_spi_xc7z_bus_t cyg_spi_xc7z_bus1  = {
    .spi_bus.spi_transaction_begin    = spi_xc7z_transaction_begin,
    .spi_bus.spi_transaction_transfer = spi_xc7z_transaction_transfer,
    .spi_bus.spi_transaction_tick     = spi_xc7z_transaction_tick,
    .spi_bus.spi_transaction_end      = spi_xc7z_transaction_end,
    .spi_bus.spi_get_config           = spi_xc7z_get_config,
    .spi_bus.spi_set_config           = spi_xc7z_set_config,
    .interrupt_number                 = CYGNUM_HAL_INTERRUPT_SPI1,
    .base                             = XSPI_REG_BASE_1,
};
CYG_SPI_DEFINE_BUS_TABLE(cyg_spi_xc7z_device_t, 2);
#endif

void CYGBLD_ATTRIB_C_INIT_PRI(CYG_INIT_BUS_SPI)
cyg_spi_xc7z_bus_init(void){
#ifdef CYGHWR_DEVS_SPI_ARM_XC7Z_BUS0
    diag_printf("[LOG] SPI0 initialization\n");
    spi_xc7z_init_bus(&cyg_spi_xc7z_bus0);
#endif
#ifdef CYGHWR_DEVS_SPI_ARM_XC7Z_BUS1
    diag_printf("[LOG] SPI1 initialization\n");
    spi_xc7z_init_bus(&cyg_spi_xc7z_bus1);
#endif
}

static void zynq_slcr_unlock(void){
    cyg_uint32 reg = 0x0000DF0D;
    HAL_WRITE_UINT32(XSLR_BASE + XSLR_SLCR_UNLOCK, reg);
}

static void spi_xc7z_init_bus(cyg_spi_xc7z_bus_t *spi_bus)
{
    cyg_uint32 spi_base_reg = spi_bus->base;
    cyg_uint32 spi_number;
    spi_bus->transfer_end = true;

    if (spi_base_reg == XSPI_REG_BASE_0) spi_number = 0;
    else spi_number = 1;

    zynq_intr_setup(spi_bus);
    zynq_slcr_unlock();
    zynq_spi_reset(spi_number);
    zynq_enable_spi_ref_clk(spi_number);
    zynq_slcr_lock();
    zynq_spi_disable(spi_base_reg);
    zynq_intr_disable(spi_base_reg);
    zynq_intr_enable(spi_base_reg);
    zynq_read_and_clear_intr_status_reg(spi_base_reg);
    zynq_spi_set_conf(spi_base_reg, spi_number);
    zynq_spi_enable(spi_base_reg);
}

static void spi_xc7z_transaction_begin(cyg_spi_device *dev){
    cyg_uint32 reg;
    cyg_spi_xc7z_device_t *xc7z_spi_dev = (cyg_spi_xc7z_device_t *) dev;
    cyg_spi_xc7z_bus_t *spi_bus =
        (cyg_spi_xc7z_bus_t *)xc7z_spi_dev->spi_device.spi_bus;
    zynq_spi_disable(spi_bus->base);
    zynq_set_spi_mode(xc7z_spi_dev);
    zynq_set_src_clk_divisor(xc7z_spi_dev->src_divisor);
    zynq_set_cs_clk_divisor(spi_bus->base, xc7z_spi_dev->cs_divisor);
    zynq_set_clk_ph(xc7z_spi_dev);
    zynq_set_clk_pol(xc7z_spi_dev);
    zynq_set_cs_num(xc7z_spi_dev);

    if (!xc7z_spi_dev->init)
    {
        xc7z_spi_dev->init = true;
    }

    zynq_spi_enable(spi_bus->base);
    zynq_clear_rx_fifo(spi_bus);
    zynq_read_and_clear_intr_status_reg(spi_bus->base);
}

static void spi_xc7z_transaction_transfer(  cyg_spi_device  *dev,
                                            cyg_bool         polled,
                                            cyg_uint32       count,
                                            cyg_uint8       *tx_data,
                                            cyg_uint8       *rx_data,
                                            cyg_bool         drop_cs)
{
    cyg_spi_xc7z_device_t *xc7z_spi_dev = (cyg_spi_xc7z_device_t *) dev;
    cyg_spi_xc7z_bus_t *spi_bus =
        (cyg_spi_xc7z_bus_t *)xc7z_spi_dev->spi_device.spi_bus;

    // Setup poiner to buffers
    spi_bus->lp_tx_buf = tx_data;
    spi_bus->lp_rx_buf = rx_data;
    spi_bus->us_tx_bytes = count;
    spi_bus->us_rx_bytes = count;

    spi_xc7z_start_transfer(xc7z_spi_dev);

    spi_bus->transfer_cycles = count/128;
    if ((count % 128) != 0) spi_bus->transfer_cycles++;

    // Perform the transfer
    if (polled)
        spi_xc7z_transfer_polled(xc7z_spi_dev, count, tx_data, rx_data);
    else{
        spi_xc7z_transfer(xc7z_spi_dev, count, tx_data, rx_data);
    }

    // Deselect the device if requested
    if (drop_cs)
        spi_xc7z_drop_cs(xc7z_spi_dev);

    // Clear pointers
    spi_bus->lp_tx_buf = NULL;
    spi_bus->lp_rx_buf = NULL;
    spi_bus->us_tx_bytes = NULL;
    spi_bus->us_rx_bytes = NULL;
}

static  void spi_xc7z_transaction_tick( cyg_spi_device *dev,
                                        cyg_bool        polled,
                                        cyg_uint32      count)
{
    cyg_spi_xc7z_device_t *xc7z_spi_dev = (cyg_spi_xc7z_device_t *) dev;

    // Transfer count zeros to the device - we don't touch the
    // chip select, the device could be selected or deselected.
    // It is up to the device driver to decide in which state the
    // device will be ticked.
    spi_xc7z_start_transfer(xc7z_spi_dev);
    if (polled) spi_xc7z_transfer_polled(xc7z_spi_dev, count, NULL, NULL);
    else spi_xc7z_transfer(xc7z_spi_dev, count, NULL,NULL);
}

static void spi_xc7z_transaction_end(cyg_spi_device* dev){
    spi_xc7z_drop_cs((cyg_spi_xc7z_device_t *) dev);

}

static int spi_xc7z_get_config(cyg_spi_device *dev,
                                cyg_uint32      key,
                                void           *buf,
                                cyg_uint32     *len)
{
    cyg_spi_xc7z_device_t *xc7z_spi_dev = (cyg_spi_xc7z_device_t *) dev;

    switch (key)
    {
        case CYG_IO_GET_CONFIG_SPI_CLOCKRATE:
        {
            if (*len != sizeof(cyg_uint32))
                return -EINVAL;
            else
            {
                cyg_uint32 *cl_brate = (cyg_uint32 *)buf;
                *cl_brate = xc7z_spi_dev->cl_brate;
            }
        }
        break;
        default:
            return -EINVAL;
    }
    return ENOERR;
}

static int spi_xc7z_set_config(cyg_spi_device *dev,
                                cyg_uint32      key,
                                const void     *buf,
                                cyg_uint32     *len)
{
    cyg_spi_xc7z_device_t *xc7z_spi_dev = (cyg_spi_xc7z_device_t *) dev;

    switch (key)
    {
        case CYG_IO_SET_CONFIG_SPI_CLOCKRATE:
        {
            if (*len != sizeof(cyg_uint32))
                return -EINVAL;
            else{
                cyg_uint32 cl_brate     = *((cyg_uint32 *)buf);
                cyg_uint32 old_cl_brate = xc7z_spi_dev->cl_brate;

                xc7z_spi_dev->cl_brate = cl_brate;

                if (!spi_xc7z_calc_bratediv(xc7z_spi_dev))
                {
                    xc7z_spi_dev->cl_brate = old_cl_brate;
                    spi_xc7z_calc_bratediv(xc7z_spi_dev);
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

static void spi_xc7z_start_transfer(cyg_spi_xc7z_device_t *dev)
{
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)dev->spi_device.spi_bus;

    if (spi_bus->cs_up) return;
    // Force minimal delay between two transfers - in case two tranrsfers
    // follow each other w/o delay, then we have to wait here in order for
    // the peripheral device to detect cs transition from inactive to active.
    CYGACC_CALL_IF_DELAY_US(dev->tr_bt_udly);

    // Raise CS
    spi_xc7z_set_cs(spi_bus, 0);
    CYGACC_CALL_IF_DELAY_US(dev->cs_up_udly);

    spi_bus->cs_up = true;
}

static void spi_xc7z_set_cs(cyg_spi_xc7z_bus_t *spi_bus, int val)
{
    cyg_uint32 reg;
    HAL_READ_UINT32(spi_bus->base, reg);
    reg &= ~(0b1111 << 10);
    reg |= (val << 10);
    HAL_WRITE_UINT32(spi_bus->base, reg);
}

static void spi_xc7z_transfer_polled(  cyg_spi_xc7z_device_t *dev,
                                        cyg_uint32             count,
                                        const cyg_uint8       *tx_data,
                                        cyg_uint8             *rx_data)
{
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)dev->spi_device.spi_bus;
    cyg_uint32 last_transfer = spi_bus->transfer_cycles-1;

    while(spi_bus->cycles_sended < spi_bus->transfer_cycles){
        if (spi_bus->cycles_sended == last_transfer)
            count = count - last_transfer*128;

        zynq_fill_tx_fifo(spi_bus, count);
        zynq_read_rx_fifo(spi_bus, count);
        zynq_read_and_clear_intr_status_reg(spi_bus->base);
        spi_bus->cycles_sended++;
    }
}


static void spi_xc7z_transfer(  cyg_spi_xc7z_device_t *dev,
                                cyg_uint32             count,
                                const cyg_uint8       *tx_data,
                                cyg_uint8             *rx_data)
{
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)dev->spi_device.spi_bus;
    cyg_uint32 buf_size;
    cyg_uint32 last_transfer = spi_bus->transfer_cycles-1;

    if (spi_bus->transfer_cycles == 1) buf_size = spi_bus->us_tx_bytes;
    else buf_size = 128;

    spi_bus->cycles_sended = 0;

    if (count > 128) zynq_set_rx_thres(spi_bus->base, 127);

    while(spi_bus->cycles_sended < spi_bus->transfer_cycles){
        if (spi_bus->cycles_sended == last_transfer) {
            buf_size = count - last_transfer*128;
            zynq_set_rx_thres(spi_bus->base, spi_bus->us_tx_bytes-last_transfer*128-1);
        }
        zynq_fill_tx_fifo(spi_bus, buf_size);
        while(!cyg_drv_mutex_lock(&spi_bus->transfer_mx));
        {
            cyg_thread_delay(10);
            spi_bus->transfer_end = false;

            // Unmask the SPI int
            cyg_drv_interrupt_unmask(spi_bus->interrupt_number);

            // Wait for its completion
            cyg_drv_dsr_lock();
            {
                while (!spi_bus->transfer_end)
                    cyg_drv_cond_wait(&spi_bus->transfer_cond);
            }
            cyg_drv_dsr_unlock();
        }

    spi_bus->cycles_sended++;
    cyg_drv_mutex_unlock(&spi_bus->transfer_mx);
    }
}

static void spi_xc7z_drop_cs(cyg_spi_xc7z_device_t *dev){
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)dev->spi_device.spi_bus;
    cyg_uint32 reg;

    if (!spi_bus->cs_up)
       return;

    // Drop CS

    CYGACC_CALL_IF_DELAY_US(dev->cs_dw_udly);
    HAL_READ_UINT32(spi_bus->base, reg);
    reg = reg | (0b1111 << 10);
    HAL_WRITE_UINT32(spi_bus->base, reg);
    spi_bus->cs_up = false;
}

static void zynq_slcr_lock(void){
    cyg_uint32 reg = 0x0000767B;
    HAL_WRITE_UINT32(XSLR_BASE + XSLR_SLCR_LOCK, reg);
}

static void zynq_spi_reset(cyg_uint8 spi_number){
    cyg_uint32 reg;
    HAL_READ_UINT32(XSLR_BASE + XSLR_SPI_RST_CTRL, reg);
    spi_number += 1;
    //enable reset
    HAL_WRITE_UINT32(XSLR_BASE + XSLR_SPI_RST_CTRL, reg | spi_number);
    //wait some delay (?)
    cyg_thread_delay(5);
    //disable reset
    reg &= ~(spi_number);
    HAL_WRITE_UINT32(XSLR_BASE + XSLR_SPI_RST_CTRL, reg);
}

static void zynq_enable_spi_ref_clk(cyg_uint8 spi_number){
    cyg_uint32 reg;
    HAL_READ_UINT32(XSLR_BASE + XSLR_SPI_CLK_CTRL, reg);

    reg |= (spi_number+1);
    HAL_WRITE_UINT32(XSLR_BASE + XSLR_SPI_CLK_CTRL, reg);
    HAL_READ_UINT32(XSLR_BASE+XSLR_APER_CLK_CTRL, reg);
    if (spi_number == 0) reg |= 0x00004000;
    else reg |= 0x00008000;
    HAL_WRITE_UINT32(XSLR_BASE+XSLR_APER_CLK_CTRL, reg);
}

static void zynq_spi_disable(cyg_uint32 spi_base_reg){
    cyg_uint32 reg = 0x00000000;
    HAL_WRITE_UINT32(spi_base_reg + XSPI_EN_REG, reg);
}

static void zynq_spi_enable(cyg_uint32 spi_base_reg){
    cyg_uint32 reg;
    reg = 0xFFFFFFFF;
    HAL_WRITE_UINT32(spi_base_reg + XSPI_EN_REG, reg);
}

static cyg_uint32 zynq_read_and_clear_intr_status_reg(cyg_uint32 spi_base_reg){
    cyg_uint32 reg, tmp;
    HAL_READ_UINT32(spi_base_reg+XSPI_INTR_STATUS_REG, reg);
    HAL_WRITE_UINT32(spi_base_reg+XSPI_INTR_STATUS_REG, reg);
    return reg;
}

static void zynq_intr_disable(cyg_uint32 spi_base_reg){
    cyg_uint32 reg = 0x000000FF;
    HAL_WRITE_UINT32(spi_base_reg+XSPI_INTRPT_DIS_REG, reg);
}

static void zynq_intr_enable(cyg_uint32 spi_base_reg){
    cyg_uint32 reg;
    if (spi_base_reg == XSPI_REG_BASE_1) reg = SPI_IXR_RXNEMTY_MASK;
    else reg = SPI_IXR_TXOW_MASK;
    HAL_WRITE_UINT32(spi_base_reg+XSPI_INTRPT_EN_REG, reg);
}

//SPI modes:
//1 - master
//0 - slave
static void zynq_spi_set_conf(cyg_uint32 spi_base_reg, cyg_uint8 spi_mode){
    //TODO: slave selecting
    cyg_uint32 reg;

    reg = (0x0002000E & ~0x38); // set default mask
    HAL_WRITE_UINT32(spi_base_reg, reg);
    HAL_READ_UINT32(spi_base_reg, reg);
}


static void zynq_set_src_clk_divisor(cyg_uint32 divisor){
    cyg_uint32 reg;
    divisor <<= 8;
    HAL_READ_UINT32(XSLR_BASE + XSLR_SPI_CLK_CTRL, reg);
    reg &= 0x000000FF;
    reg |= divisor;
    zynq_slcr_unlock();
    HAL_WRITE_UINT32(XSLR_BASE + XSLR_SPI_CLK_CTRL, reg);
    zynq_slcr_lock();
}

static void zynq_set_cs_clk_divisor(cyg_uint32 reg_base, cyg_uint32 divisor){
    divisor <<= 3;
    cyg_uint32 reg;
    HAL_READ_UINT32(reg_base, reg);
    reg &= ~0x38;
    reg |= divisor;
    HAL_WRITE_UINT32(reg_base, reg);
}

static void zynq_intr_setup(cyg_spi_xc7z_bus_t *spi_bus){
    // Create and attach SPI interrupt object
    cyg_drv_interrupt_create(spi_bus->interrupt_number,
                             4,
                             (cyg_addrword_t)spi_bus,
                             spi_xc7z_ISR,
                             spi_xc7z_DSR,
                             &spi_bus->spi_interrupt_handle,
                             &spi_bus->spi_interrupt);

    cyg_drv_interrupt_attach(spi_bus->spi_interrupt_handle);
    cyg_drv_interrupt_mask(spi_bus->interrupt_number);

    // Init transfer mutex and condition
    cyg_drv_mutex_init(&spi_bus->transfer_mx);
    cyg_drv_cond_init(&spi_bus->transfer_cond,
                      &spi_bus->transfer_mx);

    // Init flags
    spi_bus->transfer_end = true;
    spi_bus->cs_up        = false;
}

static cyg_uint32 spi_xc7z_ISR(cyg_vector_t vector, cyg_addrword_t data){
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)data;
    cyg_uint32 return_cond = CYG_ISR_HANDLED;
    cyg_uint32 stat = zynq_read_and_clear_intr_status_reg(spi_bus->base);
    cyg_uint32 reg;

    if (stat & SPI_IXR_MODF_MASK){
        cyg_drv_interrupt_mask(vector);
        return_cond  |= CYG_ISR_CALL_DSR;
    }

    cyg_drv_interrupt_mask(vector);
    return_cond  |= CYG_ISR_CALL_DSR;
    cyg_drv_interrupt_acknowledge(vector);
    return return_cond;

}

static void spi_xc7z_DSR(cyg_vector_t vector, cyg_ucount32 count,
                         cyg_addrword_t data){
    cyg_uint32 buf_size;
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)data;
    cyg_uint32 last_transfer = spi_bus->transfer_cycles-1;

    if (spi_bus->cycles_sended == last_transfer) buf_size = spi_bus->us_rx_bytes - last_transfer*128;
    else buf_size = 128;

    zynq_read_rx_fifo(spi_bus, buf_size);

    // Transfer ended
    spi_bus->transfer_end = true;
    cyg_drv_cond_signal(&spi_bus->transfer_cond);
}

static void zynq_fill_tx_fifo(cyg_spi_xc7z_bus_t *spi_bus, cyg_uint32 max){
    int count = 0;
    cyg_uint8 *buf_pos;

    if (spi_bus->lp_tx_buf == NULL){
        while (count < max){
            HAL_WRITE_UINT32(spi_bus->base+XSPI_TX_DATA_REG, 0);
            count++;
        }
    }

    else{
        buf_pos = spi_bus->lp_tx_buf + spi_bus->cycles_sended*128;
        while (count < max){
            HAL_WRITE_UINT32(spi_bus->base+XSPI_TX_DATA_REG, buf_pos[count]);
            count++;
        }
    }

}

static void zynq_read_rx_fifo(cyg_spi_xc7z_bus_t *spi_bus, cyg_uint32 max){
    if (spi_bus->lp_rx_buf == NULL) return;
    int count = 0;
    cyg_uint8 *buf_pos = spi_bus->lp_rx_buf + spi_bus->cycles_sended*128;
    while (count < max) {
        HAL_READ_UINT32(spi_bus->base+XSPI_RX_DATA_REG, buf_pos[count]);
        count++;
   }
}

static void zynq_set_spi_mode(cyg_spi_xc7z_device_t *spi_device){
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)spi_device->spi_device.spi_bus;
    cyg_uint32 reg;
    HAL_READ_UINT32(spi_bus->base, reg);
    reg &= ~(0x1);
    reg |= spi_device->master_mode;
    HAL_WRITE_UINT32(spi_bus->base, reg);
}

static void zynq_set_clk_ph(cyg_spi_xc7z_device_t *spi_device){
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)spi_device->spi_device.spi_bus;
    cyg_uint32 reg;
    HAL_READ_UINT32(spi_bus->base, reg);
    reg &= ~(0b100);
    reg |= (spi_device->cl_pha << 2);
    HAL_WRITE_UINT32(spi_bus->base, reg);
}

static void zynq_set_clk_pol(cyg_spi_xc7z_device_t *spi_device){
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)spi_device->spi_device.spi_bus;
    cyg_uint32 reg;
    HAL_READ_UINT32(spi_bus->base, reg);
    reg &= ~(0b10);
    reg |= (spi_device->cl_pol << 1);
    HAL_WRITE_UINT32(spi_bus->base, reg);
}

static void zynq_set_cs_num(cyg_spi_xc7z_device_t *spi_device){
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)spi_device->spi_device.spi_bus;
    cyg_uint32 reg;
    HAL_READ_UINT32(spi_bus->base, reg);
    reg &= ~(0b1111 << 10);
    reg |= (spi_device->cs_num << 10);
    HAL_WRITE_UINT32(spi_bus->base, reg);
}

static void zynq_clear_rx_fifo(cyg_spi_xc7z_bus_t *spi_bus){
    int i = 0;
    cyg_uint32 reg;
    while (i < 128){
        HAL_READ_UINT32(spi_bus->base + XSPI_RX_DATA_REG, reg);
        i++;
    }
}

static void zynq_set_rx_thres(cyg_uint32 reg_base, cyg_uint32 size){
    cyg_uint32 reg;
    HAL_WRITE_UINT32(reg_base+XSPI_RX_THRES_REG, size);
    HAL_READ_UINT32(reg_base+XSPI_RX_THRES_REG, reg);
    cyg_thread_delay(30);
}

static cyg_bool spi_xc7z_calc_bratediv(cyg_spi_xc7z_device_t *dev)
{
    cyg_spi_xc7z_bus_t *spi_bus = (cyg_spi_xc7z_bus_t *)dev->spi_device.spi_bus;
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

    HAL_READ_UINT32(spi_bus->base, reg);
    reg &= 0xFFFFFFC7;
    reg |= (baud_rate_val << 3);
    HAL_WRITE_UINT32(spi_bus->base, reg);

    return res;
}
