//==========================================================================
//
//      i2c_freescale.h
//
//      I2C driver for Freescale Kinetis and ColdFire+ microcontrollers
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2010, 2011 Free Software Foundation, Inc.
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
// Author(s):     Tomas Frydrych <tomas@sleepfive.com>
// Contributors:
// Date:          2011-11-20
// Original:      ccoutand
//                I2C driver for Stellaris Cortex M3 microcontroller
// Description:   I2C driver for Freescale Kinetis and ColdFire+
//                microcontrollers
//####DESCRIPTIONEND####
//==========================================================================

#ifndef CYGONCE_I2C_FREESCALE_H
#define CYGONCE_I2C_FREESCALE_H

#include <pkgconf/devs_i2c_freescale_i2c.h>
#include <cyg/io/i2c.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>

//---------------------------------------------------------------------------
// i2c memory structure
typedef volatile struct freescale_i2c_s {
    cyg_uint8 a1;
    cyg_uint8 f;
    cyg_uint8 c1;
    cyg_uint8 s;
    cyg_uint8 d;
    cyg_uint8 c2;
    cyg_uint8 flt;
    cyg_uint8 ra;
    cyg_uint8 smb;
    cyg_uint8 a2;
    cyg_uint8 slth;
    cyg_uint8 sltl;
} freescale_i2c_t;

// I2C register base pointers
#define FREESCALE_I2C_P(__base)  ((freescale_i2c_t*)(__base))

// Modes
// Master write
#define FREESCALE_I2C_MODE_MW 0x0u
// Master read
#define FREESCALE_I2C_MODE_MR 0x1u

// Bit Fields
#define FREESCALE_I2C_A1_AD_M  0xFEu
#define FREESCALE_I2C_A1_AD_S  1
// adress register from address and mode
#define FREESCALE_I2C_A1_AD(__a, __m)                                   \
  ((((cyg_uint8)(((cyg_uint8)(__a))<<FREESCALE_I2C_A1_AD_S)) &          \
    FREESCALE_I2C_A1_AD_M)|(__m & 0x1u))

#define FREESCALE_I2C_F_ICR_M  0x3Fu
#define FREESCALE_I2C_F_ICR_S  0
#define FREESCALE_I2C_F_ICR(x)                                          \
  (((cyg_uint8)(((cyg_uint8)(x))<<FREESCALE_I2C_F_ICR_S)) &             \
   FREESCALE_I2C_F_ICR_M)
#define FREESCALE_I2C_F_MULT_M 0xC0u
#define FREESCALE_I2C_F_MULT_S 6
#define FREESCALE_I2C_F_MULT(x)                                         \
  (((cyg_uint8)(((cyg_uint8)(x))<<FREESCALE_I2C_F_MULT_S)) &            \
   FREESCALE_I2C_F_MULT_M)
#define FREESCALE_I2C_F(__mult, __icr) \
  (FREESCALE_I2C_F_MULT (__mult) |     \
   FREESCALE_I2C_F_ICR (__icr))

#define FREESCALE_I2C_C1_DMAEN_M 0x1u
#define FREESCALE_I2C_C1_WUEN_M  0x2u
#define FREESCALE_I2C_C1_RSTA_M  0x4u
#define FREESCALE_I2C_C1_TXAK_M  0x8u
#define FREESCALE_I2C_C1_TX_M    0x10u
#define FREESCALE_I2C_C1_MST_M   0x20u
#define FREESCALE_I2C_C1_IICIE_M 0x40u
#define FREESCALE_I2C_C1_IICEN_M 0x80u

#define FREESCALE_I2C_S_RXAK_M   0x1u
#define FREESCALE_I2C_S_IICIF_M  0x2u
#define FREESCALE_I2C_S_SRW_M    0x4u
#define FREESCALE_I2C_S_RAM_M    0x8u
#define FREESCALE_I2C_S_ARBL_M   0x10u
#define FREESCALE_I2C_S_BUSY_M   0x20u
#define FREESCALE_I2C_S_IAAS_M   0x40u
#define FREESCALE_I2C_S_TCF_M    0x80u

#define FREESCALE_I2C_C2_AD_M    0x7u
#define FREESCALE_I2C_C2_AD_S    0
#define FREESCALE_I2C_C2_AD(x)                                          \
  (((cyg_uint8)(((cyg_uint8)(x))<<FREESCALE_I2C_C2_AD_S)) &             \
   FREESCALE_I2C_C2_AD_M)
#define FREESCALE_I2C_C2_RMEN_M  0x8u
#define FREESCALE_I2C_C2_SBRC_M  0x10u
#define FREESCALE_I2C_C2_HDRS_M  0x20u
#define FREESCALE_I2C_C2_ADEXT_M 0x40u
#define FREESCALE_I2C_C2_GCAEN_M 0x80u

#define FREESCALE_I2C_FLT_FLT_M  0x1Fu
#define FREESCALE_I2C_FLT_FLT_S  0
#define FREESCALE_I2C_FLT_FLT(x)                                        \
  (((cyg_uint8)(((cyg_uint8)(x))<<FREESCALE_I2C_FLT_FLT_S)) &           \
   FREESCALE_I2C_FLT_FLT_M)

#define FREESCALE_I2C_RA_RAD_M   0xFEu
#define FREESCALE_I2C_RA_RAD_S   1
#define FREESCALE_I2C_RA_RAD(x)                                         \
  (((cyg_uint8)(((cyg_uint8)(x))<<FREESCALE_I2C_RA_RAD_S)) &            \
   FREESCALE_I2C_RA_RAD_M)

#define FREESCALE_I2C_SMB_SHTF2IE_M 0x1u
#define FREESCALE_I2C_SMB_SHTF2_M   0x2u
#define FREESCALE_I2C_SMB_SHTF1_M   0x4u
#define FREESCALE_I2C_SMB_SLTF_M    0x8u
#define FREESCALE_I2C_SMB_TCKSEL_M  0x10u
#define FREESCALE_I2C_SMB_SIICAEN_M 0x20u
#define FREESCALE_I2C_SMB_ALERTEN_M 0x40u
#define FREESCALE_I2C_SMB_FACK_M    0x80u

#define FREESCALE_I2C_A2_SAD_M      0xFEu
#define FREESCALE_I2C_A2_SAD_S      1
#define FREESCALE_I2C_A2_SAD(x)                                         \
  (((cyg_uint8)(((cyg_uint8)(x))<<FREESCALE_I2C_A2_SAD_S)) &            \
   FREESCALE_I2C_A2_SAD_M)

#define FREESCALE_I2C_SLTH_SSLT_M   0xFFu
#define FREESCALE_I2C_SLTH_SSLT_S   0
#define FREESCALE_I2C_SLTH_SSLT(x)                                      \
  (((cyg_uint8)(((cyg_uint8)(x))<<FREESCALE_I2C_SLTH_SSLT_S)) &         \
   FREESCALE_I2C_SLTH_SSLT_M)

#define FREESCALE_I2C_SLTL_SSLT_M   0xFFu
#define FREESCALE_I2C_SLTL_SSLT_S   0
#define FREESCALE_I2C_SLTL_SSLT(x)                                      \
  (((cyg_uint8)(((cyg_uint8)(x))<<FREESCALE_I2C_SLTL_SSLT_S)) &         \
   FREESCALE_I2C_SLTL_SSLT_M)

#define FREESCALE_I2C_DELAY_F_CACHED_S    31
#define FREESCALE_I2C_DELAY_FIT_S         29
#define FREESCALE_I2C_DELAY_AGR_S         28

#define FREESCALE_I2C_DELAY_F_IS_CACHED_M BIT_(FREESCALE_I2C_DELAY_F_CACHED_S)
#define FREESCALE_I2C_DELAY_F_CACHE_M     0xFF
#define FREESCALE_I2C_DELAY_M             0x0FFFFFFF

#define FREESCALE_I2C_DELAY_CFG(__delay, __fit, __agr)                    \
            ((__delay) + (((__fit) & 0x3) << FREESCALE_I2C_DELAY_FIT_S) + \
            (((__agr) & 0x1) << FREESCALE_I2C_DELAY_AGR_S))

#define FREEDCALE_I2C_DELAY_IS_AGRESIVE(__delay) \
            ((__delay) & (1 << FREESCALE_I2C_DELAY_AGR_S))

typedef enum freescale_i2c_xfer_mode {
    FREESCALE_I2C_XFER_MODE_INVALID = 0x00,
    FREESCALE_I2C_XFER_MODE_TX = 0x01,
    FREESCALE_I2C_XFER_MODE_RX = 0x02
} freescale_i2c_xfer_mode;

// Resources borrowed from HAL
typedef struct freescale_i2c_extra_hal_s {
    cyg_uint32 pin_sda;                // Data pin
    cyg_uint32 pin_scl;                // Clock pin
    cyg_uint32 clock;                  // Clock gate
} freescale_i2c_extra_hal_t;

/*
 * Extra data for the i2c bus
 */
typedef struct freescale_i2c_extra {
    cyg_uint32      i2c_base;          // I2C base address
    cyg_uint32      i2c_delay;
    cyg_uint8       i2c_bus_number;    // bus number
    cyg_uint8       i2c_owner;         // We have bus ownership
    cyg_uint8       i2c_lost_arb;      // Error condition leading to loss of
                                       // bus ownership
    cyg_uint8       i2c_send_nack;     // As per rx send_nack argument
    cyg_uint8       i2c_got_nack;      // The last tx resulted in a nack
    cyg_uint8       i2c_completed;     // Set by DSR, checked by thread

    cyg_uint8       c2_cfg;             // C2 register default settings
    cyg_uint8       flt_cfg;            // Glitch filter setting

    union {
        const cyg_uint8 *i2c_tx_data;
        cyg_uint8       *i2c_rx_data;
    } i2c_data;                        // The current buffer for rx or tx
    cyg_uint32      i2c_count;         // Number of bytes left in buffer

    freescale_i2c_xfer_mode i2c_mode;       // TX, RX, ...

    cyg_bool        send_stop;
    cyg_bool        send_start;
    cyg_drv_mutex_t i2c_lock;          // For synchronizing between DSR and
                                       // foreground
    cyg_drv_cond_t  i2c_wait;
    // For initializing the interrupt
    cyg_handle_t    i2c_interrupt_handle;
    cyg_interrupt   i2c_interrupt_data;
    cyg_uint32      i2c_isr_id;
    cyg_uint32      i2c_isr_pri;
    const freescale_i2c_extra_hal_t *hal_p;         // Resources borrowed from HAL
} freescale_i2c_extra;

externC void freescale_i2c_init(struct cyg_i2c_bus *);
externC cyg_uint32 freescale_i2c_tx(cyg_i2c_device *, cyg_bool,
                                    const cyg_uint8 *, cyg_uint32, cyg_bool);
externC cyg_uint32 freescale_i2c_rx(cyg_i2c_device *,
                                    cyg_bool, cyg_uint8 *, cyg_uint32,
                                    cyg_bool, cyg_bool);
externC void freescale_i2c_stop(const cyg_i2c_device *);
externC void freescale_i2c_set_frequency (cyg_i2c_device * dev,
                                          cyg_uint8 premul);

#endif // CYGONCE_I2C_FREESCALE_H
// -------------------------------------------------------------------------
// EOF i2c_freescale.h
