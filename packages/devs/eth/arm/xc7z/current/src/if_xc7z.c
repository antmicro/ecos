//==========================================================================
//
//      devs/eth/arm/xc7z/current/src/if_xc7z.c
//
//	    Ethernet driver for Xilinx Zynq 
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2003, 2004 Free Software Foundation, Inc.                  
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
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Ant Micro <www.antmicro.com>
// Date:         2012-07-27
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/hal.h>
#include <pkgconf/io_eth_drivers.h>
#include <pkgconf/devs_eth_arm_xc7z.h>

#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h> 
#include <cyg/hal/hal_cache.h>
#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#include <cyg/io/eth/netdev.h>

// this solves ia_ifa error
#if defined(CYGPKG_KERNEL)
#define _KERNEL
#endif

#include <cyg/io/eth/eth_drv.h>

#ifdef CYGPKG_DEVS_ETH_PHY
#include <cyg/io/eth_phy.h>
#include <pkgconf/devs_eth_phy.h>
#endif //CYGPKG_DEVS_ETH_PHY

#include "if_xc7z.h"

#if defined(CYGPKG_REDBOOT)
#include <pkgconf/redboot.h>
#endif //CYGPKG_REDBOOT

#ifdef CYGPKG_NET
#include <pkgconf/net.h>
#include <net/if.h>
#endif //CYGPKG_NET

#ifdef CYGPKG_DEVS_ETH_ARM_XC7Z_DEBUG
#define DEBUG(_info) diag_printf(_info)
#else
#define DEBUG(...)
#endif

// DMA buffer descriptor structure. Each BD is two words
typedef struct __attribute__ ((__packed__)){
    cyg_uint32 addr;
    cyg_uint32 ctrl;
} zynq_eth_bd;


/* This is an internal structure used to maintain the DMA list */
typedef struct {
    cyg_uint32 firstbdaddr;     /* Virtual address of 1st BD in list */
    cyg_uint32 lastbdaddr;      /* Virtual address of last BD in the list */
    cyg_uint32 length;          /* size of ring in bytes */
    cyg_uint32 separation;      /* Number of bytes between the starting address of adjacent BDs */

    zynq_eth_bd *freehead; /* First BD in the free group */
    zynq_eth_bd *prehead;  /* First BD in the pre-work group */
    zynq_eth_bd *hwhead;   /* First BD in the work group */
    zynq_eth_bd *hwtail;   /* Last BD in the work group */
    zynq_eth_bd *posthead; /* First BD in the post-work group */

    unsigned freecnt;      /* Number of BDs in the free group */
    unsigned hwcnt;        /* Number of BDs in work group */
    unsigned precnt;       /* Number of BDs in pre-work group */
    unsigned postcnt;      /* Number of BDs in post-work group */
    unsigned allcnt;       /* Total Number of BDs for channel */

    int is_rx;             /* Is this an RX or a TX ring? */
} zynq_eth_bdring;


typedef struct
{
    // bd descriptor buffers
    zynq_eth_bd tx_bd[CYGPKG_DEVS_ETH_ARM_XC7Z_TXBUF_COUNT];
    zynq_eth_bd rx_bd[CYGPKG_DEVS_ETH_ARM_XC7Z_RXBUF_COUNT];
    // max link speed
    cyg_uint16 max_link_speed;
    // link speed
    cyg_uint16 link_speed;
    // base addr
    cyg_uint32 base_addr;

#ifdef CYGPKG_DEVS_ETH_PHY
    // phy
    eth_phy_access_t *phy;
#endif //CYGPKG_DEVS_ETH_PHY

    // MAC address
    unsigned char MAC_addr[6];
    // MAC address from CDL
    cyg_uint32 MAC_addr_cdl;
    // MAC source 1=EEPROM, 2=const (set in CDL)
    unsigned char MAC_source;
    // DMA rings 
    zynq_eth_bdring tx_ring;
    zynq_eth_bdring rx_ring;
    // Buffer to store received frames
    char rx_buffer[CYGPKG_DEVS_ETH_ARM_XC7Z_RXBUF_COUNT][CYGPKG_DEVS_ETH_ARM_XC7Z_PACKETLEN];
    int rx_buf_count;
    // Number of rx buffer descriptor that ETHMAC is pointing to
    int rx_head;
    // Array of keys (handlers given by the stack) 
    unsigned long tx_key[CYGPKG_DEVS_ETH_ARM_XC7Z_TXBUF_COUNT];
    // Buffer to store frames for transmission
    char tx_buffer[CYGPKG_DEVS_ETH_ARM_XC7Z_TXBUF_COUNT][CYGPKG_DEVS_ETH_ARM_XC7Z_PACKETLEN];
    int tx_buf_count;
    // Interrupt resources
    int irq_no;
    cyg_handle_t int_handle;
    cyg_interrupt int_object;
    int packet_len;
} zynq_eth_t;

static void zynq_eth_mdio_write(cyg_uint32 base_addr, int mii_id, int phyreg, cyg_uint16 value);
static int zynq_eth_mdio_read(cyg_uint32 base_addr, int mii_id, int phyreg);

volatile zynq_eth_bd *last_added_bd = NULL;
volatile zynq_eth_bd *last_tx_bd = NULL;

#if defined(CYGPKG_DEVS_ETH_ARM_XC7Z_ETH0) || defined(CYGPKG_DEVS_ETH_ARM_XC7Z_ETH1)
#ifdef CYGPKG_DEVS_ETH_PHY                                      
#if CYGHWR_DEVS_ETH_PHY_KSZ90x1
extern int ksz90x1_phy_cfg(eth_phy_access_t *f, int mode);            
#define eth_phy_cfg(f, mode) ksz90x1_phy_cfg(f,mode)                 
#else  //!CYGHWR_DEVS_ETH_PHY_KSZ90x1
#define eth_phy_cfg(f, mode) _eth_phy_state(f)
#endif //CYGHWR_DEVS_ETH_PHY_KSZ90x1
#endif //CYGPKG_DEVS_ETH_PHY
#endif //CYGPKG_DEVS_ETH_ARM_XC7Z_ETH0 || CYGPKG_DEVS_ETH_ARM_XC7Z_ETH1
/******************************************************************************
 *
 * Ethernet 0
 *
 ******************************************************************************/
#ifdef CYGPKG_DEVS_ETH_ARM_XC7Z_ETH0

static void zynq_write_phy0(int reg_addr, int phy_addr, unsigned short data)
{
    if( reg_addr < 32)
        {
            zynq_eth_mdio_write(XEMACPS0_BASE, phy_addr, reg_addr, data);
        }
    else
        {
            zynq_eth_mdio_write(XEMACPS0_BASE, phy_addr, 0x0B, reg_addr | (1<<15) );
            zynq_eth_mdio_write(XEMACPS0_BASE, phy_addr, 0x0C, data);
        }
}


static bool zynq_read_phy0(int reg_addr, int phy_addr, unsigned short *data)
{
    if( reg_addr < 32 )
        {
            *data = zynq_eth_mdio_read(XEMACPS0_BASE, phy_addr, reg_addr) & 0xFFFF;
        }
    else
        {
            zynq_eth_mdio_write(XEMACPS0_BASE, phy_addr, 0x0B, reg_addr);
            *data = zynq_eth_mdio_read(XEMACPS0_BASE, phy_addr, 0x0D) & 0xFFFF;
        }
    
    return true;
}


static void zynq_init_phy0(void)
{ }

static void zynq_reset_phy0(void) 
{ }

#ifdef CYGPKG_DEVS_ETH_PHY
ETH_PHY_REG_LEVEL_ACCESS_FUNS(zynq_phy0,
                              zynq_init_phy0,
                              zynq_reset_phy0,
                              zynq_write_phy0,
                              zynq_read_phy0);
#endif //CYGPKG_DEVS_ETH_PHY

    
zynq_eth_t eth0_data = {
#ifdef CYGPKG_DEVS_ETH_PHY
    .phy = &zynq_phy0,
#endif //CYGPKG_DEVS_ETH_PHY
#ifdef CYGPKG_DEVS_ETH_ARM_XC7Z_MAX_LINK_SPEED_ETH0
    .max_link_speed = CYGPKG_DEVS_ETH_ARM_XC7Z_MAX_LINK_SPEED_ETH0,
#else
    .max_link_speed = 0,
#endif
    .base_addr = XEMACPS0_BASE,
    .irq_no = CYGNUM_HAL_INTERRUPT_GEM0,
    .MAC_source = CYGPKG_DEVS_ETH_ARM_XC7Z_MAC_SOURCE_ETH0,
    .MAC_addr_cdl = CYGPKG_DEVS_ETH_ARM_XC7Z_MAC_ADDR_ETH0,
    .tx_buf_count = CYGPKG_DEVS_ETH_ARM_XC7Z_TXBUF_COUNT,
    .rx_buf_count = CYGPKG_DEVS_ETH_ARM_XC7Z_RXBUF_COUNT,
    .packet_len = CYGPKG_DEVS_ETH_ARM_XC7Z_PACKETLEN
};


ETH_DRV_SC(zynq_eth0_sc,
           (void*) &eth0_data,
           "eth0",
           zynq_eth_start,
           zynq_eth_stop,
           zynq_eth_control,
           zynq_eth_can_send,
           zynq_eth_send,
           zynq_eth_recv,
           zynq_eth_deliver,
           zynq_eth_poll,
           zynq_eth_int_vector);


NETDEVTAB_ENTRY(zynq_netdev0,
                "zynq_eth0",
                zynq_eth_init,
                &zynq_eth0_sc);
#endif //CYGPKG_DEVS_ETH_ARM_XC7Z_ETH0

/******************************************************************************
 *
 * Ethernet 1
 *
 ******************************************************************************/
#ifdef CYGPKG_DEVS_ETH_ARM_XC7Z_ETH1

static void zynq_write_phy1(int reg_addr, int phy_addr, unsigned short data)
{
    if( reg_addr < 32)
        {
            zynq_eth_mdio_write(XEMACPS1_BASE, phy_addr, reg_addr, data);
        }
    else
        {
            zynq_eth_mdio_write(XEMACPS1_BASE, phy_addr, 0x0B, reg_addr | (1<<15) );
            zynq_eth_mdio_write(XEMACPS1_BASE, phy_addr, 0x0C, data);
        }
}


static bool zynq_read_phy1(int reg_addr, int phy_addr, unsigned short *data)
{
    if( reg_addr < 32 )
        {
            *data = zynq_eth_mdio_read(XEMACPS1_BASE, phy_addr, reg_addr) & 0xFFFF;
        }
    else
        {
            zynq_eth_mdio_write(XEMACPS1_BASE, phy_addr, 0x0B, reg_addr);
            *data = zynq_eth_mdio_read(XEMACPS1_BASE, phy_addr, 0x0D) & 0xFFFF;
        }
    
    return true;
}

static void zynq_init_phy1(void)
{ }

static void zynq_reset_phy1(void) 
{ }

#ifdef CYGPKG_DEVS_ETH_PHY
ETH_PHY_REG_LEVEL_ACCESS_FUNS(zynq_phy1,
                              zynq_init_phy1,
                              zynq_reset_phy1,
                              zynq_write_phy1,
                              zynq_read_phy1);
#endif
zynq_eth_t eth1_data = {
#ifdef CYGPKG_DEVS_ETH_PHY
    .phy = &zynq_phy1,
#endif //CYGPKG_DEVS_ETH_PHY
#ifdef CYGPKG_DEVS_ETH_ARM_XC7Z_MAX_LINK_SPEED_ETH1
    .max_link_speed = CYGPKG_DEVS_ETH_ARM_XC7Z_MAX_LINK_SPEED_ETH1,
#else
    .max_link_speed = 0,
#endif
    .base_addr = XEMACPS1_BASE,
    .irq_no = CYGNUM_HAL_INTERRUPT_GEM1,
    .MAC_source = CYGPKG_DEVS_ETH_ARM_XC7Z_MAC_SOURCE_ETH1,
    .MAC_addr_cdl = CYGPKG_DEVS_ETH_ARM_XC7Z_MAC_ADDR_ETH1,
    .tx_buf_count = CYGPKG_DEVS_ETH_ARM_XC7Z_TXBUF_COUNT,
    .rx_buf_count = CYGPKG_DEVS_ETH_ARM_XC7Z_RXBUF_COUNT,
    .packet_len = CYGPKG_DEVS_ETH_ARM_XC7Z_PACKETLEN
};


ETH_DRV_SC(zynq_eth1_sc,
           (void*) &eth1_data,
           "eth1",
           zynq_eth_start,
           zynq_eth_stop,
           zynq_eth_control,
           zynq_eth_can_send,
           zynq_eth_send,
           zynq_eth_recv,
           zynq_eth_deliver,
           zynq_eth_poll,
           zynq_eth_int_vector);


NETDEVTAB_ENTRY(zynq_netdev1,
                "zynq_eth1",
                zynq_eth_init,
                &zynq_eth1_sc);
#endif //CYGPKG_DEVS_ETH_ARM_XC7Z_ETH0

/******************************************************************************
 *
 * Functions
 *
 ******************************************************************************/

/*
 * zynq_eth_write - Write single memory location
 */
static inline void zynq_eth_write(cyg_uint32 base, cyg_uint32 reg, cyg_uint32 val)
{
    HAL_WRITE_UINT32(base + reg, val);
}


/*
 * zynq_eth_read - Read single memory location
 */
static inline cyg_uint32 zynq_eth_read(cyg_uint32 base, cyg_uint32 reg)
{
    cyg_uint32 read_val;
    HAL_READ_UINT32(base + reg, read_val);

    return read_val;
}


/*
 * zynq_eth_mdio_read - Read current value of phy register indicated by
 * phyreg.
 */
static int zynq_eth_mdio_read(cyg_uint32 base_addr, int mii_id, int phyreg)
{
    cyg_uint32 regval;
    int value;
    volatile cyg_uint32 ipisr;

    regval  = XEMACPS_PHYMNTNC_OP_MASK;
    regval |= XEMACPS_PHYMNTNC_OP_R_MASK;
    regval |= (mii_id << XEMACPS_PHYMNTNC_PHYAD_SHIFT_MASK);
    regval |= (phyreg << XEMACPS_PHYMNTNC_PHREG_SHIFT_MASK);
    zynq_eth_write(base_addr, XEMACPS_PHYMNTNC_OFFSET, regval);

    // wait for end of transfer
    do
        {
            cpu_relax();
            ipisr = zynq_eth_read(base_addr, XEMACPS_NWSR_OFFSET);
        } while ((ipisr & XEMACPS_NWSR_MDIOIDLE_MASK) == 0);

    value = zynq_eth_read(base_addr, XEMACPS_PHYMNTNC_OFFSET) & XEMACPS_PHYMNTNC_DATA_MASK;

    return value;
}


/*
 *  zynq_eth_mdio_write - Write passed in value to phy register indicated
 *  by phyreg.
 */
static void zynq_eth_mdio_write(cyg_uint32 base_addr, int mii_id, int phyreg, cyg_uint16 value)
{
    cyg_uint32 regval;
    volatile cyg_uint32 ipisr;

    regval  = XEMACPS_PHYMNTNC_OP_MASK;
    regval |= XEMACPS_PHYMNTNC_OP_W_MASK;
    regval |= (mii_id << XEMACPS_PHYMNTNC_PHYAD_SHIFT_MASK);
    regval |= (phyreg << XEMACPS_PHYMNTNC_PHREG_SHIFT_MASK);
    regval |= value;

    zynq_eth_write(base_addr, XEMACPS_PHYMNTNC_OFFSET, regval);

    // wait for end of transfer
    do
        {
            cpu_relax();
            ipisr = zynq_eth_read(base_addr, XEMACPS_NWSR_OFFSET);
        } while ((ipisr & XEMACPS_NWSR_MDIOIDLE_MASK) == 0);
}


static void zynq_eth_set_link_speed(zynq_eth_t *data, cyg_uint16 link_speed)
{
    cyg_uint32 regval32;

    data->link_speed = link_speed;

    //MAC setup
    regval32 = zynq_eth_read(data->base_addr, XEMACPS_NWCFG_OFFSET);
    if (link_speed == 10)
        regval32 &= ~(0x1);    // enable 10Mbps
    else
        regval32 |= 0x1;       // enable 100Mbps
	
    if (link_speed == 1000)
        regval32 |= 0x400;     // enable 1000Mbps
    else
        regval32 &= ~(0x400);  // disable gigabit

    zynq_eth_write(data->base_addr, XEMACPS_NWCFG_OFFSET, regval32);

    //slcr unlock
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCR_UNLOCK_OFFSET, XSLCR_UNLOCK_KEY);
    
    if(data->base_addr == XEMACPS0_BASE)
        {
            //GEM0_RCLK_CTRL configuration (Enable Rx Clock, MIO Rx Clock source)
            HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRGEM0_RCLK_CTRL_OFFSET, (XSLCRGEM_RCLK_CTRL_CLKACT_EN));
            //setting divisor
            regval32 = 0;
            if (link_speed == 1000)
                {
                    // 125MHz
                    regval32 = (1 << XSLCRGEM_CLK_CTRL_DIVISOR1_BITPOS);
                    regval32 |= (8 << XSLCRGEM_CLK_CTRL_DIVISOR_BITPOS);
                }
            else if (link_speed == 100)
                {
                    // 25 MHz
                    regval32 = (1 << XSLCRGEM_CLK_CTRL_DIVISOR1_BITPOS);
                    regval32 |= (40 << XSLCRGEM_CLK_CTRL_DIVISOR_BITPOS);
                }
            else
                {
                    // 2.5 MHz
                    regval32 = (10 << XSLCRGEM_CLK_CTRL_DIVISOR1_BITPOS);
                    regval32 |= (40 << XSLCRGEM_CLK_CTRL_DIVISOR_BITPOS);
                }
            regval32 |= XSLCRGEM_CLK_CTRL_SRCSEL_IOPLL;
            regval32 |= XSLCRGEM_CLK_CTRL_CLKACT_EN;
            HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRGEM0_CLK_CTRL_OFFSET, regval32);
        }

    if(data->base_addr == XEMACPS1_BASE)
        {
            //GEM1_RCLK_CTRL configuration (Enable Rx Clock, MIO Rx Clock source)
            HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRGEM1_RCLK_CTRL_OFFSET, (XSLCRGEM_CLK_CTRL_CLKACT_EN));
            //setting divisor
            regval32 = 0;
            if (link_speed == 1000)
                {
                    // 125MHz
                    regval32 = (1 << XSLCRGEM_CLK_CTRL_DIVISOR1_BITPOS);
                    regval32 |= (8 << XSLCRGEM_CLK_CTRL_DIVISOR_BITPOS);
                }
            else if (link_speed == 100)
                {
                    // 25 MHz
                    regval32 = (1 << XSLCRGEM_CLK_CTRL_DIVISOR1_BITPOS);
                    regval32 |= (40 << XSLCRGEM_CLK_CTRL_DIVISOR_BITPOS);
                }
            else
                {
                    // 2.5 MHz
                    regval32 = (10 << XSLCRGEM_CLK_CTRL_DIVISOR1_BITPOS);
                    regval32 |= (40 << XSLCRGEM_CLK_CTRL_DIVISOR_BITPOS);
                }
            regval32 |= XSLCRGEM_CLK_CTRL_SRCSEL_IOPLL;
            regval32 |= XSLCRGEM_CLK_CTRL_CLKACT_EN;
            HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRGEM1_CLK_CTRL_OFFSET, regval32);
        }
    // slcr lock 
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCR_LOCK_OFFSET, XSLCR_LOCK_KEY);
}


/**
 *  zynq_eth_setup_ring - Setup both TX and RX BD rings
 *  @lp: local device instance pointer
 *  return 0 on success, negative value if error
 **/
static void zynq_eth_setup_ring(zynq_eth_t *dev_data)
{
    int i;
    zynq_eth_bd *bdptr; //current bd
    cyg_uint32 cache_en;

    HAL_DCACHE_IS_ENABLED(cache_en);

    dev_data->rx_ring.separation   = (sizeof(zynq_eth_bd) + (ALIGNMENT_BD - 1)) & ~(ALIGNMENT_BD - 1);
    dev_data->rx_ring.firstbdaddr  = (cyg_uint32) dev_data->rx_bd;
    dev_data->rx_ring.lastbdaddr   = (cyg_uint32)(dev_data->rx_bd) + (dev_data->rx_buf_count - 1) * sizeof(zynq_eth_bd);
    dev_data->rx_ring.length       = dev_data->rx_ring.lastbdaddr - dev_data->rx_ring.firstbdaddr + dev_data->rx_ring.separation;
    dev_data->rx_ring.freehead     = (zynq_eth_bd *)dev_data->rx_bd;
    dev_data->rx_ring.prehead      = (zynq_eth_bd *)dev_data->rx_bd;
    dev_data->rx_ring.hwhead       = (zynq_eth_bd *)dev_data->rx_bd;
    dev_data->rx_ring.hwtail       = (zynq_eth_bd *)dev_data->rx_bd;
    dev_data->rx_ring.posthead     = (zynq_eth_bd *)dev_data->rx_bd;
    dev_data->rx_ring.allcnt       = dev_data->rx_buf_count;
    dev_data->rx_ring.freecnt      = dev_data->rx_buf_count;
    dev_data->rx_ring.precnt       = 0;
    dev_data->rx_ring.hwcnt        = 0;
    dev_data->rx_ring.postcnt      = 0;
    dev_data->rx_ring.is_rx        = 1;

    bdptr = dev_data->rx_bd;
    /* Setup RX BD ring structure and populate buffer address. */
    for (i = 0; i < (dev_data->rx_buf_count-1); i++)
        {
            zynq_eth_write((cyg_uint32)bdptr, XEMACPS_BD_STAT_OFFSET, 0);
            zynq_eth_write((cyg_uint32)bdptr, XEMACPS_BD_ADDR_OFFSET, (cyg_uint32)dev_data->rx_buffer[i]);
            if(cache_en) 
                HAL_DCACHE_FLUSH(bdptr, sizeof(zynq_eth_bd));
            bdptr = XEMACPS_BDRING_NEXT(&dev_data->rx_ring, bdptr);
        }
	
    /* wrap bit set for last BD, bdptr is moved to last here */
    zynq_eth_write((cyg_uint32)bdptr, XEMACPS_BD_STAT_OFFSET, 0);
    zynq_eth_write((cyg_uint32)bdptr, XEMACPS_BD_ADDR_OFFSET, (cyg_uint32)dev_data->rx_buffer[i] | XEMACPS_RXBUF_WRAP_MASK);
    if(cache_en) 
        HAL_DCACHE_FLUSH(bdptr, sizeof(zynq_eth_bd));
	
    dev_data->tx_ring.separation   = (sizeof(zynq_eth_bd) + (ALIGNMENT_BD - 1)) & ~(ALIGNMENT_BD - 1);
    dev_data->tx_ring.firstbdaddr  = (cyg_uint32)dev_data->tx_bd;
    dev_data->tx_ring.lastbdaddr   = (cyg_uint32)(dev_data->tx_bd) + (dev_data->tx_buf_count - 1) * sizeof(zynq_eth_bd);
    dev_data->tx_ring.length       = dev_data->tx_ring.lastbdaddr - dev_data->tx_ring.firstbdaddr + dev_data->tx_ring.separation;
    dev_data->tx_ring.freehead     = (zynq_eth_bd *)dev_data->tx_bd;
    dev_data->tx_ring.prehead      = (zynq_eth_bd *)dev_data->tx_bd;
    dev_data->tx_ring.hwhead       = (zynq_eth_bd *)dev_data->tx_bd;
    dev_data->tx_ring.hwtail       = (zynq_eth_bd *)dev_data->tx_bd;
    dev_data->tx_ring.posthead     = (zynq_eth_bd *)dev_data->tx_bd;
    dev_data->tx_ring.allcnt       = dev_data->tx_buf_count;
    dev_data->tx_ring.freecnt      = dev_data->tx_buf_count;
    dev_data->tx_ring.precnt       = 0;
    dev_data->tx_ring.hwcnt        = 0;
    dev_data->tx_ring.postcnt      = 0;
    dev_data->tx_ring.is_rx        = 0;

    bdptr = (struct zynq_eth_bd *)dev_data->tx_ring.firstbdaddr;

    // Setup TX BD ring structure and assert used bit initially.
    for (i = 0; i < dev_data->tx_buf_count; i++)
        {
            zynq_eth_write((cyg_uint32)bdptr, XEMACPS_BD_ADDR_OFFSET, (cyg_uint32)dev_data->tx_buffer[i]);
            if (i != (dev_data->tx_buf_count - 1))
                {
                    zynq_eth_write((cyg_uint32)bdptr, XEMACPS_BD_STAT_OFFSET, XEMACPS_TXBUF_USED_MASK);
                }
            else
                {
                    // this is the last buf
                    zynq_eth_write((cyg_uint32)bdptr, XEMACPS_BD_STAT_OFFSET, XEMACPS_TXBUF_WRAP_MASK | XEMACPS_TXBUF_USED_MASK);
                }
            if(cache_en) 
                HAL_DCACHE_FLUSH(bdptr, sizeof(zynq_eth_bd));
            bdptr = XEMACPS_BDRING_NEXT(&dev_data->tx_ring, bdptr);
        }
}


/**
 *  zynq_eth_get_hwaddr - Set device's MAC address
 *  @dev_data: driver private data pointer
 **/
static void zynq_eth_get_hwaddr(zynq_eth_t *dev_data)
{
    cyg_uint32 regvall;
    cyg_uint32 regvalh;

    regvall = zynq_eth_read(dev_data->base_addr, XEMACPS_LADDR1L_OFFSET);
    regvalh = zynq_eth_read(dev_data->base_addr, XEMACPS_LADDR1H_OFFSET);

    dev_data->MAC_addr[0] = (regvall & 0xff);
    dev_data->MAC_addr[1] = ((regvall >> 8) & 0xff);
    dev_data->MAC_addr[2] = ((regvall >> 16) & 0xff);
    dev_data->MAC_addr[3] = (regvall >> 24);
    dev_data->MAC_addr[4] = (regvalh & 0xff);
    dev_data->MAC_addr[5] = (regvalh >> 8);
}


/**
 *  zynq_eth_reset_hw - Helper function to reset the underlying hardware.
 *  This is called when we get into such deep trouble that we don't know
 *  how to handle otherwise.
 */
static void zynq_eth_reset_hw(zynq_eth_t *dev_data)
{
    cyg_uint32 regisr;

    // Have a clean start
    zynq_eth_write(dev_data->base_addr, XEMACPS_NWCTRL_OFFSET, 0);

    // Clear statistic counters
    zynq_eth_write(dev_data->base_addr, XEMACPS_NWCTRL_OFFSET, XEMACPS_NWCTRL_STATCLR_MASK);

    // Clear TX and RX status
    zynq_eth_write(dev_data->base_addr, XEMACPS_TXSR_OFFSET, 0xffffffff);
    zynq_eth_write(dev_data->base_addr, XEMACPS_RXSR_OFFSET, 0xffffffff);

    // Disable all interrupts
    zynq_eth_write(dev_data->base_addr, XEMACPS_IDR_OFFSET, 0xffffffff); 

    //clear all interrupts
    regisr = zynq_eth_read(dev_data->base_addr, XEMACPS_ISR_OFFSET);
    zynq_eth_write(dev_data->base_addr, XEMACPS_ISR_OFFSET, regisr);
}


// ISR routine
static int zynq_eth_isr(cyg_vector_t vector, cyg_addrword_t _data)
{
    struct eth_drv_sc *sc = (struct eth_drv_sc*)_data;
    zynq_eth_t *data = sc->driver_private;
    cyg_uint32 regval;
    int ret;
    
    // mask & ack the interrupt
    cyg_drv_interrupt_mask(data->irq_no);
    cyg_drv_interrupt_acknowledge(data->irq_no);
    
    // read interrupt status register
    regval = zynq_eth_read(data->base_addr, XEMACPS_ISR_OFFSET);
    
    // call DSR only if there is interrupt to handle
    if(regval & 0x00000082)
        {
            ret = (CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);
        }
    else
        {
            // otherwise, clear ISR flags and unmask the interrupt
            zynq_eth_write(data->base_addr, XEMACPS_ISR_OFFSET, regval);
            ret = CYG_ISR_HANDLED;
            cyg_drv_interrupt_unmask(data->irq_no);
        }

    return ret;
}

static bool zynq_eth_phy_init(struct eth_drv_sc *sc)
{
    #ifdef CYGPKG_DEVS_ETH_PHY
    zynq_eth_t *data = sc->driver_private;
    unsigned short phy_reg;	    
        
    CYG_ASSERTC(data->phy);
    if (!_eth_phy_init(data->phy))
        {
            // phy init failed
            DEBUG("ERROR: PHY init failed.\n");
            return false;
        }

    int phy_state;
    phy_state = eth_phy_cfg(data->phy, data->max_link_speed);
    if (phy_state & ETH_PHY_STAT_1000MB)
        {
            DEBUG("Link speed 1GB\n");
            data->link_speed = 1000;
        }
    else if (phy_state & ETH_PHY_STAT_100MB)
        {
            DEBUG("Link speed 100MB\n");
            data->link_speed = 100;
        }
    else
        {
            DEBUG("Link speed 10MB\n");
            data->link_speed = 10;
        }
	
    zynq_eth_set_link_speed(data, data->link_speed);
    //TODO: This should be moved to phy layer, as it is Mars_ZX3 specific phy	
    _eth_phy_read(data->phy, 27, data->phy->phy_addr, &phy_reg); //clear interrupt
    return true;
    #else
    DEBUG("ERROR: No phy attached.\n");
    return false; // phy is needed for the driver to work
    #endif
}


static bool zynq_eth_init(struct cyg_netdevtab_entry *tab)
{

    struct eth_drv_sc *sc = tab->device_instance;
    zynq_eth_t *data = sc->driver_private;
    cyg_uint32 regval;
    int i, phy_state;

#ifndef CYGPKG_DEVS_ETH_PHY
    DEBUG("ERROR: No phy attached.\n");
    return false; // phy is needed for the driver to work
#endif

    zynq_eth_reset_hw(data);

    regval = zynq_eth_read(data->base_addr, XEMACPS_MODID_OFFSET);
    
    if ( ((regval & XEMACPS_MODID_ID_MASK) >> XEMACPS_MODID_ID_SHIFT) != XEMACPS_ID )
	{
	    DEBUG("ERROR: Not GEM or device disabled\n");
	    return false;
	}

    if(data->MAC_source == 1)
        {
            zynq_eth_get_hwaddr(data);
        }
    else
        {
            for(i = 0; i < 6; i++) 
                data->MAC_addr[i] = (unsigned char)((data->MAC_addr_cdl >> (5 - i)) & 0xff);
        }
    /*************************** MAC Setup ***************************/
    regval = (7 << 18);	 /* MDC clock division (48 for up to 120MHz) */ // jakis clock btw
    regval |= (1 << 17); /* set for FCS removal */
    regval |= (1 << 10); /* enable gigabit */
    regval |= (1 << 4);	 /* copy all frames */
    regval |= (1 << 1);	 /* enable full duplex */

    zynq_eth_write(data->base_addr, XEMACPS_NWCFG_OFFSET, regval);
    /* MDIO enable */
    regval =  zynq_eth_read(data->base_addr, XEMACPS_NWCTRL_OFFSET);
    regval |= XEMACPS_NWCTRL_MDEN_MASK;
    zynq_eth_write(data->base_addr, XEMACPS_NWCTRL_OFFSET, regval);

    zynq_eth_setup_ring(data);

    zynq_eth_write(data->base_addr, XEMACPS_RXQBASE_OFFSET, (cyg_uint32)&data->rx_bd);
    zynq_eth_write(data->base_addr, XEMACPS_TXQBASE_OFFSET, (cyg_uint32)&data->tx_bd);
    // DMA Configuration
    // MAX_LEN_FRAME_1536
    regval = (0x18 << XEMACPS_DMACR_RXBUF_SHIFT);
    regval |= XEMACPS_DMACR_BLENGTH_INCR4;
    // RX_PACKET_BUF_SIZE_FULL (0x3 << 8)
    regval |= (3 << 8);
    // TX_PACKET_BUF_SIZE_FULL (0x4 << 8)
    regval |= (4 << 8);
    zynq_eth_write(data->base_addr, XEMACPS_DMACR_OFFSET, regval); // = 0x180704

    if ( !zynq_eth_phy_init(sc) ) //initialize phy
    {
	return false;
    }

    // Attach an interrupt
    cyg_drv_interrupt_create((cyg_vector_t) data->irq_no,
                             (cyg_priority_t)ZYNQ_ETH_INT_PRIO,
                             (cyg_addrword_t) sc,
                             (cyg_ISR_t*)zynq_eth_isr,
                             (cyg_DSR_t*)eth_drv_dsr,
                             (cyg_handle_t*)&(data->int_handle),
                             (cyg_interrupt*)&(data->int_object));
    cyg_drv_interrupt_attach(data->int_handle);
    cyg_drv_interrupt_acknowledge(data->irq_no);
    cyg_drv_interrupt_unmask(data->irq_no);

    // Initialize upper level driver by sending MAC address
    (sc->funs->eth_drv->init)(sc, data->MAC_addr);

    return true;
}


static void zynq_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
    zynq_eth_t *data = sc->driver_private;
    cyg_uint32 regval;

    // Disable all the MAC Interrupts
    zynq_eth_write(data->base_addr, XEMACPS_IDR_OFFSET, 0xFFFFFFFF);

    // Rx and Tx enable
    regval = zynq_eth_read(data->base_addr, XEMACPS_NWCTRL_OFFSET);
    regval |= XEMACPS_NWCTRL_RXEN_MASK | XEMACPS_NWCTRL_TXEN_MASK;
    zynq_eth_write(data->base_addr, XEMACPS_NWCTRL_OFFSET, regval);

    // Enable interrupts 
    regval = XEMACPS_IXR_ALL_MASK;
    zynq_eth_write(data->base_addr, XEMACPS_IER_OFFSET, regval);
}


static void zynq_eth_stop(struct eth_drv_sc *sc)
{

    zynq_eth_t *data = sc->driver_private;
    cyg_uint32 regval;

    // Disable TX, RX and MDIO port
    regval = zynq_eth_read(data->base_addr, XEMACPS_NWCTRL_OFFSET);
    regval &= ~XEMACPS_NWCTRL_MDEN_MASK;
    regval &= ~XEMACPS_NWCTRL_TXEN_MASK;
    regval &= ~XEMACPS_NWCTRL_RXEN_MASK;
    zynq_eth_write(data->base_addr, XEMACPS_NWCTRL_OFFSET, regval);

    // Disable interrupts
    regval = ~XEMACPS_IXR_ALL_MASK;
    zynq_eth_write(data->base_addr, XEMACPS_IER_OFFSET, regval);
}

static int zynq_eth_control(struct eth_drv_sc *sc, unsigned long key, void *data, int len)
{
     //zynq_eth_t *data = sc->driver_private;
    //TODO: write control routines
    /*switch(key)
      {
      case ETH_DRV_SET_MAC_ADDRESS:
      case ETH_DRV_GET_IF_STATS_UD:
      case ETH_DRV_GET_IF_STATS:
      case ETH_DRV_SET_MC_LIST:
      case ETH_DRV_SET_MC_ALL:
      }*/
    return 0;
}


// Function returns number of packet which can be send
static int zynq_eth_can_send(struct eth_drv_sc *sc) 
{
    zynq_eth_t *data = sc->driver_private;	 
    short phy_reg;

    //TODO: This should be moved to phy layer, as it is Mars_ZX3 specific phy	
    _eth_phy_read(data->phy, 27, data->phy->phy_addr, &phy_reg);
    if( phy_reg & 0x01 ) //link up occured
    {	
	if ( !zynq_eth_phy_init(sc) )
	{
		return 0;
	}
    }			
    return data->tx_ring.freecnt;
}


static void zynq_eth_send(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len, int total_len, unsigned long key)
{
    cyg_uint32 regval32;
    zynq_eth_t *data = sc->driver_private;
    zynq_eth_bd *bd_ptr;
    int i, j;
    unsigned int free_bd;
    cyg_uint32 cache_en;

    // Reject too big frames
    if(total_len > data->packet_len)
        {
            // Immediately notify the stack, that transmission failed
            sc->funs->eth_drv->tx_done(sc, key, -1);
            DEBUG("Frame too big\n");
            return;
        }

    cyg_drv_dsr_lock();
    
    regval32 = zynq_eth_read(data->base_addr, XEMACPS_TXQBASE_OFFSET);

    bd_ptr = regval32; // TODO: hacky way
    data->tx_ring.hwhead = bd_ptr;
    free_bd = XEMACPS_BD_TO_INDEX(&data->tx_ring, bd_ptr);

    // Upper stack gives us a scatter-gather list. We need to assemble that into one
    // single packet. We store the packet directly in the transmission buffer.
    j = 0;
    for(i = 0; i < sg_len; i++)
        {
            memcpy(&data->tx_buffer[free_bd][j], (char*)sg_list[i].buf, sg_list[i].len);
            j += sg_list[i].len;
        }
	
    HAL_DCACHE_IS_ENABLED(cache_en);
    if(cache_en) 
        HAL_DCACHE_FLUSH(data->tx_buffer[free_bd], total_len);
	
    // Store key (handler) so it will be possible to notify the stack later
    data->tx_key[free_bd] = key;
    
    // store buffer address in BD 
    regval32 = zynq_eth_read((cyg_uint32)bd_ptr, XEMACPS_BD_STAT_OFFSET);
    // Preserve only critical status bits.  Packet is NOT to be committed to hardware at this time.
    regval32 &= (XEMACPS_TXBUF_USED_MASK | XEMACPS_TXBUF_WRAP_MASK);
    // update length field
    regval32 |= ((regval32 & ~XEMACPS_TXBUF_LEN_MASK) | total_len);
    // update last bd bit
    regval32 |= XEMACPS_TXBUF_LAST_MASK;

    last_added_bd = bd_ptr;
    data->tx_ring.freecnt--;

    zynq_eth_write((cyg_uint32)bd_ptr, XEMACPS_BD_STAT_OFFSET, regval32);

    // give it back to hardware
    regval32 = zynq_eth_read((cyg_uint32)bd_ptr, XEMACPS_BD_STAT_OFFSET);
    regval32 &= ~XEMACPS_TXBUF_USED_MASK;
    zynq_eth_write((cyg_uint32)bd_ptr, XEMACPS_BD_STAT_OFFSET, regval32);
	
    if(cache_en) 
        HAL_DCACHE_FLUSH(bd_ptr, sizeof(zynq_eth_bd));
	
    //write send bit to ctrl register
    regval32 = zynq_eth_read(data->base_addr, XEMACPS_NWCTRL_OFFSET);
    zynq_eth_write(data->base_addr, XEMACPS_NWCTRL_OFFSET, (regval32 |XEMACPS_NWCTRL_STARTTX_MASK));

    cyg_drv_dsr_unlock();
}


static void zynq_eth_recv(struct eth_drv_sc *sc, struct eth_drv_sg *sg_list, int sg_len)
{
    // This function is called by the upper layer as a result of calling
    // _recv callback. We need to move data from reception buffer
    // to given scatter-gather list.
    zynq_eth_t *data = sc->driver_private;
    cyg_uint32 cache_en;

    HAL_DCACHE_IS_ENABLED(cache_en);
	
    if(sg_list == NULL)
        {
            DEBUG("sg_list is empty!\n");
            return;
        }

    // Copy sg to one buf
    // TODO: this can be reimplemented so it is done by hw
    int i, j;
    j = 0;
    for(i = 0; i < sg_len; i++)
        {
            if(sg_list[i].buf)
                {
                    memcpy((char*)sg_list[i].buf, data->rx_buffer[data->rx_head] + j, sg_list[i].len);
                    j += sg_list[i].len;
                }
        }

    if(cache_en) 
        HAL_DCACHE_FLUSH(sg_list, sg_len*sizeof(struct eth_drv_sg));
}


static void zynq_eth_deliver(struct eth_drv_sc *sc)
{
    // This function is called from default network DSR provided
    // by common eCos package. Is is called when network
    // interrupt occurs. It simply calls _poll to analyze status
    // registers. 

    cyg_uint32 regval32;
    zynq_eth_t *data = sc->driver_private;

    // clear all interrupts
    regval32 = zynq_eth_read(data->base_addr, XEMACPS_ISR_OFFSET);
    zynq_eth_write(data->base_addr, XEMACPS_ISR_OFFSET, regval32);

    // poll only if sth to do
    if(regval32 & 0x82)
        zynq_eth_poll(sc); 
		
    // Unmask the interrupt
    cyg_drv_interrupt_unmask(data->irq_no);
}


static void zynq_eth_poll(struct eth_drv_sc *sc)
{
    //at this time we got only tx poll routine
    zynq_eth_t *data = sc->driver_private;
    cyg_uint32 regval32,regval_txsr, regval_rxsr;
    cyg_uint32 size;
    zynq_eth_bd *bd_ptr;
    unsigned int bd_to_handle;
    cyg_uint32 cache_en;
	
    HAL_DCACHE_IS_ENABLED(cache_en);

    cyg_drv_dsr_lock();

    // Read the status register to know if any tx packet needs attention
    regval_txsr = zynq_eth_read(data->base_addr, XEMACPS_TXSR_OFFSET);
	
    if(regval_txsr & (XEMACPS_TXSR_TXCOMPL_MASK | XEMACPS_TXSR_HRESPNOK_MASK | XEMACPS_TXSR_URUN_MASK | XEMACPS_TXSR_BUFEXH_MASK))
        {
            //check if any tx BD's are ready to postprocess
            bd_ptr = last_tx_bd == NULL ? data->tx_ring.hwhead : XEMACPS_BDRING_NEXT(&data->tx_ring, last_tx_bd);

            while (1) 
                {	
                    bd_to_handle = XEMACPS_BD_TO_INDEX(&data->tx_ring, bd_ptr);
                    regval32 = zynq_eth_read((cyg_uint32)bd_ptr, XEMACPS_BD_STAT_OFFSET);

                    // packet empty?
                    if ((regval32 & 0x1FFF) == 0)
                        {
                            if (bd_ptr == last_added_bd)
                                break;
                            bd_ptr = XEMACPS_BDRING_NEXT(&data->tx_ring, bd_ptr);
                            continue;
                        } 
                    last_tx_bd = bd_ptr;

                    if (data->tx_key[bd_to_handle] != 0)
                        {
                            sc->funs->eth_drv->tx_done(sc, data->tx_key[bd_to_handle], 0); //inform upper layer driver
                        } 

                    regval32 &= (XEMACPS_TXBUF_USED_MASK | XEMACPS_TXBUF_WRAP_MASK);
                    zynq_eth_write((cyg_uint32)bd_ptr, XEMACPS_BD_STAT_OFFSET, regval32);
                    data->tx_ring.freecnt++;

                    if (bd_ptr == last_added_bd)
                        break;
                    bd_ptr = XEMACPS_BDRING_NEXT(&data->tx_ring, bd_ptr);
                }

            //clean Tx status reg
            zynq_eth_write(data->base_addr, XEMACPS_TXSR_OFFSET, 0xFFFFFFFF);
        }

    // Read the status register to know if any rx packet needs attention
    regval_rxsr = zynq_eth_read(data->base_addr, XEMACPS_RXSR_OFFSET);	
    if(regval_rxsr & XEMACPS_RXSR_FRAMERX_MASK)
        {
            bd_ptr = data->rx_ring.freehead;	
            while(1)
                {
                    //receive one or more packets
                    if ( !(bd_ptr->addr & XEMACPS_RXBUF_NEW_MASK) )
                        {	
                            break;
                        }
                    else if ( !(bd_ptr->ctrl & XEMACPS_RXBUF_EOF_MASK) )
                        {
                            break;
                        }
                    else if ( !(bd_ptr->ctrl & XEMACPS_RXBUF_SOF_MASK) )
                        {
                            break;
                        }
                    else if ( (bd_ptr->ctrl & XEMACPS_RXBUF_LEN_MASK) == 0)
                        {
                            break;
                        }
                    else 
                        {
                            size = (bd_ptr->ctrl & XEMACPS_RXBUF_LEN_MASK);
                            data->rx_head = XEMACPS_BD_TO_INDEX(&data->rx_ring, bd_ptr);
                            sc->funs->eth_drv->recv(sc, size);
                            bd_ptr->ctrl = 0x00000000;
                            bd_ptr->addr &= ~(XEMACPS_RXBUF_NEW_MASK);
                            if(cache_en) 
                                HAL_DCACHE_FLUSH(bd_ptr, sizeof(zynq_eth_bd));
                            bd_ptr = XEMACPS_BDRING_NEXT(&data->rx_ring, bd_ptr);
                            data->rx_ring.freehead = bd_ptr;
                        }
                }
            //clear RX status reg
            zynq_eth_write(data->base_addr, XEMACPS_RXSR_OFFSET, regval_rxsr);
        }
    cyg_drv_dsr_unlock();
}


static int zynq_eth_int_vector(struct eth_drv_sc *sc)
{
    zynq_eth_t *data = sc->driver_private;
    return data->irq_no;
}

