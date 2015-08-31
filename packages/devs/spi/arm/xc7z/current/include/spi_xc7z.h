#ifndef SPI_XC7Z_H
#define SPI_XC7Z_H
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
#include <cyg/io/spi.h>
#include <cyg/hal/drv_api.h>
#include <pkgconf/devs_spi_arm_xc7z.h>

typedef struct cyg_spi_xc7z_bus_s{
    cyg_spi_bus         spi_bus;
    cyg_interrupt       spi_interrupt;
    cyg_handle_t        spi_interrupt_handle;
    cyg_drv_mutex_t     transfer_mx;
    cyg_drv_cond_t      transfer_cond;
    cyg_bool            transfer_end;
    cyg_bool            cs_up;
    cyg_vector_t        interrupt_number;
    cyg_addrword_t      base;
    cyg_uint8           *lp_tx_buf;
    cyg_uint8           *lp_rx_buf;
    cyg_uint32          us_tx_bytes;
    cyg_uint32          us_rx_bytes;
    volatile cyg_uint32 transfer_cycles;
    volatile cyg_uint32 cycles_sended;
} cyg_spi_xc7z_bus_t;

typedef struct cyg_spi_xc7z_device_s
{
    // ---- Upper layer data ----

    cyg_spi_device spi_device;  // Upper layer SPI device data

    // ---- Lower layer data (configurable) ----

    cyg_bool   master_mode;     // 1-master, 0-slave
    cyg_uint8  src_divisor;     // SRCSEL divisor
    cyg_uint8  cs_divisor;      // CS divisor
    cyg_uint8  cs_num;          // Device number
    cyg_uint8  cl_pol;          // Clock polarity (0 or 1)
    cyg_uint8  cl_pha;          // Clock phase    (0 or 1)
    cyg_uint32 cl_brate;        // Clock baud rate
    cyg_uint16 cs_up_udly;      // Delay in us between CS up and transfer start
    cyg_uint16 cs_dw_udly;      // Delay in us between transfer end and CS down
    cyg_uint16 tr_bt_udly;      // Delay in us between two transfers
    cyg_bool   init;            // Is device initialized

} cyg_spi_xc7z_device_t;

#ifdef CYGHWR_DEVS_SPI_ARM_XC7Z_BUS0
externC cyg_spi_xc7z_bus_t cyg_spi_xc7z_bus0;
#endif

#ifdef CYGHWR_DEVS_SPI_ARM_XC7Z_BUS1
externC cyg_spi_xc7z_bus_t cyg_spi_xc7z_bus1;
#endif

#endif
