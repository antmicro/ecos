//==========================================================================
//
//      if_stm32.c
//
//      Fast ethernet device driver for ST STM32 controller
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2010, 2011, 2012, 2013 Free Software Foundation, Inc.
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
// Author(s):    Jerzy Dyrda <jerzdy@gmail.com>
// Contributors:
// Date:         2010-10-05
// Purpose:
// Description:	 hardware driver for STM32 ETH
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/devs_eth_cortexm_stm32.h>
#include <pkgconf/io_eth_drivers.h>
#if defined(CYGPKG_REDBOOT)
   #include <pkgconf/redboot.h>
#endif

#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/plf_io.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/eth_drv_stats.h>
#include <cyg/io/eth_phy.h>
#include <errno.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Maintenance and debug macros.

// Set up the level of debug output
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
#define DIAG_PRINTF(__fmt, ... )                    \
    diag_printf("if_stm32:%s[%d]:"                  \
        _fmt, __FUNCTION__, __LINE__, ## __VA_ARGS__ );
#else
#define DIAG_PRINTF( __fmt, ... )                   \
    while(0){}
#endif

#define IF_STM32_INC_DESC(_curr_desc_, _max_desc_)  \
    _curr_desc_++;                                  \
    if (_curr_desc_ >=	_max_desc_) {               \
        _curr_desc_ = 0;                            \
    }

//---------------------------------------------------------------------
// Shorthand for some of the configuration options.
#define ETH_BASE                    CYGHWR_HAL_STM32_ETH
#define TDES_NUM                    (CYGNUM_IO_ETH_DRIVERS_SG_LIST_SIZE >> 1)
#define RDES_NUM                    CYGNUM_DEVS_ETH_CORTEXM_STM32_RX_BUFS
#define ETH_MAX_PACKET_SIZE         1520

// Bits in status register of tx descriptor which are set up during
// initialization and remain unchanged while sending process
#define ETH_DMA_TDES_INIT_MASK    \
    (CYGHWR_HAL_STM32_ETH_TDES0_TER | CYGHWR_HAL_STM32_ETH_TDES0_CIC_HPP)

//Driver interface callbacks
#define _eth_drv_init(sc,mac)				\
    (sc->funs->eth_drv->init)(sc,(unsigned char *)mac)
#define _eth_drv_tx_done(sc,key,status)			\
    (sc->funs->eth_drv->tx_done)(sc,key,status)
#define _eth_drv_recv(sc,len)				\
    (sc->funs->eth_drv->recv)(sc,len)

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
static cyg_uint32 stm32_eth_isr (cyg_vector_t vector, cyg_addrword_t data);
#endif

// --------------------------------------------------------------
// RedBoot configuration options for managing ESAs for us

// Decide whether to have redboot config vars for it
#if defined(CYGSEM_REDBOOT_FLASH_CONFIG) && defined(CYGPKG_REDBOOT_NETWORKING)
    #include <redboot.h>
    #include <flash_config.h>

    #ifdef CYGSEM_DEVS_ETH_CORTEXM_STM32_REDBOOT_HOLDS_ESA_ETH0
RedBoot_config_option("Network hardware address [MAC] for eth0",
                      eth0_esa_data,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0);
    #endif

#endif  // CYGPKG_REDBOOT_NETWORKING && CYGSEM_REDBOOT_FLASH_CONFIG

// and initialization code to read them
// - independent of whether we are building RedBoot right now:
#ifdef CYGPKG_DEVS_ETH_CORTEXM_STM32_REDBOOT_HOLDS_ESA

    #include <cyg/hal/hal_if.h>

    #ifndef CONFIG_ESA
        #define CONFIG_ESA (6)
    #endif

    #define CYGHWR_DEVS_ETH_CORTEXM_STM32_GET_ESA( mac_address, ok )	\
    CYG_MACRO_START                                                     \
    ok = CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,     \
                                    "eth0_esa_data",                    \
                                    mac_address,                        \
                                    CONFIG_ESA);                        \
    CYG_MACRO_END

#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_REDBOOT_HOLDS_ESA

//============================================================================

// Private Data
typedef struct stm32_eth_dma_des_s {
    cyg_uint32   status;          // DES0 : Own and Status
    cyg_uint32   ctrl_len;        // DES1 : Ctrl/Buffer1 and Buffer2 length
    cyg_uint32   buff1_addr;      // DES2 : Buffer1 address
    cyg_uint32   buff2_addr;      // DES3 : Buffer2 address
} stm32_eth_dma_des_t;

static cyg_uint8 __attribute__ ((aligned (4)))
    rx_buff[RDES_NUM][ETH_MAX_PACKET_SIZE];
static stm32_eth_dma_des_t __attribute__ ((aligned (4))) rdes[RDES_NUM];

static stm32_eth_dma_des_t __attribute__ ((aligned (4))) tdes[TDES_NUM];

// STM32 Ethernet private data
typedef struct stm32_eth_priv_s
{
   cyg_uint32    intr_vector;
   char * esa_key;      // RedBoot 'key' for device ESA
   eth_phy_access_t *phy;
   unsigned long tx_key[TDES_NUM];
   cyg_uint32 curr_tdes_idx;
   cyg_uint32 prev_tdes_idx;
   cyg_uint32 curr_rdes_idx;
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
   cyg_interrupt intr;
   cyg_handle_t  intr_handle;
#endif
} stm32_eth_priv_t;

__externC cyg_uint32 hal_stm32_hclk;
static cyg_uint32 mdc_clock_div;

//============================================================================
// PHY access bits and pieces

static void stm32_init_phy(void)
{
    int stm32_hclk_mhz = hal_stm32_hclk / 1000000;
    mdc_clock_div = 0;

    CYG_ASSERTC(CYGHWR_HAL_STM32_ETH_MACMIIAR_CR_MHZ_CHECK(stm32_hclk_mhz));

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
    DIAG_PRINTF("init phy: hclk %d MHz\n", stm32_hclk_mhz);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2

    if (CYGHWR_HAL_STM32_ETH_MACMIIAR_CR_MHZ_CHECK(stm32_hclk_mhz))
        mdc_clock_div = CYGHWR_HAL_STM32_ETH_MACMIIAR_CR_MHZ(stm32_hclk_mhz);
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
    else
       DIAG_PRINTF("init phy : clk for ETH module out of supported range\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
}

// Write one of the PHY registers via the MII bus

static void stm32_write_phy(int reg_addr, int phy_addr, unsigned short data)
{
    cyg_uint32 addr = 0;
    volatile cyg_uint32 reg_val;

    CYG_ASSERTC(reg_addr >= 0 && reg_addr < 32);
    CYG_ASSERTC(phy_addr >= 0 && phy_addr < 32);

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
    DIAG_PRINTF("write phy: addr %d, reg %d, data 0x%x\n",
        phy_addr, reg_addr, data);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2

    /* Prepare the ENET MIIA address */
    addr |= CYGHWR_HAL_STM32_ETH_MACMIIAR_PA(phy_addr);
    addr |= CYGHWR_HAL_STM32_ETH_MACMIIAR_MR(reg_addr);
    addr |= mdc_clock_div;
    addr |= CYGHWR_HAL_STM32_ETH_MACMIIAR_MW;
    addr |= CYGHWR_HAL_STM32_ETH_MACMIIAR_MB;

    do
    {
        HAL_READ_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_MACMIIAR, reg_val);
    } while (reg_val & CYGHWR_HAL_STM32_ETH_MACMIIAR_MB);

    // Lauch MDIO transaction
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_MACMIIDR, data);
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_MACMIIAR, addr);
}

// Read one of the PHY registers via the MII bus

static bool stm32_read_phy(int reg_addr, int phy_addr, unsigned short *data)
{
    cyg_uint32 addr = 0;
    volatile cyg_uint32 reg_val;

    CYG_ASSERTC(reg_addr >= 0 && reg_addr < 32);
    CYG_ASSERTC(phy_addr >= 0 && phy_addr < 32);

    /* Prepare the ENET MIIA address */
    addr |= CYGHWR_HAL_STM32_ETH_MACMIIAR_PA(phy_addr);
    addr |= CYGHWR_HAL_STM32_ETH_MACMIIAR_MR(reg_addr);
    addr |= mdc_clock_div;
    addr |= CYGHWR_HAL_STM32_ETH_MACMIIAR_MB;

    do
    {
        HAL_READ_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_MACMIIAR, reg_val);
    } while (reg_val & CYGHWR_HAL_STM32_ETH_MACMIIAR_MB);

    // Lauch MDIO transaction
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_MACMIIAR, addr);

    do
    {
        HAL_READ_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_MACMIIAR, reg_val);
    } while (reg_val & CYGHWR_HAL_STM32_ETH_MACMIIAR_MB);

    HAL_READ_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_MACMIIDR, *data);

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
    DIAG_PRINTF("read phy: addr %d, reg %d, data 0x%x\n",
        phy_addr, reg_addr, *data);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2

    return (true);
}

ETH_PHY_REG_LEVEL_ACCESS_FUNS(stm32_phy,
                              stm32_init_phy,
                              NULL,
                              stm32_write_phy,
                              stm32_read_phy);

//======================================================================
// Receiver buffer handling

// Initialize the receiver buffers and descriptors in ring mode

static void stm32_rdes_init(void)
{
    int ii;

    for (ii = 0; ii < RDES_NUM; ii++) {
        rdes[ii].status = (cyg_uint32) CYGHWR_HAL_STM32_ETH_RDES0_OWN;
        rdes[ii].ctrl_len = (cyg_uint32) ETH_MAX_PACKET_SIZE;
        rdes[ii].buff1_addr = (cyg_uint32) &rx_buff[ii][0];
    }

    // End of ring
    rdes[RDES_NUM - 1].ctrl_len |=
        (cyg_uint32) CYGHWR_HAL_STM32_ETH_RDES1_RER;

    /* Set Receive Desciptor List Address Register */
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMARDLAR,
        ((cyg_uint32) &rdes[0]));
}

//======================================================================
// Transmit buffer handling

// Initialize the transmit buffer descriptors in ring mode

static void stm32_tdes_init(void)
{

#ifdef CYGSEM_DEVS_ETH_CORTEXM_STM32_TX_CHECKSUM_GEN
    int ii;

    for (ii = 0; ii < TDES_NUM; ii++) {
        tdes[ii].status = (cyg_uint32) CYGHWR_HAL_STM32_ETH_TDES0_CIC_HPP;
    }
#endif

    // End of ring
    tdes[TDES_NUM - 1].status |= (cyg_uint32) CYGHWR_HAL_STM32_ETH_TDES0_TER;

    /* Set Receive Desciptor List Address Register */
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMATDLAR,
        ((cyg_uint32) &tdes[0]));
}

//======================================================================
// Initialization code

// Set a specific address match to a given address. Packets received which
// match this address will be passed on.

static void stm32_set_mac(cyg_uint8 * enaddr)
{
    cyg_uint32 hi, lo;

    lo = ((enaddr[3] << 24) | (enaddr[2] << 16) |
	    (enaddr[1] <<  8) | (enaddr[0]));

    hi = ((enaddr[5] <<  8) | (enaddr[4]));

    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_MACA0LR, lo);
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_MACA0HR, hi);
}

// Enable and Disable of the receiver and transmitter.
// Initialize the interface. This configures the interface ready for use.
// Interrupts are grabbed etc. This means the start function has
// little to do except enable the receiver

static bool stm32_eth_init(struct cyg_netdevtab_entry *tab)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
    stm32_eth_priv_t *priv = (stm32_eth_priv_t *)sc->driver_private;
#ifdef CYGHWR_DEVS_ETH_ARM_STM32_GET_ESA
    bool esa_ok = false;
#endif
    unsigned char enaddr[6] = { CYGPKG_DEVS_ETH_CORTEXM_STM32_MACADDR};
    volatile cyg_uint32 reg_val;
    unsigned short phy_state = 0;

#ifdef CYGHWR_DEVS_ETH_CORTEXM_STM32_PHY_CLK_MCO
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MCO);
#endif

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
    DIAG_PRINTF("Initialising ETH module\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0

    // Enable clock for ETH module
    HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_AHB1ENR, reg_val);
    reg_val |= BIT_(CYGHWR_HAL_STM32_RCC_AHB1ENR_ETHMAC) |
        BIT_(CYGHWR_HAL_STM32_RCC_AHB1ENR_ETHMACTX) |
        BIT_(CYGHWR_HAL_STM32_RCC_AHB1ENR_ETHMACRX) |
        BIT_(CYGHWR_HAL_STM32_RCC_AHB1ENR_ETHMACPTP);
    HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_AHB1ENR, reg_val);

#if defined (CYGHWR_HAL_CORTEXM_STM32_FAMILY_HIPERFORMANCE)
    HAL_READ_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB2ENR, reg_val);
    reg_val |= BIT_(CYGHWR_HAL_STM32_RCC_APB2ENR_SYSCFG);
    HAL_WRITE_UINT32(CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_APB2ENR, reg_val);
#endif
    hal_delay_us(100);

    // Configure the GPIO ports
#ifdef CYGSEM_DEVS_ETH_CORTEXM_STM32_INTF_MII

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
    DIAG_PRINTF("Initialise MII interface\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0

    // Configure MAC interface in MII mode
#ifdef CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1
    HAL_READ_UINT32(CYGHWR_HAL_STM32_AFIO + CYGHWR_HAL_STM32_AFIO_MAPR, reg_val);
    reg_val &= ~CYGHWR_HAL_STM32_AFIO_MAPR_ETH_RMII;
    HAL_WRITE_UINT32(CYGHWR_HAL_STM32_AFIO + CYGHWR_HAL_STM32_AFIO_MAPR, reg_val);
#else
    HAL_WRITE_UINT32(CYGHWR_HAL_STM32_SYSCFG + CYGHWR_HAL_STM32_SYSCFG_PMC,
        CYGHWR_HAL_STM32_SYSCFG_PMC_MII);
#endif // CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1

    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_TX_CLK);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_RX_CLK);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_MDIO);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_MDC);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_COL);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_TX_CRS);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_TX_EN);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_TXD0);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_TXD1);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_TXD2);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_TXD3);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_RX_ER);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_RX_DV);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_RXD0);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_RXD1);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_RXD2);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_MII_RXD3);
#elif defined(CYGSEM_DEVS_ETH_CORTEXM_STM32_INTF_RMII)

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
    DIAG_PRINTF("Initialise RMII interface\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0

    // Configure MAC interface in RMII mode
#ifdef CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1
    HAL_READ_UINT32(CYGHWR_HAL_STM32_AFIO + CYGHWR_HAL_STM32_AFIO_MAPR, reg_val);
    reg_val |= CYGHWR_HAL_STM32_AFIO_MAPR_ETH_RMII;
    HAL_WRITE_UINT32(CYGHWR_HAL_STM32_AFIO + CYGHWR_HAL_STM32_AFIO_MAPR, reg_val);
#else
    HAL_WRITE_UINT32(CYGHWR_HAL_STM32_SYSCFG + CYGHWR_HAL_STM32_SYSCFG_PMC,
        CYGHWR_HAL_STM32_SYSCFG_PMC_RMII);
#endif // CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1

    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_RMII_REF_CLK);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_RMII_MDIO);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_RMII_MDC);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_RMII_TX_EN);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_RMII_TXD0);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_RMII_TXD1);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_RMII_CRS_DV);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_RMII_RXD0);
    CYGHWR_HAL_STM32_GPIO_SET(CYGHWR_HAL_STM32_ETH_RMII_RXD1);
#else
#error "Neither MII nor RMII interface defined"
#endif // CYGSEM_DEVS_ETH_CORTEXM_STM32_INTF_MII

#ifdef CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1
    HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_AFIO + CYGHWR_HAL_STM32_RCC_AHBRSTR,
        CYGHWR_HAL_STM32_RCC_AHBRSTR_ETHMACRST);
    hal_delay_us(1000);
    HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_AFIO + CYGHWR_HAL_STM32_RCC_AHBRSTR, 0);
#else
    HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_AHB1RSTR,
        CYGHWR_HAL_STM32_RCC_AHB1RSTR_ETHMACRST);
    hal_delay_us(1000);
    HAL_WRITE_UINT32 (CYGHWR_HAL_STM32_RCC + CYGHWR_HAL_STM32_RCC_AHB1RSTR, 0);
#endif // CYGHWR_HAL_CORTEXM_STM32_FAMILY_F1

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
    DIAG_PRINTF("Reset ETH DMA module\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0

    // Software reset of ETH module and wait until it will be done
    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMABMR,
        CYGHWR_HAL_STM32_ETH_DMABMR_SR);

    do {
        hal_delay_us(100);
        HAL_READ_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMABMR, reg_val);
    } while (reg_val & CYGHWR_HAL_STM32_ETH_DMABMR_SR);

    // Setup the PHY
    CYG_ASSERTC(priv->phy);

    if (!_eth_phy_init(priv->phy))
    {
        return (false);
    }

    reg_val = 0;

    // Get the current mode and print it
    phy_state = _eth_phy_state(priv->phy);

    if ((phy_state & ETH_PHY_STAT_LINK) != 0) {
        //Connection mode
        if((phy_state & ETH_PHY_STAT_FDX)) {
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
            DIAG_PRINTF("Link type : Full Duplex\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
            reg_val |= CYGHWR_HAL_STM32_ETH_MACCR_DM;
	    } else {
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
            DIAG_PRINTF("Link type : Half Duplex\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
            reg_val &= ~(CYGHWR_HAL_STM32_ETH_MACCR_DM);
	    }
	    //Connection speed
	    if((phy_state & ETH_PHY_STAT_100MB)) {
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
            DIAG_PRINTF("Link speed : 100Mbits/s\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
            reg_val |= CYGHWR_HAL_STM32_ETH_MACCR_FES;
	    } else {
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
            DIAG_PRINTF("Link speed : 10Mbits/s\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
		    reg_val &= ~(CYGHWR_HAL_STM32_ETH_MACCR_FES);
	    }
    } else {
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
	    DIAG_PRINTF("No Link\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
    }
    // ------------------- Configure MAC module -------------------

    reg_val |= CYGHWR_HAL_STM32_ETH_MACCR_RD;

#ifdef CYGSEM_DEVS_ETH_CORTEXM_STM32_RX_CHECKSUM_VER
    reg_val |= CYGHWR_HAL_STM32_ETH_MACCR_IPCO;
#endif

    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_MACCR, reg_val);

    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_MACFFR,
        (CYGHWR_HAL_STM32_ETH_MACFFR_HU |
        CYGHWR_HAL_STM32_ETH_MACFFR_HM |
        CYGHWR_HAL_STM32_ETH_MACFFR_HPF));

    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_MACHTHR, 0);
    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_MACHTLR, 0);

    // Give the MAC its ethernet address
    stm32_set_mac(enaddr);

    // ------------------- Configure DMA module -------------------

#ifdef CYGSEM_DEVS_ETH_CORTEXM_STM32_TX_CHECKSUM_GEN
    // The checksum generation is enabled only if the
    // Transmit FIFO is configured for Store-and-forward mode
    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAOMR,
        CYGHWR_HAL_STM32_ETH_DMAOMR_TSF);
#else
    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAOMR,
        CYGHWR_HAL_STM32_ETH_DMAOMR_TTC(3) /* TTC = 256 */);
#endif

    // Clear any pending interrupt and disable it
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR, 0x1FFFF);
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAIER, 0x0);

    // If we are building an interrupt enabled version, install the
    // interrupt handler
#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
    DIAG_PRINTF("Installing Interrupts on IRQ %d\n", priv->intr_vector);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0

    cyg_drv_interrupt_create(priv->intr_vector,
        4,
        (cyg_addrword_t)sc,
        stm32_eth_isr,
        eth_drv_dsr,
        &priv->intr_handle,
        &priv->intr);

    cyg_drv_interrupt_attach(priv->intr_handle);
    cyg_drv_interrupt_unmask(priv->intr_vector);
#endif // CYGINT_IO_ETH_INT_SUPPORT_REQUIRED

#ifdef CYGHWR_DEVS_ETH_ARM_STM32_GET_ESA
    // Get MAC address from RedBoot configuration variables
    CYGHWR_DEVS_ETH_ARM_STM32_GET_ESA(&enaddr[0], esa_ok);
    // If this call fails myMacAddr is unchanged and MAC address from
    // CDL is used

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
    if (!esa_ok)
    {
        // Can't figure out ESA
        DIAG_PRINTF("Warning! ESA unknown\n");
    }
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
#endif // CYGHWR_DEVS_ETH_ARM_STM32_GET_ESA

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
    DIAG_PRINTF("MAC addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
        enaddr[0],enaddr[1],enaddr[2],
        enaddr[3],enaddr[4],enaddr[5]);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1

    // Initialize Rx/Tx Descriptors list
    stm32_tdes_init();
    stm32_rdes_init();

    // Enable transmit and receive state machine of the MAC
    HAL_READ_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_MACCR, reg_val);
    reg_val |= CYGHWR_HAL_STM32_ETH_MACCR_TE |
        CYGHWR_HAL_STM32_ETH_MACCR_RE;
    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_MACCR, reg_val);

    // Flush Transmit FIFO
    HAL_READ_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAOMR, reg_val);
    reg_val |= CYGHWR_HAL_STM32_ETH_DMAOMR_FTF;
    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAOMR, reg_val);

    do {
        HAL_READ_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAOMR, reg_val);
    } while (reg_val & CYGHWR_HAL_STM32_ETH_DMAOMR_FTF);

    // Initialize the upper layer driver
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
    DIAG_PRINTF("Initialize the upper layer driver\n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
    _eth_drv_init(sc,enaddr);

    return (true);
}

// This function is called to stop the interface.

static void stm32_eth_stop(struct eth_drv_sc *sc)
{
    volatile cyg_uint32 reg_val;

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
    DIAG_PRINTF("Stoping \n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1

    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR, 0x1FFFF);
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAIER, 0x0);

    HAL_READ_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAOMR, reg_val);
    reg_val &= ~(CYGHWR_HAL_STM32_ETH_DMAOMR_SR |
        CYGHWR_HAL_STM32_ETH_DMAOMR_ST);
    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAOMR, reg_val);
}

// This function is called to "start up" the interface. It may be called
// multiple times, even when the hardware is already running.

static void stm32_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr,
        int flags)
{
    volatile cyg_uint32 reg_val;

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
    DIAG_PRINTF("Starting \n");
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1

    // Enable the interrupts we are interested in
    HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAIER,
        (CYGHWR_HAL_STM32_ETH_DMAIER_RIE |
        CYGHWR_HAL_STM32_ETH_DMAIER_TIE |
        CYGHWR_HAL_STM32_ETH_DMAIER_NISE));

    // Start DMA transmission and reception
    HAL_READ_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAOMR, reg_val);
    reg_val |= CYGHWR_HAL_STM32_ETH_DMAOMR_SR |
        CYGHWR_HAL_STM32_ETH_DMAOMR_ST;
    HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMAOMR, reg_val);
}

// This function is called for low level "control" operations

static int stm32_eth_control(struct eth_drv_sc *sc, unsigned long key,
        void *data, int length)
{
    switch (key)
    {
        case ETH_DRV_SET_MAC_ADDRESS:
            if(length >= ETHER_ADDR_LEN) {
                stm32_eth_stop(sc);
                cyg_uint8 * enaddr = (cyg_uint8 *)data;

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
                DIAG_PRINTF("Change MAC addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    enaddr[0],enaddr[1],enaddr[2],
                    enaddr[3],enaddr[4],enaddr[5]);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
                stm32_set_mac(enaddr);
                stm32_eth_start(sc,enaddr,0);
                return 0;
            }
            return 1;

#ifdef CYGPKG_NET
        case ETH_DRV_GET_IF_STATS:
        {
            // todo
            return 1;
        }
#endif

        default:
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
            DIAG_PRINTF("Unsupported key %lx\n", key);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
           return 1;
    }
}

// This routine is called to receive data from the hardware

static void stm32_eth_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
        int sg_len)
{
    stm32_eth_priv_t *priv = (stm32_eth_priv_t *)sc->driver_private;
    int ii, rx_frame_length;
    cyg_uint32 rx_buffer_pos = 0, data_to_copy;
    cyg_uint8 * psg_buf, * prx_buff;

    prx_buff = (cyg_uint8 *) rx_buff[priv->curr_rdes_idx];
    rx_frame_length =
        CYGHWR_HAL_STM32_ETH_RDES0_FL(rdes[priv->curr_rdes_idx].status) - 4;

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
    DIAG_PRINTF("current rx desc %d, sg_len %d, total_len %d\n",
        priv->curr_rdes_idx, sg_len, rx_frame_length);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2

    for(ii = 0; (ii < sg_len) && (rx_frame_length > 0); ii++) {
        psg_buf = (cyg_uint8 *)(sg_list[ii].buf);
        data_to_copy = rx_frame_length > sg_list[ii].len ?
        sg_list[ii].len : rx_frame_length;
        memcpy(psg_buf, &prx_buff[rx_buffer_pos], data_to_copy);
        // frame copied
        rx_buffer_pos += data_to_copy;
        rx_frame_length -= data_to_copy;
    }
}

// This function is called to see if another packet can be sent.

static int stm32_eth_can_send(struct eth_drv_sc *sc)
{
    stm32_eth_priv_t *priv = (stm32_eth_priv_t *)sc->driver_private;

    return ((tdes[priv->curr_tdes_idx].status &
        CYGHWR_HAL_STM32_ETH_TDES0_OWN) ? 0 : 1);
}

// This routine is called to send data to the hardware - zero copying version

static void stm32_eth_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list,
        int sg_len, int total_len, unsigned long key)
{
    stm32_eth_priv_t *priv = (stm32_eth_priv_t *) sc->driver_private;
    cyg_uint8 sg_item = 0, sg_len_even, sg_len_rest;
    volatile cyg_uint32 reg_val;

    CYG_ASSERT(total_len <= ETH_MAX_PACKET_SIZE, "packet too long");

    sg_len_even = sg_len / 2;
    sg_len_rest = sg_len % 2;

    // Look if it's enough numer of free descriptors
    CYG_ASSERT((sg_len_even + sg_len_rest) <
        ((priv->curr_tdes_idx >= priv->prev_tdes_idx) ?
        (TDES_NUM - (priv->curr_tdes_idx - priv->prev_tdes_idx)) :
        (priv->prev_tdes_idx - priv->curr_tdes_idx)),
        "not enough number of descriptors");

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
    DIAG_PRINTF("current tx desc %d, sg_len %d, total_len %d\n",
        priv->curr_tdes_idx, sg_len, total_len);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2

    for(sg_item = 0; sg_item < 2 * sg_len_even; sg_item += 2) {
        tdes[priv->curr_tdes_idx].status &= ETH_DMA_TDES_INIT_MASK;
        tdes[priv->curr_tdes_idx].buff1_addr = sg_list[sg_item].buf;
        tdes[priv->curr_tdes_idx].ctrl_len =
        CYGHWR_HAL_STM32_ETH_TDES1_TBS1(sg_list[sg_item].len);
        tdes[priv->curr_tdes_idx].buff2_addr = sg_list[sg_item + 1].buf;
        tdes[priv->curr_tdes_idx].ctrl_len |=
        CYGHWR_HAL_STM32_ETH_TDES1_TBS2(sg_list[sg_item + 1].len);

        if (sg_item == 0)
            tdes[priv->curr_tdes_idx].status |=
                CYGHWR_HAL_STM32_ETH_TDES0_FS;

        if ((sg_item == sg_len - 2) && !sg_len_rest) {
            tdes[priv->curr_tdes_idx].status |=
                CYGHWR_HAL_STM32_ETH_TDES0_IC |
                CYGHWR_HAL_STM32_ETH_TDES0_LS;
        }
        // All flags set thus gives the buffer back to DMA
        tdes[priv->curr_tdes_idx].status |=
            CYGHWR_HAL_STM32_ETH_TDES0_OWN;
        priv->tx_key[priv->curr_tdes_idx] = key;

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
        DIAG_PRINTF("tx desc %d : status %x, len1 %d, len2 %d\n",
            priv->curr_tdes_idx, tdes[priv->curr_tdes_idx].status,
            sg_list[sg_item].len, sg_list[sg_item + 1].len);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2

        IF_STM32_INC_DESC(priv->curr_tdes_idx, TDES_NUM);
    }

    if (sg_len_rest) {
        tdes[priv->curr_tdes_idx].status &= ETH_DMA_TDES_INIT_MASK;
        tdes[priv->curr_tdes_idx].buff1_addr = sg_list[sg_item].buf;
        tdes[priv->curr_tdes_idx].ctrl_len =
            CYGHWR_HAL_STM32_ETH_TDES1_TBS1(sg_list[sg_item].len);
        tdes[priv->curr_tdes_idx].buff2_addr = 0;
        if (sg_item == 0) // Only one sg element to send
            tdes[priv->curr_tdes_idx].status |=
                CYGHWR_HAL_STM32_ETH_TDES0_FS;

        // All flags set thus gives the buffer back to DMA
        tdes[priv->curr_tdes_idx].status |=
            CYGHWR_HAL_STM32_ETH_TDES0_OWN |
            CYGHWR_HAL_STM32_ETH_TDES0_IC |
            CYGHWR_HAL_STM32_ETH_TDES0_LS;
        priv->tx_key[priv->curr_tdes_idx] = key;

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
        DIAG_PRINTF("tx desc %d : status %x, len1 %d\n",
        priv->curr_tdes_idx, tdes[priv->curr_tdes_idx].status,
        sg_list[sg_item].len);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2

        IF_STM32_INC_DESC(priv->curr_tdes_idx, TDES_NUM);
    }

    // Starts desc refetch by DMA
    HAL_READ_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR, reg_val);
    if (reg_val & CYGHWR_HAL_STM32_ETH_DMASR_TBUS) {
        HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR,
            CYGHWR_HAL_STM32_ETH_DMASR_TBUS);
        HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMATPDR, 0);
    }

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
    DIAG_PRINTF("packets put into dma - number of used desc %d\n",
        sg_len_even + sg_len_rest);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
}

//======================================================================

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
static cyg_uint32 stm32_eth_isr (cyg_vector_t vector, cyg_addrword_t data)
{
    volatile cyg_uint32 status_reg;
    cyg_uint32 status = CYG_ISR_HANDLED;

    HAL_READ_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR, status_reg);
    if (status_reg & CYGHWR_HAL_STM32_ETH_DMASR_NIS) {
        cyg_interrupt_mask(vector);
        status |= CYG_ISR_CALL_DSR;
    }
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
    // It seems that comes other eth int even it's nothing more
    // enabled than Trans/Recv interrupt
    else {
        DIAG_PRINTF("unexpected eth int - reason 0x%x\n",
            status_reg);
    }
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1

    cyg_interrupt_acknowledge(vector);

    return(status);
}
#endif

static void stm32_eth_deliver(struct eth_drv_sc *sc)
{
    stm32_eth_priv_t *priv = (stm32_eth_priv_t *) sc->driver_private;
    volatile cyg_uint32 status_reg;
    cyg_uint32 rx_frame_length;

    // Get the interrupt status
    HAL_READ_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR, status_reg);

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1
    DIAG_PRINTF("DMA status 0x%x\n", status_reg);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 1

    if (status_reg & CYGHWR_HAL_STM32_ETH_DMASR_RS) {
        HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR,
            CYGHWR_HAL_STM32_ETH_DMASR_NIS |
            CYGHWR_HAL_STM32_ETH_DMASR_RS);

        // check if desc is ready to service - just in case
        while (!(rdes[priv->curr_rdes_idx].status &
            CYGHWR_HAL_STM32_ETH_RDES0_OWN)) {

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
            DIAG_PRINTF("New packet in rx desc %d\n",
                priv->curr_rdes_idx);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2

            // and eth frame is ok
            if (((rdes[priv->curr_rdes_idx].status &
                    CYGHWR_HAL_STM32_ETH_RDES0_ES) == 0) &&
                ((rdes[priv->curr_rdes_idx].status &
                        CYGHWR_HAL_STM32_ETH_RDES0_FS) != 0) &&
                ((rdes[priv->curr_rdes_idx].status &
                    CYGHWR_HAL_STM32_ETH_RDES0_LS) != 0) &&
                (CYGHWR_HAL_STM32_ETH_RDES0_FL(
                    rdes[priv->curr_rdes_idx].status) != 0)) {

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
                DIAG_PRINTF("Rcv packet status 0x%x\n",
                    rdes[priv->curr_rdes_idx].status);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
                rx_frame_length =
                    CYGHWR_HAL_STM32_ETH_RDES0_FL(
                        rdes[priv->curr_rdes_idx].status) - 4;
                _eth_drv_recv(sc, rx_frame_length);
            }
#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
            else {
                DIAG_PRINTF("Error in rcv frame - reason 0x%x\n",
                    rdes[priv->curr_rdes_idx].status);
            }
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0

            // Starts desc refetch by DMA
            if (status_reg & CYGHWR_HAL_STM32_ETH_DMASR_RBUS) {
                HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR,
                CYGHWR_HAL_STM32_ETH_DMASR_RBUS);
                HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMARPDR, 0);
            }

            // Desc handeled
            rdes[priv->curr_rdes_idx].status = CYGHWR_HAL_STM32_ETH_RDES0_OWN;
            IF_STM32_INC_DESC(priv->curr_rdes_idx, RDES_NUM);
        }
    }

    if (status_reg & CYGHWR_HAL_STM32_ETH_DMASR_TS) {
        HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR,
            CYGHWR_HAL_STM32_ETH_DMASR_NIS | CYGHWR_HAL_STM32_ETH_DMASR_TS);

        // Walk throug descriptor list until find serviced last segment
        while ((priv->prev_tdes_idx != priv->curr_tdes_idx) &&
            !(tdes[priv->prev_tdes_idx].status &
                CYGHWR_HAL_STM32_ETH_TDES0_OWN)) {

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2
            DIAG_PRINTF("Sent packet (key 0x%x) from desc %d with status 0x%x\n",
                (cyg_uint32) priv->tx_key[priv->prev_tdes_idx],
            priv->prev_tdes_idx, tdes[priv->prev_tdes_idx].status);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 2

            // Starts desc refetch by DMA if it happens error during transmit
            if (tdes[priv->prev_tdes_idx].status &
                    CYGHWR_HAL_STM32_ETH_TDES0_ES) {
                HAL_WRITE_UINT32(ETH_BASE + CYGHWR_HAL_STM32_ETH_DMASR,
                    CYGHWR_HAL_STM32_ETH_DMASR_TUS);
                HAL_WRITE_UINT32 (ETH_BASE + CYGHWR_HAL_STM32_ETH_DMATPDR, 0);

#if CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
                DIAG_PRINTF("Error in sending - reason 0x%x\n",
                    tdes[priv->prev_tdes_idx].status);
#endif // CYGPKG_DEVS_ETH_CORTEXM_STM32_DEBUG_LEVEL > 0
            }

            if (tdes[priv->prev_tdes_idx].status &
                    CYGHWR_HAL_STM32_ETH_TDES0_LS) {
                if (!(tdes[priv->prev_tdes_idx].status &
                        CYGHWR_HAL_STM32_ETH_TDES0_ES))
                    _eth_drv_tx_done(sc,
                        priv->tx_key[priv->prev_tdes_idx], 0);
                else
                    _eth_drv_tx_done(sc,
                        priv->tx_key[priv->prev_tdes_idx], -1);
            }
            IF_STM32_INC_DESC(priv->prev_tdes_idx, TDES_NUM);
        }
    }

#ifdef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
    cyg_interrupt_unmask(priv->intr_vector);
#endif
}

// routine called to handle ethernet controller in polled mode
static void stm32_eth_poll(struct eth_drv_sc *sc)
{
    /* Service the buffers */
    stm32_eth_deliver(sc);
}

static int stm32_eth_int_vector(struct eth_drv_sc *sc)
{
    return(CYGNUM_HAL_INTERRUPT_ETH);
}

stm32_eth_priv_t stm32_priv_data =
{
    .intr_vector = CYGNUM_HAL_INTERRUPT_ETH,
    .phy = &stm32_phy
};

ETH_DRV_SC(stm32_sc,
           &stm32_priv_data,      // Driver specific data
           "eth0",                // Name for this interface
           stm32_eth_start,
           stm32_eth_stop,
           stm32_eth_control,
           stm32_eth_can_send,
           stm32_eth_send,
           stm32_eth_recv,
           stm32_eth_deliver,
           stm32_eth_poll,
           stm32_eth_int_vector);

NETDEVTAB_ENTRY(stm32_netdev,
                "stm32",
                stm32_eth_init,
                &stm32_sc);

// EOF if_stm32.c
