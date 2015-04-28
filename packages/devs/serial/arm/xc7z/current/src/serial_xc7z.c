//==========================================================================
//
//      serial_xc7z.c
//
//      Xilinx Zynq Cadence UART driver
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Free Software Foundation, Inc.
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
// Author(s):    Ant Micro <www.antmicro.com>
// Contributors: 
// Date:         2012-08-15
// Purpose:      Xilinx Zynq Cadence UART driver
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================


//==========================================================================
//                               INCLUDES
//==========================================================================

#include <pkgconf/system.h>
#include <pkgconf/io.h>
#include <pkgconf/io_serial.h>
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#include <cyg/io/serial.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_diag.h>


#ifdef CYGPKG_IO_SERIAL_ARM_XC7Z

#include "serial_xc7z.h"

typedef struct zynq_serial_info
{
    CYG_ADDRWORD   base;
    CYG_WORD       int_num;
    cyg_interrupt  serial_interrupt;
    cyg_handle_t   serial_interrupt_handle;
} zynq_serial_info;

static cyg_uint32    zynq_serial_ISR(cyg_vector_t vector, cyg_addrword_t data);
static void          zynq_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data);
static bool          zynq_serial_init(struct cyg_devtab_entry *tab);
static Cyg_ErrNo     zynq_serial_lookup(struct cyg_devtab_entry **tab, struct cyg_devtab_entry *sub_tab, const char *name);
static bool          zynq_serial_putc(serial_channel *chan, unsigned char c);
static unsigned char zynq_serial_getc(serial_channel *chan);
static Cyg_ErrNo     zynq_serial_set_config(serial_channel *chan, cyg_uint32 key, const void *xbuf, cyg_uint32 *len);
static void          zynq_serial_start_xmit(serial_channel *chan);
static void          zynq_serial_stop_xmit(serial_channel *chan);

static SERIAL_FUNS(zynq_serial_funs,
                   zynq_serial_putc,
                   zynq_serial_getc,
                   zynq_serial_set_config,
                   zynq_serial_start_xmit,
                   zynq_serial_stop_xmit
                  );

#ifdef CYGPKG_IO_SERIAL_ARM_XC7Z_SERIAL0

static zynq_serial_info zynq_serial_info0 = 
{ 
    XUARTPS0_BASE,
    CYGNUM_HAL_INTERRUPT_UART0,
};

#if CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL0_BUFSIZE > 0
static unsigned char zynq_serial_out_buf0[CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL0_BUFSIZE];
static unsigned char zynq_serial_in_buf0[CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL0_BUFSIZE];
static SERIAL_CHANNEL_USING_INTERRUPTS(zynq_serial_channel0,
                                       zynq_serial_funs, 
                                       zynq_serial_info0,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL0_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &zynq_serial_out_buf0[0], 
                                       sizeof(zynq_serial_out_buf0),
                                       &zynq_serial_in_buf0[0], 
                                       sizeof(zynq_serial_in_buf0)
    );
#else //CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL0_BUFSIZE == 0
static SERIAL_CHANNEL(zynq_serial_channel0,
                      zynq_serial_funs, 
                      zynq_serial_info0,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL0_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif //CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL0_BUFSIZE

//Create device driver entry
DEVTAB_ENTRY(zynq_serial_io0, 
             CYGDAT_IO_SERIAL_ARM_XC7Z_SERIAL0_NAME,
             0,
             &cyg_io_serial_devio, 
             zynq_serial_init, 
             zynq_serial_lookup,
             &zynq_serial_channel0
    );

#define XSLCRAPER_CLK_CTRL_UART0_DIS        0x00000000
#define XSLCRUART_CLK_CTRL_CLKACT0_DIS      0x00000000

#else //CYGPKG_IO_SERIAL_ARM_XC7Z_SERIAL0

#undef  XSLCRAPER_CLK_CTRL_UART0_EN
#define XSLCRAPER_CLK_CTRL_UART0_EN         0x00000000
#define XSLCRAPER_CLK_CTRL_UART0_DIS        0x00100000

#undef  XSLCRUART_CLK_CTRL_CLKACT0_EN
#define XSLCRUART_CLK_CTRL_CLKACT0_EN       0x00000000
#define XSLCRUART_CLK_CTRL_CLKACT0_DIS      0x00000001

#undef  XSLCRUART_RST_CTRL_UART0_REF_RST
#define XSLCRUART_RST_CTRL_UART0_REF_RST    0x00000000
#undef  XSLCRUART_RST_CTRL_UART0_CPU1X_RST
#define XSLCRUART_RST_CTRL_UART0_CPU1X_RST  0x00000000

#endif //CYGPKG_IO_SERIAL_ARM_XC7Z_SERIAL0


#ifdef CYGPKG_IO_SERIAL_ARM_XC7Z_SERIAL1

static zynq_serial_info zynq_serial_info1 = 
{ 
    XUARTPS1_BASE,
    CYGNUM_HAL_INTERRUPT_UART1,
};

#if CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL1_BUFSIZE > 0
static unsigned char zynq_serial_out_buf1[CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL1_BUFSIZE];
static unsigned char zynq_serial_in_buf1[CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL1_BUFSIZE];

static SERIAL_CHANNEL_USING_INTERRUPTS(zynq_serial_channel1,
                                       zynq_serial_funs, 
                                       zynq_serial_info1,
                                       CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL1_BAUD),
                                       CYG_SERIAL_STOP_DEFAULT,
                                       CYG_SERIAL_PARITY_DEFAULT,
                                       CYG_SERIAL_WORD_LENGTH_DEFAULT,
                                       CYG_SERIAL_FLAGS_DEFAULT,
                                       &zynq_serial_out_buf1[0], 
                                       sizeof(zynq_serial_out_buf1),
                                       &zynq_serial_in_buf1[0], 
                                       sizeof(zynq_serial_in_buf1)
    );
#else //CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL1_BUFSIZE == 0
static SERIAL_CHANNEL(zynq_serial_channel1,
                      zynq_serial_funs, 
                      zynq_serial_info1,
                      CYG_SERIAL_BAUD_RATE(CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL1_BAUD),
                      CYG_SERIAL_STOP_DEFAULT,
                      CYG_SERIAL_PARITY_DEFAULT,
                      CYG_SERIAL_WORD_LENGTH_DEFAULT,
                      CYG_SERIAL_FLAGS_DEFAULT
    );
#endif //CYGNUM_IO_SERIAL_ARM_XC7Z_SERIAL1_BUFSIZE

//Create device driver entry
DEVTAB_ENTRY(zynq_serial_io1, 
             CYGDAT_IO_SERIAL_ARM_XC7Z_SERIAL1_NAME,
             0,
             &cyg_io_serial_devio, 
             zynq_serial_init, 
             zynq_serial_lookup, 
             &zynq_serial_channel1
    );
    
#define XSLCRAPER_CLK_CTRL_UART1_DIS        0x00000000
#define XSLCRUART_CLK_CTRL_CLKACT1_DIS      0x00000000

#else //CYGPKG_IO_SERIAL_ARM_XC7Z_SERIAL1

#undef  XSLCRAPER_CLK_CTRL_UART1_EN
#define XSLCRAPER_CLK_CTRL_UART1_EN         0x00000000
#define XSLCRAPER_CLK_CTRL_UART1_DIS        0x00200000

#undef  XSLCRUART_CLK_CTRL_CLKACT1_EN
#define XSLCRUART_CLK_CTRL_CLKACT1_EN       0x00000000
#define XSLCRUART_CLK_CTRL_CLKACT1_DIS      0x00000002

#undef  XSLCRUART_RST_CTRL_UART1_REF_RST
#define XSLCRUART_RST_CTRL_UART1_REF_RST    0x00000000
#undef  XSLCRUART_RST_CTRL_UART1_CPU1X_RST
#define XSLCRUART_RST_CTRL_UART1_CPU1X_RST  0x00000000

#endif //CYGPKG_IO_SERIAL_ARM_XC7Z_SERIAL1

static cyg_int32 select_baud[] = {
        0,      // Unused
        50,     // 50
        75,     // 75
        110,    // 110
        0,      // Unused: 134.5
        150,    // 150
        200,    // 200
        300,    // 300
        600,    // 600
        1200,   // 1200
        1800,   // 1800
        2400,   // 2400
        3600,   // 3600
        4800,   // 4800
        7200,   // 7200
        9600,   // 9600
        14400,  // 14400
        19200,  // 19200
        38400,  // 38400
        57600,  // 57600
        115200, // 115200
        230400, // 230400
};

// Configure the hardware
static bool zynq_serial_config_port(serial_channel *chan, cyg_serial_info_t *new_config, bool init)
{
	zynq_serial_info *zynq_chan = (zynq_serial_info *)chan->dev_priv;
    cyg_uint16 baud_div, baud_gen;
    cyg_uint32 calc_baud = 0;
    cyg_uint16 calc_baud_error;
    cyg_uint32 new_baud;

    // get new baud rate value
    new_baud = select_baud[new_config->baud];
    
    if(new_baud == 0)
        return false;

    // Calculate baud div & gen values
    for (baud_div = 4; baud_div < 255; baud_div++)
    {
        baud_gen = UART_REF_CLK / (new_baud * (baud_div + 1));
        
        if (baud_gen < 2 || baud_gen > 65535)
            continue;

        calc_baud = UART_REF_CLK / (baud_gen * (baud_div + 1));

        // calculate baud error
        if (new_baud > calc_baud)
            calc_baud_error = new_baud - calc_baud;
        else
            calc_baud_error = calc_baud - new_baud;

        // get first baud with error < 3%
        if ( ((calc_baud_error * 100) / new_baud) < 3 )
            break;
    }
    // set config of baud rate generator into controler
	HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_BAUD_RATE_DIVIDER, baud_div);
	HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_BAUD_RATE_GEN, baud_gen);

	cyg_uint32 mode = 0;
	switch (new_config->word_length) 
	{
		case CYGNUM_SERIAL_WORD_LENGTH_6:
			mode |= (0x3 << 1);
			break;
		case CYGNUM_SERIAL_WORD_LENGTH_7:	
			mode |= (0x2 << 1);
			break;
		case CYGNUM_SERIAL_WORD_LENGTH_8:
			mode |= (0x0 << 1);
			break;
    	default:
    	    return false;
	}

	switch (new_config->parity)
	{
		case CYGNUM_SERIAL_PARITY_EVEN:
			mode |= (0x0 << 3);
			break;
		case CYGNUM_SERIAL_PARITY_ODD:
			mode |= (0x1 << 3);
			break;
		case CYGNUM_SERIAL_PARITY_NONE:
		default:
			mode |= (0x4 << 3);
			break;
	}

	switch (new_config->stop)
	{
		case CYGNUM_SERIAL_STOP_1:
			mode |= (0x0 << 6);
			break;
		case CYGNUM_SERIAL_STOP_1_5:
			mode |= (0x1 << 6);
			break;
		case CYGNUM_SERIAL_STOP_2:
		default:
			mode |= (0x2 << 6);
			break;
	}
	// 0x20 --> 8bits, no parity, 1 stop bit
	HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_MODE, mode); 

	// enable uart
	HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_CONTROL, XUARTPS_CONTROL_TXEN | XUARTPS_CONTROL_RXEN); 

   	// save the configuration
	if (new_config != &chan->config)
	{
	        chan->config = *new_config;
	}

    // success
    return true;
}

static Cyg_ErrNo zynq_serial_set_config(serial_channel *chan, cyg_uint32 key, const void *xbuf, cyg_uint32 *len)
{
	zynq_serial_info *zynq_chan = (zynq_serial_info *)chan->dev_priv;

    switch (key) 
    {
        case CYG_IO_SET_CONFIG_SERIAL_INFO:
        {
            //check for device config
            cyg_serial_info_t *config = (cyg_serial_info_t *)xbuf;
            if ( *len < sizeof(cyg_serial_info_t) )
                return -EINVAL;
            
            //check for port config
            *len = sizeof(cyg_serial_info_t);
            if ( zynq_serial_config_port(chan, config, false) != true)
                return -EINVAL;
            break;
        }
        default:
            return -EINVAL;
    }
    //if initialization passed succesfull
    return ENOERR;
}

//Initializing the device
static bool zynq_serial_init(struct cyg_devtab_entry *tab)
{
    serial_channel *chan = (serial_channel *)tab->priv;
    zynq_serial_info *zynq_chan = (zynq_serial_info *)chan->dev_priv;

    cyg_uint32 pll_clock, calc_uart_clk;
    cyg_uint32 slcr_reg, control_reg, pll_fdiv;
    cyg_uint16 uart_divisor, best_uart_divisor;
    cyg_int32 act_error, prev_error;

    // Calculate DIVISOR value for UART_REF_CLK
    // Read I/O PLL configuration
    HAL_READ_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRIO_PLL_CTRL_OFFSET, pll_fdiv);
    // extract value of PLL divisor
    pll_fdiv = pll_fdiv >> 12;
    pll_fdiv = pll_fdiv & 0x3f;
    // Calculate I/O PLL freq on given PS_CLK
    pll_clock = PS_CLK * pll_fdiv;
    // Calculate best DIVISOR value to obtain UART_REF_CLK on given I/O PLL clock
    prev_error = UART_REF_CLK;
    for(uart_divisor = 1; uart_divisor < 64; uart_divisor++)
    {
        calc_uart_clk = pll_clock / uart_divisor;
        if(calc_uart_clk > UART_REF_CLK)        
            act_error = calc_uart_clk - UART_REF_CLK;
        else
            act_error = UART_REF_CLK - calc_uart_clk;

        if(act_error < prev_error)
        {
            best_uart_divisor = uart_divisor;
            prev_error = act_error;
        }
    }
    best_uart_divisor &= 0x3f;

    // unlock SLCR
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCR_UNLOCK_OFFSET, XSLCR_UNLOCK_KEY);

    // read APER_CLK_CTRL
    HAL_READ_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRAPER_CLK_CTRL_OFFSET, slcr_reg);
    // disable AMBA Clock for unused UARTs
    slcr_reg &= ~(XSLCRAPER_CLK_CTRL_UART1_DIS | XSLCRAPER_CLK_CTRL_UART0_DIS);
    // enable AMBA Clock for used UARTs
    slcr_reg |= (XSLCRAPER_CLK_CTRL_UART1_EN | XSLCRAPER_CLK_CTRL_UART0_EN);
    // write APER_CLK_CTRL
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRAPER_CLK_CTRL_OFFSET, slcr_reg);

    // read UART_CLK_CTRL
    HAL_READ_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRUART_CLK_CTRL_OFFSET, slcr_reg);
	// set maximum DIVISOR so the UART_REF_CLK not exceed max value
	slcr_reg |= (0x3f << XSLCRUART_CLK_CTRL_DIVISOR_BITPOS);
	// write divisor to UART_CLK_CTRL
	HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRUART_CLK_CTRL_OFFSET, slcr_reg);

    // read UART_CLK_CTRL
    HAL_READ_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRUART_CLK_CTRL_OFFSET, slcr_reg);
    // Select the PLL source as IO PLL
    slcr_reg &= ~(XSLCRUART_CLK_CTRL_SRCSEL_MASK);
    slcr_reg |= XSLCRUART_CLK_CTRL_SRCSEL_IO_PLL_EN;
	// write config to UART_CLK_CTRL
	HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRUART_CLK_CTRL_OFFSET, slcr_reg);
	
    // read UART_CLK_CTRL
    HAL_READ_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRUART_CLK_CTRL_OFFSET, slcr_reg);
	// clear divisor field
	slcr_reg &= ~(XSLCRUART_CLK_CTRL_DIVISOR_MASK);
	// set DIVISOR
	slcr_reg |= (best_uart_divisor << XSLCRUART_CLK_CTRL_DIVISOR_BITPOS);
    // disable unused UARTs Clock in UART_CLK_CTRL
    slcr_reg &= ~(XSLCRUART_CLK_CTRL_CLKACT0_DIS | XSLCRUART_CLK_CTRL_CLKACT0_DIS);
    // enable used UARTs Clock in UART_CLK_CTRL
    slcr_reg |= (XSLCRUART_CLK_CTRL_CLKACT0_EN | XSLCRUART_CLK_CTRL_CLKACT0_EN);
	// write config to UART_CLK_CTRL
	HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRUART_CLK_CTRL_OFFSET, slcr_reg);
   
    // read UART Software Reset Control register
    HAL_READ_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRUART_RST_CTRL_OFFSET, slcr_reg);
    // deassert used UARTs RST
    slcr_reg &= ~(XSLCRUART_RST_CTRL_UART1_REF_RST | XSLCRUART_RST_CTRL_UART0_REF_RST |
                XSLCRUART_RST_CTRL_UART1_CPU1X_RST | XSLCRUART_RST_CTRL_UART0_CPU1X_RST);
    // write UART Software Reset Control register
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRUART_RST_CTRL_OFFSET, slcr_reg);
    
    // lock SLCR
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCR_LOCK_OFFSET, XSLCR_LOCK_KEY);
    
    // Initialize high level serial driver
    (chan->callbacks->serial_init)(chan);

    // Disable RX & TX
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_CONTROL, XUARTPS_CONTROL_TXDIS | XUARTPS_CONTROL_RXDIS);
    // Reset RX & TX    
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_CONTROL, XUARTPS_CONTROL_TXRES | XUARTPS_CONTROL_RXRES);
    // Enable RX & TX
    HAL_READ_UINT32(zynq_chan->base + XUARTPS_CONTROL, control_reg);
    control_reg &= ~(XUARTPS_CONTROL_TXDIS | XUARTPS_CONTROL_RXDIS);
    control_reg |= (XUARTPS_CONTROL_TXEN | XUARTPS_CONTROL_RXEN);
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_CONTROL, control_reg);
    
    // configure port
    zynq_serial_config_port(chan, &chan->config, true);

    // set receiver receiver trigger level
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_RX_WM, 56);
    // set receiver timeout value
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_RX_TOUT, 10);
    
    // Register interrupt (only if buffers are defined)
    if (chan->out_cbuf.len != 0) 
    {
        cyg_drv_interrupt_create(zynq_chan->int_num,
                                 0,
                                 (cyg_addrword_t)chan,   //  Data item passed to interrupt handler
                                 zynq_serial_ISR,
                                 zynq_serial_DSR,
                                 &zynq_chan->serial_interrupt_handle,
                                 &zynq_chan->serial_interrupt);
        cyg_drv_interrupt_attach(zynq_chan->serial_interrupt_handle);
        cyg_drv_interrupt_unmask(zynq_chan->int_num);
    }

    // clear ISR status register
    HAL_READ_UINT32(zynq_chan->base + XUARTPS_CHNL_INT_STS, control_reg);
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_CHNL_INT_STS, control_reg);
    // enable receiver interrupts
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_INTRPT_EN, XUARTPS_INTRPT_EN_RTRIG | XUARTPS_INTRPT_EN_TIMEOUT); 

    return true;
}

// Enable the transmitter on the device
static void zynq_serial_start_xmit(serial_channel *chan)
{
    zynq_serial_info *zynq_chan = (zynq_serial_info *)chan->dev_priv;
    cyg_uint32 reg;

    HAL_READ_UINT32(zynq_chan->base + XUARTPS_CONTROL, reg);
    reg &= ~XUARTPS_CONTROL_TXDIS; // both TXDIS
    reg |= XUARTPS_CONTROL_TXEN; // and TXEN must be set correctly
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_CONTROL, reg);

    HAL_READ_UINT32(zynq_chan->base + XUARTPS_CHNL_INT_STS, reg);
    if ((reg & XUARTPS_CHNL_INT_STS_TFULL) != XUARTPS_CHNL_INT_STS_TFULL)
    {
        chan->callbacks->xmt_char(chan);
    }

    // Clear Tx EMPTY flag in ISR Status Register
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_CHNL_INT_STS, XUARTPS_INTRPT_EN_TEMPTY);
    // Enable Tx EMPTY interrupt
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_INTRPT_EN, XUARTPS_INTRPT_EN_TEMPTY);
}

// Disable the transmitter on the device
static void zynq_serial_stop_xmit(serial_channel *chan)
{
    zynq_serial_info *zynq_chan = (zynq_serial_info *)chan->dev_priv;
    cyg_uint32 reg;

    // Disable transmitter
    HAL_READ_UINT32(zynq_chan->base + XUARTPS_CONTROL, reg);
    reg &= ~XUARTPS_CONTROL_TXEN; // this is enough to disable
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_CONTROL, reg);
    // Disable Tx FULL interrupt
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_INTRPT_DIS, XUARTPS_INTRPT_EN_TEMPTY | XUARTPS_INTRPT_EN_TFUL);
}


// Send a character to the device output buffer.
// Return 'true' if character is sent to device
static bool zynq_serial_putc(serial_channel *chan, unsigned char c)
{
    zynq_serial_info *zynq_chan = (zynq_serial_info *)chan->dev_priv;
    cyg_uint32 status;

    // get channel status
    HAL_READ_UINT32(zynq_chan->base + XUARTPS_CHANNEL_STS, status);

    // Check if FIFO is full, so we can not put a char into FIFO
    if(status & XUARTPS_CHANNEL_STS_TFUL)
        return false;

    // put character into FIFO    
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_TX_RX_FIFO, c);
    return true;
}

// Fetch a character from the device input buffer, waiting if necessary
static unsigned char zynq_serial_getc(serial_channel *chan)
{
    unsigned char c;
    cyg_uint32 temp;
    zynq_serial_info *zynq_chan = (zynq_serial_info *)chan->dev_priv;

    // Wait for data if FIFO is empty
    do
    {
        HAL_READ_UINT32(zynq_chan->base + XUARTPS_CHANNEL_STS, temp);
    }
    while(temp & XUARTPS_CHANNEL_STS_REMPTY);
        
    // Return recived data from FIFO
    HAL_READ_UINT32(zynq_chan->base + XUARTPS_TX_RX_FIFO, temp);
    c =  (unsigned char)(temp & 0xFF);
    return c;
}

// This routine is called when the device is "looked" up (i.e. attached)
static Cyg_ErrNo zynq_serial_lookup(struct cyg_devtab_entry **tab, struct cyg_devtab_entry *sub_tab, const char *name)
{
    serial_channel *chan = (serial_channel *)(*tab)->priv;
    // Really only required for interrupt driven devices
    (chan->callbacks->serial_init)(chan);
    return ENOERR;
}

// Serial I/O - low level interrupt handler (ISR)
static cyg_uint32 zynq_serial_ISR(cyg_vector_t vector, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    zynq_serial_info *zynq_chan = (zynq_serial_info *)chan->dev_priv;
    
    cyg_drv_interrupt_mask(zynq_chan->int_num);
    cyg_drv_interrupt_acknowledge(zynq_chan->int_num);
    
    return (CYG_ISR_HANDLED | CYG_ISR_CALL_DSR);  // Cause DSR to be run
}

// Serial I/O - high level interrupt handler (DSR)
static void zynq_serial_DSR(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
    serial_channel *chan = (serial_channel *)data;
    zynq_serial_info *zynq_chan = (zynq_serial_info *)chan->dev_priv;
    volatile cyg_uint32 isr;
    cyg_uint32 status, temp;
    unsigned char c;

    HAL_READ_UINT32(zynq_chan->base + XUARTPS_CHNL_INT_STS, isr);

    if (isr & XUARTPS_CHNL_INT_STS_TEMPTY)
		(chan->callbacks->xmt_char)(chan);

    if ((isr & XUARTPS_CHNL_INT_STS_RTRIG) || (isr & XUARTPS_CHNL_INT_STS_RTOUT)) // Data in the RxFiFo
    {
        HAL_READ_UINT32(zynq_chan->base + XUARTPS_CHANNEL_STS, status);
        while( !(status & XUARTPS_CHANNEL_STS_REMPTY) )
        {
            HAL_READ_UINT32(zynq_chan->base + XUARTPS_TX_RX_FIFO, temp);
            c = (unsigned char)(temp & 0xff);
            (chan->callbacks->rcv_char)(chan, c);
            HAL_READ_UINT32(zynq_chan->base + XUARTPS_CHANNEL_STS, status);
        }
        
    }
    
    // clear
    HAL_WRITE_UINT32(zynq_chan->base + XUARTPS_CHNL_INT_STS, isr);
    cyg_drv_interrupt_unmask(zynq_chan->int_num);
}

#endif //CYGPKG_IO_SERIAL_ARM_XC7Z

//----------------------------------------------------------------------------
// EOF 

