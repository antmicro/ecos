/**************************************************************************/
/**
*
* @file     spi_xc7z.h
*
* @brief    Xilinx XC7Z (ARM) QSPI driver defines
*
***************************************************************************/
#ifndef CYGONCE_DEVS_QSPI_ARM_XC7Z_H
#define CYGONCE_DEVS_QSPI_ARM_XC7Z_H
//==========================================================================
//
//      qspi_xc7z.h
//
//      Xilinx XC7Z (ARM) QSPI driver defines
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2012 Free Software Foundation, Inc.
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
// Date:          2012-07-09
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/io_spi.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/drv_api.h>
#include <cyg/io/spi.h>

/****************************************************************************/
/**
*
* QSPI bus structure definition
*
*****************************************************************************/
typedef struct cyg_qspi_xc7z_bus_s
{
    // ---- Upper layer data ----

    cyg_spi_bus   qspi_bus;                  // Upper layer QSPI bus data

    // ---- Lower layer data ----
     
    cyg_interrupt     qspi_interrupt;        // QSPI interrupt object
    cyg_handle_t      qspi_interrupt_handle; // QSPI interrupt handle
    cyg_drv_mutex_t   transfer_mx;           // Transfer mutex
    cyg_drv_cond_t    transfer_cond;         // Transfer condition
    cyg_bool          transfer_end;          // Transfer end flag
    cyg_bool          cs_up;                 // Chip Select up flag 
    cyg_vector_t      interrupt_number;      // QSPI Interrupt Number
    cyg_addrword_t    base;                  // Base Address of the QSPI peripheral
    cyg_uint8         cs_en[1];              // The Configurations state for the CS
    cyg_uint32        cs_gpio[1];            // The GPIO Configurations for the CS

    cyg_haladdress    dma_reg_base;          // Base address of DMA register block.
    cyg_uint8         dma_tx_channel;        // TX DMA channel for this bus.
    cyg_uint8         dma_rx_channel;        // RX DMA channel for this bus.
    cyg_vector_t      dma_tx_intr;           // Interrupt used for DMA transmit.
    cyg_vector_t      dma_rx_intr;           // Interrupt used for DMA receive.
    cyg_bool          tx_dma_done;           // Flags used to signal completion.
    cyg_bool          rx_dma_done;           // Flags used to signal completion.
    
    cyg_uint8        *lp_tx_buf;
    cyg_uint8        *lp_rx_buf;
    cyg_uint32        us_tx_bytes;
    cyg_uint32        us_rx_bytes;
    
    cyg_uint8         uc_tx_instr;
    
} cyg_qspi_xc7z_bus_t;

/****************************************************************************/
/**
*
* QSPI device structure definition
*
*****************************************************************************/
typedef struct cyg_qspi_xc7z_device_s
{
    // ---- Upper layer data ----

    cyg_spi_device qspi_device;  // Upper layer QSPI device data

    // ---- Lower layer data (configurable) ----

    cyg_uint8  dev_num;         // Device number
    cyg_uint8  cl_pol;          // Clock polarity (0 or 1)
    cyg_uint8  cl_pha;          // Clock phase    (0 or 1)
    cyg_uint32 cl_brate;        // Clock baud rate
    cyg_uint16 cs_up_udly;      // Delay in us between CS up and transfer start
    cyg_uint16 cs_dw_udly;      // Delay in us between transfer end and CS down
    cyg_uint16 tr_bt_udly;      // Delay in us between two transfers

    // ---- Lower layer data (internal) ----

    cyg_bool   init;            // Is device initialized
    cyg_uint8  cl_scbr;         // Value of SCBR (QSPI clock) reg field
    cyg_uint8  cl_div32;        // Divide QSPI master clock by 32

} cyg_qspi_xc7z_device_t;

/****************************************************************************/
/**
*
* XC7Z QSPI exported busses: For backwards compatability
*
*****************************************************************************/
#define cyg_qspi_xc7z_bus cyg_qspi_xc7z_bus0

/****************************************************************************/
/**
*
* XC7Z QSPI bus exported structure
*
*****************************************************************************/
externC cyg_qspi_xc7z_bus_t cyg_qspi_xc7z_bus;

//-----------------------------------------------------------------------------

#endif // CYGONCE_DEVS_QSPI_ARM_XC7Z_H 

//-----------------------------------------------------------------------------
// End of qspi_xc7z.h
