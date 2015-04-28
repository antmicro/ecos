#ifndef I2C_XC7Z_H
#define I2C_XC7Z_H
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

/************************** INCLUDES **************************************/
#include <pkgconf/devs_i2c_arm_xc7z.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>

/************************** DRIVER SPECIFIC DATA **************************/
//TODO: revision of the extra struct fields, add mutex?
typedef struct cyg_zynq_i2c_extra {
        cyg_uint8* i2c_tx_buf;
        cyg_uint8* i2c_rx_buf;
        cyg_uint8  i2c_addr;
        cyg_uint32 i2c_bytes_left;
        cyg_uint32 i2c_hold_flag;
        cyg_uint32 i2c_bus_freq;
        cyg_uint32      i2c_flag;
        cyg_drv_mutex_t i2c_lock;
        cyg_drv_cond_t  i2c_wait;
        cyg_handle_t    i2c_int_handle;
        cyg_interrupt   i2c_int_data;
        cyg_vector_t    i2c_isr_vector;
        int             i2c_isr_priority;
} cyg_zynq_i2c_extra;

/************************** DRIVER INTERFACE ******************************/
externC void       zynq_i2c_init(struct cyg_i2c_bus*);
externC cyg_uint32 zynq_i2c_tx(const cyg_i2c_device*, cyg_bool,
                               const cyg_uint8*, cyg_uint32, cyg_bool);
externC cyg_uint32 zynq_i2c_rx(const cyg_i2c_device*, cyg_bool,
                               cyg_uint8*, cyg_uint32, cyg_bool, cyg_bool);
externC void       zynq_i2c_stop(const cyg_i2c_device*);

/************************** I2C BUS MACRO *********************************/
//TODO: why user pass the _init_fn_ value?
#define CYG_ZYNQ_I2C_BUS(_name_, _i2c_freq_) \
        static cyg_zynq_i2c_extra _name_ ## _extra = {  \
        i2c_tx_buf : NULL,                      \
        i2c_rx_buf : NULL,                      \
        i2c_bus_freq : _i2c_freq_               \
        };                                      \
CYG_I2C_BUS(_name_,                             \
        &zynq_i2c_init,                         \
        &zynq_i2c_tx,                           \
        &zynq_i2c_rx,                           \
        &zynq_i2c_stop,                         \
        (void*) & (_name_ ## _extra) );
#endif
