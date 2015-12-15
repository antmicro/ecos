#ifndef CYGONCE_I2C_STM32_H
#define CYGONCE_I2C_STM32_H
//==========================================================================
//
//      devs/i2c/cortexm/stm32/current/src/i2c_stm32.h
//
//      I2C driver for STM32 CortexM processors
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
// Author(s):    Antmicro LTd <contact@antmicro.com>
// Contributors:
// Date:         2015-12-15
// Description:  I2C bus driver for STM32F7
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/devs_i2c_cortexm_stm32f7.h>
#include <cyg/io/i2c.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>

//--------------------------------------------------------------------------
// Single I2C bus sepecififc data
typedef struct cyg_stm32_i2c_extra {
    cyg_uint32       i2c_base;               // Bus register base address
    cyg_uint8        i2c_addr;               // Slave address
    cyg_uint32       i2c_txtotal;            // Bytes in acutal transfer
    cyg_uint32       i2c_rxtotal;            // Bytes actually to receive
    cyg_uint32       i2c_txleft;             // Bytes left to send
    cyg_uint32       i2c_rxleft;             // Bytes left to receive
    const cyg_uint8* i2c_txbuf;              // Reception buffer
    cyg_uint8*       i2c_rxbuf;              // Transmission buffer
    cyg_bool         i2c_rxnak;              // Flag for NACK generation
    cyg_bool         i2c_stop;               // Flag for stop generation
    cyg_drv_mutex_t  i2c_lock;                // For synchronizing between DSR and foreground
    cyg_drv_cond_t   i2c_wait;                // For synchronizing between DSR and foreground
    cyg_uint32       i2c_periph_clock;        // Clock definition for bus, used to enable/disable peripheral clock
    cyg_uint32       i2c_periph_reset;        // Reset bit number for peripheral
    cyg_uint32       i2c_bus_mode;            // Bus mode: poll or interrupt
    cyg_vector_t     i2c_ev_vec;              // Event vector
    cyg_priority_t   i2c_ev_priority;         // Event interrupt priority
    cyg_handle_t     i2c_ev_interrupt_handle; // For initializing the interrupt
    cyg_interrupt    i2c_ev_interrupt_data;
    cyg_vector_t     i2c_err_vec;             // Error vector
    cyg_priority_t   i2c_err_priority;        // Error interrupt priority
    cyg_handle_t     i2c_err_interrupt_handle;
    cyg_interrupt    i2c_err_interrupt_data;
} cyg_stm32_i2c_extra;

//--------------------------------------------------------------------------
// Exported I2C driver functions
externC void        cyg_stm32_i2c_init(struct cyg_i2c_bus*);
externC void        cyg_stm32_i2c_stop(const cyg_i2c_device*);

//--------------------------------------------------------------------------
// I2C bus declaration macros
#define CYG_STM32_I2C_BUS(                                 \
    _name_,                                                \
    _base_,                                                \
    _ev_int_vec_,                                          \
    _ev_priority_,                                         \
    _err_int_vec_,                                         \
    _err_priority_,                                        \
    _periph_clock_,                                        \
    _periph_reset_,                                        \
    _bus_mode_)                                            \
    static cyg_stm32_i2c_extra _name_ ## _extra = {        \
        i2c_base        : _base_,                          \
        i2c_ev_vec      : _ev_int_vec_,                    \
        i2c_ev_priority : _ev_priority_,                   \
        i2c_err_vec     : _err_int_vec_,                   \
        i2c_err_priority: _err_priority_,                  \
        i2c_txleft      :  0,                              \
        i2c_rxleft      :  0,                              \
        i2c_txbuf       :  NULL,                           \
        i2c_rxbuf       :  NULL,                           \
        i2c_periph_clock: _periph_clock_,                  \
        i2c_periph_reset: _periph_reset_,                  \
        i2c_bus_mode    : _bus_mode_,                      \
    };                                                     \
  CYG_I2C_BUS(_name_,                                      \
              &cyg_stm32_i2c_init,                         \
              NULL,                                        \
              NULL,                                        \
              &cyg_stm32_i2c_stop,                         \
              (void*) & ( _name_ ## _extra));

//--------------------------------------------------------------------------
// Exported I2C bus instances
#ifdef CYGHWR_DEVS_I2C_CORTEXM_STM32_BUS1
externC struct cyg_i2c_bus i2c_bus1;
#endif

#ifdef CYGHWR_DEVS_I2C_CORTEXM_STM32_BUS2
externC struct cyg_i2c_bus i2c_bus2;
#endif

#ifdef CYGHWR_DEVS_I2C_CORTEXM_STM32_BUS3
externC struct cyg_i2c_bus i2c_bus3;
#endif

#ifdef CYGHWR_DEVS_I2C_CORTEXM_STM32_BUS4
externC struct cyg_i2c_bus i2c_bus4;
#endif

//-----------------------------------------------------------------------------
#endif // #endif CYGONCE_I2C_STM32_H
