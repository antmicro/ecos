#ifndef SPI_FREESCALE_DSPI_BUSES_INL
#define SPI_FREESCALE_DSPI_BUSES_INL
//=============================================================================
//
//      spi_freescale_dspi_buses.inl
//
//      SPI bus instantiation for Freescale DSPI
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2011, 2013 Free Software Foundation, Inc.
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   ilijak
// Date:        2011-11-09
// Purpose:     Freescale DSPI DSPI bus instantiation
//
//####DESCRIPTIONEND####
//
//=============================================================================

#define PUSHQUE_ALIGN CYGBLD_ATTRIB_ALIGN(4)

#define DSPI_DMA_BANDWIDTH FREESCALE_EDMA_CSR_BWC_0

//-----------------------------------------------------------------------------
// Instantiate the bus state data structures.

// Some auxiliary macros

#if (CYG_BYTEORDER == CYG_MSBFIRST)  // AKA Big endian
#define EDMA_TCD_SADDR(__bus) \
        .saddr = (void *)(((unsigned int)&CYGADDR_IO_SPI_FREESCALE_DSPI ## __bus ## _P->popr) + 3)
#else // AKA Little endian
#define EDMA_TCD_SADDR(__bus) \
        .saddr = (cyg_uint32 *)&CYGADDR_IO_SPI_FREESCALE_DSPI ## __bus ## _P->popr
#endif

#define DSPI_EDMA_CHAN_SET(__bus) \
static volatile cyg_uint32 \
dspi ## __bus ## _pushque[CYGNUM_DEVS_SPI_FREESCALE_DSPI ## __bus ## _PUSHQUE_SIZE+4] \
PUSHQUE_ALIGN EDMA_RAM_BUF_SECTION;                                                   \
                                                                                      \
static const cyghwr_hal_freescale_dma_chan_set_t dspi ## __bus ## _dma_chan[2] = \
{                                                                                \
    {                                                                            \
        .dma_src = FREESCALE_DMAMUX_SRC_SPI ## __bus ## _TX                      \
        | FREESCALE_DMAMUX_CHCFG_ENBL_M,                                         \
        .dma_chan_i = CYGHWR_DEVS_SPI_FREESCALE_DSPI ## __bus ## _TX_DMA_CHAN,   \
        .dma_prio = CYGNUM_DEVS_SPI_FREESCALE_DSPI ## __bus ## _TX_DMA_PRI,      \
    },                                                                           \
    {                                                                            \
        .dma_src = FREESCALE_DMAMUX_SRC_SPI ## __bus ## _RX                      \
        | FREESCALE_DMAMUX_CHCFG_ENBL_M,                                         \
        .dma_chan_i = CYGHWR_DEVS_SPI_FREESCALE_DSPI ## __bus ## _RX_DMA_CHAN,   \
        .dma_prio = CYGNUM_DEVS_SPI_FREESCALE_DSPI ## __bus ## _RX_DMA_PRI,      \
    }                                                                            \
};                                                                               \
                                                                                 \
static cyghwr_hal_freescale_dma_set_t dspi ## __bus ## _dma_set = {              \
    .chan_p = dspi ## __bus ## _dma_chan,                                        \
    .chan_n = 2                                                                  \
};                                                                               \
                                                                                 \
static const cyghwr_hal_freescale_edma_tcd_t dspi ## __bus ## _dma_tcd_tx_ini =  \
{                                                                                \
        .saddr =  (cyg_uint32 *) dspi ## __bus ## _pushque,                      \
        .soff = 4,                                                               \
        .attr = FREESCALE_EDMA_ATTR_SSIZE(FREESCALE_EDMA_ATTR_SIZE_32) |         \
                FREESCALE_EDMA_ATTR_DSIZE(FREESCALE_EDMA_ATTR_SIZE_32) |         \
                FREESCALE_EDMA_ATTR_SMOD(0) |                                    \
                FREESCALE_EDMA_ATTR_DMOD(0),                                     \
        .daddr = (cyg_uint32 *)                                                  \
                 &CYGADDR_IO_SPI_FREESCALE_DSPI ## __bus ## _P->pushr,           \
        .doff = 0,                                                               \
        .nbytes.mlno = 4,                                                        \
        .slast = 0,                                                              \
        .citer.elinkno = 1,                                                      \
        .dlast_sga.dlast = 0,                                                    \
        .biter.elinkno = 1,                                                      \
        .csr = DSPI_DMA_BANDWIDTH                                                \
};                                                                               \
                                                                                 \
static const cyghwr_hal_freescale_edma_tcd_t dspi ## __bus ## _dma_tcd_rx_ini =  \
{                                                                                \
       EDMA_TCD_SADDR(__bus),                                                    \
        .soff = 0,                                                               \
        .attr = FREESCALE_EDMA_ATTR_SSIZE(FREESCALE_EDMA_ATTR_SIZE_32) |         \
                FREESCALE_EDMA_ATTR_DSIZE(FREESCALE_EDMA_ATTR_SIZE_32) |         \
                FREESCALE_EDMA_ATTR_SMOD(0) |                                    \
                FREESCALE_EDMA_ATTR_DMOD(0),                                     \
        .daddr = NULL,                                                           \
        .doff = 4,                                                               \
        .nbytes.mlno = 4,                                                        \
        .slast = 0,                                                              \
        .citer.elinkno = 1,                                                      \
        .dlast_sga.dlast = 0,                                                    \
        .biter.elinkno = 1,                                                      \
        .csr = DSPI_DMA_BANDWIDTH                                                \
}

#define DSPI_EDMA_SET_P_YES(__bus)  .dma_set_p = &dspi ## __bus ## _dma_set
#define DSPI_EDMA_SET_P_NULL(__bus)  .dma_set_p = NULL

#define DSPI_EDMA_TCD_YES(__bus)                                      \
    .tx_dma_tcd_p = &CYGHWR_IO_FREESCALE_EDMA0_P->                    \
        tcd[CYGHWR_DEVS_SPI_FREESCALE_DSPI ## __bus ## _TX_DMA_CHAN], \
    .rx_dma_tcd_p = &CYGHWR_IO_FREESCALE_EDMA0_P->                    \
    tcd[CYGHWR_DEVS_SPI_FREESCALE_DSPI ## __bus ## _RX_DMA_CHAN],     \
    .tx_dma_tcd_ini_p = &dspi ## __bus ## _dma_tcd_tx_ini,            \
    .rx_dma_tcd_ini_p = &dspi ## __bus ## _dma_tcd_rx_ini,            \
    .pushque_p = dspi ## __bus ## _pushque

#define DSPI_EDMA_TCD_NULL(__bus) \
    .tx_dma_tcd_p = NULL,         \
    .rx_dma_tcd_p = NULL,         \
    .tx_dma_tcd_ini_p = NULL,     \
    .rx_dma_tcd_ini_p = NULL,     \
    .pushque_p = NULL

// End DSPI_EDMA

#define DSPI_BUS_SETUP(__bus) \
static const cyg_spi_freescale_dspi_bus_setup_t dspi ## __bus ## _setup = {   \
    .dspi_p          = CYGADDR_IO_SPI_FREESCALE_DSPI ## __bus ## _P,          \
    .intr_num        = CYGNUM_HAL_INTERRUPT_SPI ## __bus,                     \
    .intr_prio       = CYGNUM_DEVS_SPI_FREESCALE_DSPI ## __bus ## _ISR_PRI,   \
    .spi_pin_list_p  = spi ## __bus ## _pins,                                 \
    .cs_pin_list_p   = spi ## __bus ## _cs_pins,                              \
    .clk_gate        = CYGHWR_IO_FREESCALE_DSPI ## __bus ## _CLK,             \
    .cs_pin_num      = sizeof(spi ## __bus ## _cs_pins)/                      \
                       sizeof(spi ## __bus ## _cs_pins[0]),                   \
    .mcr_opt         = CYGHWR_FREESCALE_DSPI ## __bus ## _MCR_PCSSE |         \
                       FREESCALE_DSPI_MCR_PCSIS(                              \
                          CYGHWR_DEVS_SPI_FREESCALE_DSPI ## __bus ## _PCSIS), \
    DSPI ## __bus ## _EDMA_SET_P                                              \
}

#define DSPI_BUS(__bus) \
cyg_spi_freescale_dspi_bus_t cyg_spi_dspi_bus ## __bus = {                 \
    .spi_bus.spi_transaction_begin    = dspi_transaction_begin,            \
    .spi_bus.spi_transaction_transfer = dspi_transaction_transfer,         \
    .spi_bus.spi_transaction_tick     = dspi_transaction_tick,             \
    .spi_bus.spi_transaction_end      = dspi_transaction_end,              \
    .spi_bus.spi_get_config           = dspi_get_config,                   \
    .spi_bus.spi_set_config           = dspi_set_config,                   \
    .setup_p                          = &dspi ## __bus ## _setup,          \
    DSPI ## __bus ## _EDMA_TCD,                                            \
    .pushque_n = CYGNUM_DEVS_SPI_FREESCALE_DSPI ## __bus ## _PUSHQUE_SIZE, \
    .txfifo_n = CYGHWR_DEVS_SPI_FREESCALE_DSPI_FIFO_SIZE,                  \
    .rxfifo_n = CYGHWR_DEVS_SPI_FREESCALE_DSPI_FIFO_SIZE                   \
}

#define DSPI_BUS_PINS(__bus) \
static const cyg_uint32 spi ## __bus ## _pins[] = { \
    CYGHWR_IO_FREESCALE_SPI ## __bus ## _PIN_SIN,   \
    CYGHWR_IO_FREESCALE_SPI ## __bus ## _PIN_SOUT,  \
    CYGHWR_IO_FREESCALE_SPI ## __bus ## _PIN_SCK    \
}

// DSPI BUS Instances =======================================================

// DSPI BUS 0 ================================================================
#ifdef CYGHWR_DEVS_SPI_FREESCALE_DSPI0

#define  CYGBLD_DSPI0_TCD_SECTION \
    CYGBLD_ATTRIB_SECTION(CYGOPT_DEVS_SPI_FREESCALE_DSPI0_TCD_SECTION)

#ifdef CYHGWR_DEVS_SPI_FREESCALE_DSPI0_PCSS
#define CYGHWR_FREESCALE_DSPI0_MCR_PCSSE FREESCALE_DSPI_MCR_PCSSE_M
#else
#define CYGHWR_FREESCALE_DSPI0_MCR_PCSSE 0
#endif

// SPI chip select pins.
static const cyg_uint32 spi0_cs_pins[] = {
#ifdef CYGHWR_FREESCALE_DSPI0_CS0
    CYGHWR_IO_FREESCALE_SPI0_PIN_CS0,
#endif
#ifdef CYGHWR_FREESCALE_DSPI0_CS1
    CYGHWR_IO_FREESCALE_SPI0_PIN_CS1,
#endif
#ifdef CYGHWR_FREESCALE_DSPI0_CS2
    CYGHWR_IO_FREESCALE_SPI0_PIN_CS2,
#endif
#ifdef CYGHWR_FREESCALE_DSPI0_CS3
    CYGHWR_IO_FREESCALE_SPI0_PIN_CS3,
#endif
#ifdef CYGHWR_FREESCALE_DSPI0_CS4
    CYGHWR_IO_FREESCALE_SPI0_PIN_CS4,
#endif
#ifdef CYGHWR_FREESCALE_DSPI0_CS5
    CYGHWR_IO_FREESCALE_SPI0_PIN_CS5
#endif
};

#ifdef CYGINT_DEVS_SPI_DSPI0_USES_DMA
  DSPI_EDMA_CHAN_SET(0);
# define DSPI0_EDMA_SET_P DSPI_EDMA_SET_P_YES(0)
# define DSPI0_EDMA_TCD DSPI_EDMA_TCD_YES(0)
#else
# define DSPI0_EDMA_SET_P DSPI_EDMA_SET_P_NULL(0)
# define DSPI0_EDMA_TCD DSPI_EDMA_TCD_NULL(0)
#endif // CYGINT_DEVS_SPI_DSPI0_USES_DMA

DSPI_BUS_PINS(0);
DSPI_BUS_SETUP(0);
DSPI_BUS(0);

#endif // CYGHWR_DEVS_SPI_FREESCALE_DSPI0

// DSPI BUS 1 ================================================================
#ifdef CYGHWR_DEVS_SPI_FREESCALE_DSPI1

#define  CYGBLD_DSPI1_TCD_SECTION \
    CYGBLD_ATTRIB_SECTION(CYGOPT_DEVS_SPI_FREESCALE_DSPI1_TCD_SECTION)

#ifdef CYHGWR_DEVS_SPI_FREESCALE_DSPI1_PCSS
#define CYGHWR_FREESCALE_DSPI1_MCR_PCSSE FREESCALE_DSPI_MCR_PCSSE_M
#else
#define CYGHWR_FREESCALE_DSPI1_MCR_PCSSE 0
#endif

// SPI chip select pins.
static const cyg_uint32 spi1_cs_pins[] = {
#ifdef CYGHWR_FREESCALE_DSPI1_CS0
    CYGHWR_IO_FREESCALE_SPI1_PIN_CS0,
#endif
#ifdef CYGHWR_FREESCALE_DSPI1_CS1
    CYGHWR_IO_FREESCALE_SPI1_PIN_CS1,
#endif
#ifdef CYGHWR_FREESCALE_DSPI1_CS2
    CYGHWR_IO_FREESCALE_SPI1_PIN_CS2,
#endif
#ifdef CYGHWR_FREESCALE_DSPI1_CS3
    CYGHWR_IO_FREESCALE_SPI1_PIN_CS3,
#endif
#ifdef CYGHWR_FREESCALE_DSPI1_CS4
    CYGHWR_IO_FREESCALE_SPI1_PIN_CS4,
#endif
#ifdef CYGHWR_FREESCALE_DSPI1_CS5
    CYGHWR_IO_FREESCALE_SPI1_PIN_CS5
#endif
};

#ifdef CYGINT_DEVS_SPI_DSPI1_USES_DMA
  DSPI_EDMA_CHAN_SET(1);
# define DSPI1_EDMA_SET_P DSPI_EDMA_SET_P_YES(1)
# define DSPI1_EDMA_TCD DSPI_EDMA_TCD_YES(1)
#else
# define DSPI1_EDMA_SET_P DSPI_EDMA_SET_P_NULL(1)
# define DSPI1_EDMA_TCD DSPI_EDMA_TCD_NULL(1)
#endif // CYGINT_DEVS_SPI_DSPI1_USES_DMA

DSPI_BUS_PINS(1);
DSPI_BUS_SETUP(1);
DSPI_BUS(1);

#endif // CYGHWR_DEVS_SPI_FREESCALE_DSPI1

// DSPI BUS 2 ================================================================
#ifdef CYGHWR_DEVS_SPI_FREESCALE_DSPI2

#define  CYGBLD_DSPI2_TCD_SECTION \
    CYGBLD_ATTRIB_SECTION(CYGOPT_DEVS_SPI_FREESCALE_DSPI2_TCD_SECTION)

#ifdef CYHGWR_DEVS_SPI_FREESCALE_DSPI2_PCSS
#define CYGHWR_FREESCALE_DSPI2_MCR_PCSSE FREESCALE_DSPI_MCR_PCSSE_M
#else
#define CYGHWR_FREESCALE_DSPI2_MCR_PCSSE 0
#endif

// SPI chip select pins.
static const cyg_uint32 spi2_cs_pins[] = {
#ifdef CYGHWR_FREESCALE_DSPI2_CS0
    CYGHWR_IO_FREESCALE_SPI2_PIN_CS0,
#endif
#ifdef CYGHWR_FREESCALE_DSPI2_CS1
    CYGHWR_IO_FREESCALE_SPI2_PIN_CS1,
#endif
#ifdef CYGHWR_FREESCALE_DSPI2_CS2
    CYGHWR_IO_FREESCALE_SPI2_PIN_CS2,
#endif
#ifdef CYGHWR_FREESCALE_DSPI2_CS3
    CYGHWR_IO_FREESCALE_SPI2_PIN_CS3,
#endif
#ifdef CYGHWR_FREESCALE_DSPI2_CS4
    CYGHWR_IO_FREESCALE_SPI2_PIN_CS4,
#endif
#ifdef CYGHWR_FREESCALE_DSPI2_CS5
    CYGHWR_IO_FREESCALE_SPI2_PIN_CS5
#endif
};

#ifdef CYGINT_DEVS_SPI_DSPI2_USES_DMA
  DSPI_EDMA_CHAN_SET(2);
# define DSPI2_EDMA_SET_P DSPI_EDMA_SET_P_YES(2)
# define DSPI2_EDMA_TCD DSPI_EDMA_TCD_YES(2)
#else
# define DSPI2_EDMA_SET_P DSPI_EDMA_SET_P_NULL(2)
# define DSPI2_EDMA_TCD DSPI_EDMA_TCD_NULL(2)
#endif // CYGINT_DEVS_SPI_DSPI2_USES_DMA

DSPI_BUS_PINS(2);
DSPI_BUS_SETUP(2);
DSPI_BUS(2);

#endif // CYGHWR_DEVS_SPI_FREESCALE_DSPI2

// DSPI BUS 3 ================================================================
#ifdef CYGHWR_DEVS_SPI_FREESCALE_DSPI3

#define  CYGBLD_DSPI3_TCD_SECTION \
    CYGBLD_ATTRIB_SECTION(CYGOPT_DEVS_SPI_FREESCALE_DSPI3_TCD_SECTION)

#ifdef CYHGWR_DEVS_SPI_FREESCALE_DSPI3_PCSS
#define CYGHWR_FREESCALE_DSPI3_MCR_PCSSE FREESCALE_DSPI_MCR_PCSSE_M
#else
#define CYGHWR_FREESCALE_DSPI3_MCR_PCSSE 0
#endif

// SPI chip select pins.
static const cyg_uint32 spi3_cs_pins[] = {
#ifdef CYGHWR_FREESCALE_DSPI3_CS0
    CYGHWR_IO_FREESCALE_SPI3_PIN_CS0,
#endif
#ifdef CYGHWR_FREESCALE_DSPI3_CS1
    CYGHWR_IO_FREESCALE_SPI3_PIN_CS1,
#endif
#ifdef CYGHWR_FREESCALE_DSPI3_CS2
    CYGHWR_IO_FREESCALE_SPI3_PIN_CS2,
#endif
#ifdef CYGHWR_FREESCALE_DSPI3_CS3
    CYGHWR_IO_FREESCALE_SPI3_PIN_CS3,
#endif
#ifdef CYGHWR_FREESCALE_DSPI3_CS4
    CYGHWR_IO_FREESCALE_SPI3_PIN_CS4,
#endif
#ifdef CYGHWR_FREESCALE_DSPI3_CS5
    CYGHWR_IO_FREESCALE_SPI3_PIN_CS5
#endif
};

#ifdef CYGINT_DEVS_SPI_DSPI3_USES_DMA
  DSPI_EDMA_CHAN_SET(3);
# define DSPI3_EDMA_SET_P DSPI_EDMA_SET_P_YES(3)
# define DSPI3_EDMA_TCD DSPI_EDMA_TCD_YES(3)
#else
# define DSPI3_EDMA_SET_P DSPI_EDMA_SET_P_NULL(3)
# define DSPI3_EDMA_TCD DSPI_EDMA_TCD_NULL(3)
#endif // CYGINT_DEVS_SPI_DSPI3_USES_DMA

DSPI_BUS_PINS(3);
DSPI_BUS_SETUP(3);
DSPI_BUS(3);

#endif // CYGHWR_DEVS_SPI_FREESCALE_DSPI3

// DSPI BUS 4 ================================================================
#ifdef CYGHWR_DEVS_SPI_FREESCALE_DSPI4

#define  CYGBLD_DSPI4_TCD_SECTION \
    CYGBLD_ATTRIB_SECTION(CYGOPT_DEVS_SPI_FREESCALE_DSPI4_TCD_SECTION)

#ifdef CYHGWR_DEVS_SPI_FREESCALE_DSPI4_PCSS
#define CYGHWR_FREESCALE_DSPI4_MCR_PCSSE FREESCALE_DSPI_MCR_PCSSE_M
#else
#define CYGHWR_FREESCALE_DSPI4_MCR_PCSSE 0
#endif

// SPI chip select pins.
static const cyg_uint32 spi4_cs_pins[] = {
#ifdef CYGHWR_FREESCALE_DSPI4_CS0
    CYGHWR_IO_FREESCALE_SPI4_PIN_CS0,
#endif
#ifdef CYGHWR_FREESCALE_DSPI4_CS1
    CYGHWR_IO_FREESCALE_SPI4_PIN_CS1,
#endif
#ifdef CYGHWR_FREESCALE_DSPI4_CS2
    CYGHWR_IO_FREESCALE_SPI4_PIN_CS2,
#endif
#ifdef CYGHWR_FREESCALE_DSPI4_CS3
    CYGHWR_IO_FREESCALE_SPI4_PIN_CS3,
#endif
#ifdef CYGHWR_FREESCALE_DSPI4_CS4
    CYGHWR_IO_FREESCALE_SPI4_PIN_CS4,
#endif
#ifdef CYGHWR_FREESCALE_DSPI4_CS5
    CYGHWR_IO_FREESCALE_SPI4_PIN_CS5
#endif
};

#ifdef CYGINT_DEVS_SPI_DSPI4_USES_DMA
  DSPI_EDMA_CHAN_SET(4);
# define DSPI4_EDMA_SET_P DSPI_EDMA_SET_P_YES(4)
# define DSPI4_EDMA_TCD DSPI_EDMA_TCD_YES(4)
#else
# define DSPI4_EDMA_SET_P DSPI_EDMA_SET_P_NULL(4)
# define DSPI4_EDMA_TCD DSPI_EDMA_TCD_NULL(4)
#endif // CYGINT_DEVS_SPI_DSPI4_USES_DMA

DSPI_BUS_PINS(4);
DSPI_BUS_SETUP(4);
DSPI_BUS(4);

#endif // CYGHWR_DEVS_SPI_FREESCALE_DSPI4

// DSPI BUS 5 ================================================================
#ifdef CYGHWR_DEVS_SPI_FREESCALE_DSPI5

#define  CYGBLD_DSPI5_TCD_SECTION \
    CYGBLD_ATTRIB_SECTION(CYGOPT_DEVS_SPI_FREESCALE_DSPI5_TCD_SECTION)

#ifdef CYHGWR_DEVS_SPI_FREESCALE_DSPI5_PCSS
#define CYGHWR_FREESCALE_DSPI5_MCR_PCSSE FREESCALE_DSPI_MCR_PCSSE_M
#else
#define CYGHWR_FREESCALE_DSPI5_MCR_PCSSE 0
#endif

// SPI chip select pins.
static const cyg_uint32 spi5_cs_pins[] = {
#ifdef CYGHWR_FREESCALE_DSPI5_CS0
    CYGHWR_IO_FREESCALE_SPI5_PIN_CS0,
#endif
#ifdef CYGHWR_FREESCALE_DSPI5_CS1
    CYGHWR_IO_FREESCALE_SPI5_PIN_CS1,
#endif
#ifdef CYGHWR_FREESCALE_DSPI5_CS2
    CYGHWR_IO_FREESCALE_SPI5_PIN_CS2,
#endif
#ifdef CYGHWR_FREESCALE_DSPI5_CS3
    CYGHWR_IO_FREESCALE_SPI5_PIN_CS3,
#endif
#ifdef CYGHWR_FREESCALE_DSPI5_CS4
    CYGHWR_IO_FREESCALE_SPI5_PIN_CS4,
#endif
#ifdef CYGHWR_FREESCALE_DSPI5_CS5
    CYGHWR_IO_FREESCALE_SPI5_PIN_CS5
#endif
};

#ifdef CYGINT_DEVS_SPI_DSPI5_USES_DMA
  DSPI_EDMA_CHAN_SET(5);
# define DSPI5_EDMA_SET_P DSPI_EDMA_SET_P_YES(5)
# define DSPI5_EDMA_TCD DSPI_EDMA_TCD_YES(5)
#else
# define DSPI5_EDMA_SET_P DSPI_EDMA_SET_P_NULL(5)
# define DSPI5_EDMA_TCD DSPI_EDMA_TCD_NULL(5)
#endif // CYGINT_DEVS_SPI_DSPI5_USES_DMA

DSPI_BUS_PINS(5);
DSPI_BUS_SETUP(5);
DSPI_BUS(5);

#endif // CYGHWR_DEVS_SPI_FREESCALE_DSPI5

// DSPI BUS 6 ================================================================
#ifdef CYGHWR_DEVS_SPI_FREESCALE_DSPI6

#define  CYGBLD_DSPI6_TCD_SECTION \
    CYGBLD_ATTRIB_SECTION(CYGOPT_DEVS_SPI_FREESCALE_DSPI6_TCD_SECTION)

#ifdef CYHGWR_DEVS_SPI_FREESCALE_DSPI6_PCSS
#define CYGHWR_FREESCALE_DSPI6_MCR_PCSSE FREESCALE_DSPI_MCR_PCSSE_M
#else
#define CYGHWR_FREESCALE_DSPI6_MCR_PCSSE 0
#endif

// SPI chip select pins.
static const cyg_uint32 spi6_cs_pins[] = {
#ifdef CYGHWR_FREESCALE_DSPI6_CS0
    CYGHWR_IO_FREESCALE_SPI6_PIN_CS0,
#endif
#ifdef CYGHWR_FREESCALE_DSPI6_CS1
    CYGHWR_IO_FREESCALE_SPI6_PIN_CS1,
#endif
#ifdef CYGHWR_FREESCALE_DSPI6_CS2
    CYGHWR_IO_FREESCALE_SPI6_PIN_CS2,
#endif
#ifdef CYGHWR_FREESCALE_DSPI6_CS3
    CYGHWR_IO_FREESCALE_SPI6_PIN_CS3,
#endif
#ifdef CYGHWR_FREESCALE_DSPI6_CS4
    CYGHWR_IO_FREESCALE_SPI6_PIN_CS4,
#endif
#ifdef CYGHWR_FREESCALE_DSPI6_CS5
    CYGHWR_IO_FREESCALE_SPI6_PIN_CS5
#endif
};

#ifdef CYGINT_DEVS_SPI_DSPI6_USES_DMA
  DSPI_EDMA_CHAN_SET(6);
# define DSPI6_EDMA_SET_P DSPI_EDMA_SET_P_YES(6)
# define DSPI6_EDMA_TCD DSPI_EDMA_TCD_YES(6)
#else
# define DSPI6_EDMA_SET_P DSPI_EDMA_SET_P_NULL(6)
# define DSPI6_EDMA_TCD DSPI_EDMA_TCD_NULL(6)
#endif // CYGINT_DEVS_SPI_DSPI6_USES_DMA

DSPI_BUS_PINS(6);
DSPI_BUS_SETUP(6);
DSPI_BUS(6);

#endif // CYGHWR_DEVS_SPI_FREESCALE_DSPI6

// DSPI BUS 7 ================================================================
#ifdef CYGHWR_DEVS_SPI_FREESCALE_DSPI7

#define  CYGBLD_DSPI7_TCD_SECTION \
    CYGBLD_ATTRIB_SECTION(CYGOPT_DEVS_SPI_FREESCALE_DSPI7_TCD_SECTION)

#ifdef CYHGWR_DEVS_SPI_FREESCALE_DSPI7_PCSS
#define CYGHWR_FREESCALE_DSPI7_MCR_PCSSE FREESCALE_DSPI_MCR_PCSSE_M
#else
#define CYGHWR_FREESCALE_DSPI7_MCR_PCSSE 0
#endif

// SPI chip select pins.
static const cyg_uint32 spi7_cs_pins[] = {
#ifdef CYGHWR_FREESCALE_DSPI7_CS0
    CYGHWR_IO_FREESCALE_SPI7_PIN_CS0,
#endif
#ifdef CYGHWR_FREESCALE_DSPI7_CS1
    CYGHWR_IO_FREESCALE_SPI7_PIN_CS1,
#endif
#ifdef CYGHWR_FREESCALE_DSPI7_CS2
    CYGHWR_IO_FREESCALE_SPI7_PIN_CS2,
#endif
#ifdef CYGHWR_FREESCALE_DSPI7_CS3
    CYGHWR_IO_FREESCALE_SPI7_PIN_CS3,
#endif
#ifdef CYGHWR_FREESCALE_DSPI7_CS4
    CYGHWR_IO_FREESCALE_SPI7_PIN_CS4,
#endif
#ifdef CYGHWR_FREESCALE_DSPI7_CS5
    CYGHWR_IO_FREESCALE_SPI7_PIN_CS5
#endif
};

#ifdef CYGINT_DEVS_SPI_DSPI7_USES_DMA
  DSPI_EDMA_CHAN_SET(7);
# define DSPI7_EDMA_SET_P DSPI_EDMA_SET_P_YES(7)
# define DSPI7_EDMA_TCD DSPI_EDMA_TCD_YES(7)
#else
# define DSPI7_EDMA_SET_P DSPI_EDMA_SET_P_NULL(7)
# define DSPI7_EDMA_TCD DSPI_EDMA_TCD_NULL(7)
#endif // CYGINT_DEVS_SPI_DSPI7_USES_DMA

DSPI_BUS_PINS(7);
DSPI_BUS_SETUP(7);
DSPI_BUS(7);

#endif // CYGHWR_DEVS_SPI_FREESCALE_DSPI7

//=============================================================================
#endif // SPI_FREESCALE_DSPI_BUSES_INL
