#ifndef CYGONCE_I2C_FREESCALE_BUSES_INL
#define CYGONCE_I2C_FREESCALE_BUSES_INL
//==========================================================================
//
//      i2c_freescale_buses.inl
//
//      Freescale I2C bus instances
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2013 Free Software Foundation, Inc.
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
// Author(s):     ilijak
// Contributors:
// Date:          2011-11-20
// Description:   Freescale I2C bus instances
//
//####DESCRIPTIONEND####
//==========================================================================

// I2C bus definition macros
// HAL should provide following macros per _bus_:
//  -I2C controller base address
//    CYGADDR_IO_I2C_FREESCALE_I2C<_bus_>_BASE
//  -Interrupt vector number and priority
//    CYGNUM_DEVS_INTERRUPT_I2C<_bus_>
//    CYGNUM_DEVS_FREESCALE_I2C<_bus_>_IRQ_PRIORITY
//  -I2C pin configuration:
//    CYGHWR_IO_I2C_FREESCALE_I2C<_bus_>_PIN_SDA
//    CYGHWR_IO_I2C_FREESCALE_I2C<_bus_>_PIN_SCL
//  -Clock gating
//    CYGHWR_IO_FREESCALE_I2C<_bus_>_CLK

#define CYG_FREESCALE_I2C_BUS(_bus_)                                              \
static const freescale_i2c_extra_hal_t cyg_i2c ## _bus_ ## _extra_hal = {         \
    .pin_sda = CYGHWR_IO_I2C_FREESCALE_I2C ## _bus_ ## _PIN_SDA,                  \
    .pin_scl = CYGHWR_IO_I2C_FREESCALE_I2C ## _bus_ ## _PIN_SCL,                  \
    .clock   = CYGHWR_IO_FREESCALE_I2C ## _bus_ ## _CLK                           \
};                                                                                \
                                                                                  \
static freescale_i2c_extra cyg_i2c ## _bus_ ## _bus_extra = {                     \
    .i2c_base       = CYGADDR_IO_I2C_FREESCALE_I2C ## _bus_ ## _BASE,             \
    .i2c_bus_number = _bus_,                                                      \
    .i2c_delay      = CYGNUM_DEVS_FREESCALE_I2C ## _bus_ ## _CLOCK & 0x80000000 ? \
                      CYGNUM_DEVS_FREESCALE_I2C ## _bus_ ## _CLOCK :              \
                      FREESCALE_I2C_DELAY_CFG(1000000000 /                        \
                          CYGNUM_DEVS_FREESCALE_I2C ## _bus_ ## _CLOCK,           \
                          CYGNUM_DEVS_FREESCALE_I2C ## _bus_ ## _CLOCK_FIT - 1,   \
                          CYGNUM_DEVS_FREESCALE_I2C ## _bus_ ## _CLOCK_AGR),      \
    .i2c_isr_id     = CYGNUM_HAL_INTERRUPT_I2C ## _bus_,                          \
    .i2c_isr_pri    = CYGNUM_DEVS_FREESCALE_I2C ## _bus_ ## _IRQ_PRIORITY,        \
    .c2_cfg         = CYGOPT_DEVS_FREESCALE_I2C ## _bus_ ## _C2,                  \
    .flt_cfg        = CYGNUM_DEVS_FREESCALE_I2C ## _bus_ ## _FLT,                 \
    .hal_p          = &(cyg_i2c ## _bus_ ## _extra_hal)                           \
};                                                                                \
                                                                                  \
CYG_I2C_BUS(cyg_i2c ## _bus_ ## _bus,                                             \
            freescale_i2c_init,                                                   \
            freescale_i2c_tx,                                                     \
            freescale_i2c_rx,                                                     \
            freescale_i2c_stop,                                                   \
            (void*) & (cyg_i2c ## _bus_ ## _bus_extra)) ;

#endif // CYGONCE_I2C_FREESCALE_BUSES_INL

//-----------------------------------------------------------------------------
// I2C bus instances
//
#ifdef CYGHWR_DEVS_FREESCALE_I2C0
CYG_FREESCALE_I2C_BUS(0);
#endif

#ifdef CYGHWR_DEVS_FREESCALE_I2C1
CYG_FREESCALE_I2C_BUS(1);
#endif
// -------------------------------------------------------------------------
// EOF i2c_freescale_buses.inl
