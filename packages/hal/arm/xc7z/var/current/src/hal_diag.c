/**************************************************************************/
/**
*
* @file     hal_diag.c
*
* @brief    HAL diagnostic output code
*
***************************************************************************/
/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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
// Author(s):   ITR-GmbH, Ant Micro <www.antmicro.com>
// Contributors:
// Date:        2012-09-27
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include CYGBLD_HAL_PLATFORM_H

#include <cyg/infra/cyg_type.h>         // base types

#include <cyg/hal/hal_arch.h>           // SAVE/RESTORE GP macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_intr.h>           // HAL_ENABLE/MASK/UNMASK_INTERRUPTS
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/drv_api.h>            // CYG_ISR_HANDLED
#include <cyg/hal/hal_diag.h>

#include <cyg/hal/var_io.h>



//-----------------------------------------------------------------------------
typedef struct {
    cyg_uint8* base;
    cyg_int32 msec_timeout;
    int isr_vector;
    int baud_rate;
} channel_data_t;

//-----------------------------------------------------------------------------

void
cyg_hal_plf_serial_putc(void *__ch_data, char c);

////////////////////////////////////////////////////////////////////

# define PS_CLK         33333000
# define XDFUART_MASTER 50000000

/****************************************************************************/
/**
*
* Set baudrate for serial port which used for diag output
*
* @param    __ch_data - pointer to channel structure.
*
* @return   none
*
*****************************************************************************/
void
cyg_hal_plf_serial_setbrg(void* __ch_data)
{
    cyg_uint32 uart_base = (cyg_uint32)((channel_data_t*)__ch_data)->base;
    
    /*              master clock
     * Baud rate = ---------------
     *              bgen*(bdiv+1)
     */
    long baud = CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD;

    /* Variables to vary. */
    unsigned int bdiv, bgen;

    /* Calculation results. */
    long calc_baud = 0;
    unsigned int calc_bauderror;

    /* Find acceptable values for baud generation. */
    for (bdiv = 4; bdiv < 255; bdiv++) {

        bgen = XDFUART_MASTER / (baud * (bdiv + 1));
        if (bgen < 2 || bgen > 65535)
            continue;

        calc_baud = XDFUART_MASTER / (bgen * (bdiv + 1));

        /* Use first calculated baudrate with an acceptable
         * (<3%) error.
         */
        if (baud > calc_baud)
            calc_bauderror = baud - calc_baud;
        else
            calc_bauderror = calc_baud - baud;
        if ( ((calc_bauderror * 100) / baud) < 3 )
            break;

    }

    HAL_WRITE_UINT32(uart_base + XDFUART_BAUDDIV_OFFSET, bdiv);
    HAL_WRITE_UINT32(uart_base + XDFUART_BAUDGEN_OFFSET, bgen);
}

/****************************************************************************/
/**
*
* Initialize serial port which used for diag output
*
* @param    __ch_data - pointer to channel structure.
*
* @return   none
*
*****************************************************************************/
static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
    cyg_uint32 uart_base = (cyg_uint32)((channel_data_t*)__ch_data)->base;
    cyg_uint32 apbctl;

    cyg_uint32 pll_clock, calc_uart_clk;
    cyg_uint32 slcr_reg, pll_fdiv;
    cyg_uint16 uart_divisor, best_uart_divisor;
    cyg_int32 act_error, prev_error;

    // Calculate DIVISOR value for XDFUART_MASTER
    // Read I/O PLL configuration
    HAL_READ_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRIO_PLL_CTRL_OFFSET, pll_fdiv);
    // extract value of PLL divisor
    pll_fdiv = pll_fdiv >> 12;
    pll_fdiv = pll_fdiv & 0x3f;
    // Calculate I/O PLL freq on given PS_CLK
    pll_clock = PS_CLK * pll_fdiv;
    // Calculate best DIVISOR value to obtain XDFUART_MASTER on given I/O PLL clock
    prev_error = XDFUART_MASTER;
    for(uart_divisor = 1; uart_divisor < 64; uart_divisor++)
    {
        calc_uart_clk = pll_clock / uart_divisor;
        if(calc_uart_clk > XDFUART_MASTER)        
            act_error = calc_uart_clk - XDFUART_MASTER;
        else
            act_error = XDFUART_MASTER - calc_uart_clk;

        if(act_error < prev_error)
        {
            best_uart_divisor = uart_divisor;
            prev_error = act_error;
        }
    }
    best_uart_divisor &= 0x3f;
    
    /* Unlock SLCR regs */
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCR_UNLOCK_OFFSET, XSLCR_UNLOCK_KEY);
    
    /* Enable AMBA Peripheral Clock */
    if (uart_base == XC7Z_UART0_BASEADDR)
    {
        HAL_READ_UINT32( XC7Z_SYS_CTRL_BASEADDR + XSLCRAPER_CLK_CTRL_OFFSET, apbctl);
        apbctl |= XSLCRAPER_CLK_CTRL_UART0_EN;
        HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRAPER_CLK_CTRL_OFFSET, apbctl);
    } else {
        HAL_READ_UINT32( XC7Z_SYS_CTRL_BASEADDR + XSLCRAPER_CLK_CTRL_OFFSET, apbctl);
        apbctl |= XSLCRAPER_CLK_CTRL_UART1_EN;
        HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRAPER_CLK_CTRL_OFFSET, apbctl);
    }

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
    // enable UARTs Clock in UART_CLK_CTRL
    slcr_reg |= (XSLCRUART_CLK_CTRL_CLKACT0_EN | XSLCRUART_CLK_CTRL_CLKACT0_EN);
	// write config to UART_CLK_CTRL
	HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCRUART_CLK_CTRL_OFFSET, slcr_reg);
    
    /* Lock SLCR regs */
    HAL_WRITE_UINT32(XC7Z_SYS_CTRL_BASEADDR + XSLCR_LOCK_OFFSET, XSLCR_LOCK_KEY);
    
    /* RX/TX enabled & reset */
    HAL_WRITE_UINT32(uart_base + XDFUART_CR_OFFSET,
                     XDFUART_CR_TX_EN | XDFUART_CR_RX_EN | XDFUART_CR_TXRST | XDFUART_CR_RXRST);
    /* 8 bit, no parity */
    HAL_WRITE_UINT32(uart_base + XDFUART_MR_OFFSET,   XDFUART_MR_PARITY_NONE);
    /* 1 byte trigger level */
    HAL_WRITE_UINT32(uart_base + XDFUART_RXWM_OFFSET, 0x01);
    
    /* Set bardrate */
    cyg_hal_plf_serial_setbrg(__ch_data);
    
}

/****************************************************************************/
/**
*
* Put char in diag output
*
* @param    __ch_data - pointer to channel structure.
* @param    c         - byte to transmit.
*
* @return   none
*
*****************************************************************************/
void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
    cyg_uint32 uart_base = (cyg_uint32)((channel_data_t*)__ch_data)->base;
    cyg_uint32 status;
    CYGARC_HAL_SAVE_GP();
    
    do {
        HAL_READ_UINT32(uart_base + XDFUART_SR_OFFSET, status);
    } while ((status & XDFUART_SR_TXFULL) != 0); 

    HAL_WRITE_UINT32(uart_base + XDFUART_FIFO_OFFSET, c);

    CYGARC_HAL_RESTORE_GP();
}

/****************************************************************************/
/**
*
* Get char from diag output with non block functionality
*
* @param    __ch_data - pointer to channel structure.
* @param    ch        - pointer to variable which will be filled with byte.
*
* @return   none
*
*****************************************************************************/
static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
    cyg_uint32 uart_base = (cyg_uint32)((channel_data_t*)__ch_data)->base;
    cyg_uint32 stat;
    cyg_uint32 c;
    
    HAL_READ_UINT32(uart_base + XDFUART_SR_OFFSET, stat);
    
    if (stat & XDFUART_SR_RXEMPTY)
        return false;

    HAL_READ_UINT32(uart_base + XDFUART_FIFO_OFFSET, c);
    *ch = (cyg_uint8)(c & 0xff);
        
    return true;
}

/****************************************************************************/
/**
*
* Get char from diag output
*
* @param    __ch_data - pointer to channel structure.
*
* @return   received byte
*
*****************************************************************************/
cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}

/****************************************************************************/
/**
*
* Send data to diag output
*
* @param    __ch_data - pointer to channel structure.
* @param    __buf     - pointer to data buffer.
* @param    __len     - length of data in bytes.
*
* @return   None
*
*****************************************************************************/
static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

/****************************************************************************/
/**
*
* Read data from diag output
*
* @param    __ch_data - pointer to channel structure.
* @param    __buf     - pointer to data buffer.
* @param    __len     - length of data in bytes.
*
* @return   None
*
*****************************************************************************/
static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

/****************************************************************************/
/**
*
* Read byte from diag output with timeout
*
* @param    __ch_data - pointer to channel structure.
* @param    ch        - pointer to variable.
*
* @return   TRUE if read was successful, overwise FALSE
*
*****************************************************************************/
cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();

    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

/****************************************************************************/
/**
*
* Serial control function
*
* @param    __ch_data - pointer to channel structure.
* @param    __func    - function.
*
* @return   result depends on function
*
*****************************************************************************/
static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint32 uart_base = (cyg_uint32)((channel_data_t*)__ch_data)->base;
    int ret = 0;
    va_list ap;

    CYGARC_HAL_SAVE_GP();
    va_start(ap, __func);

    switch (__func) {
    case __COMMCTL_GETBAUD:
        ret = chan->baud_rate;
        break;
    case __COMMCTL_SETBAUD:
        chan->baud_rate = va_arg(ap, cyg_int32);
        // Should we verify this value here?
        cyg_hal_plf_serial_setbrg(chan);
        ret = 0;
        break;
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
        HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);
        HAL_INTERRUPT_UNMASK(chan->isr_vector);
        HAL_WRITE_UINT32(uart_base + XDFUART_IER_OFFSET, XDFUART_IXR_RXOVR);
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        HAL_INTERRUPT_MASK(chan->isr_vector);
        HAL_WRITE_UINT32(uart_base + XDFUART_IDR_OFFSET, XDFUART_IXR_RXOVR);
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);
    default:
        break;
    }

    va_end(ap);
    CYGARC_HAL_RESTORE_GP();
    return ret; 
}

/****************************************************************************/
/**
*
* Serial interrupt routine
*
* @param    __ch_data - pointer to channel structure.
* @param    __ctrlc   - bool - break indicator.
* @param    __vector  - interrupt number.
* @param    __data    - ?.
*
* @return   TRUE if read was successful, overwise FALSE
*
*****************************************************************************/
static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
    int res = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_uint32 uart_base = (cyg_uint32)((channel_data_t*)__ch_data)->base;
    cyg_uint32 c;
    cyg_uint8  ch;
    cyg_uint32 stat, imr;
    CYGARC_HAL_SAVE_GP();

    *__ctrlc = 0;
    HAL_READ_UINT32(uart_base + XDFUART_IMR_OFFSET, imr);
    HAL_READ_UINT32(uart_base + XDFUART_ISR_OFFSET, stat);
    stat &= imr;
    
    if ( (stat & XDFUART_IXR_RXOVR) != 0 ) {

        HAL_READ_UINT32(uart_base + XDFUART_FIFO_OFFSET, c);
        ch = (cyg_uint8)(c & 0xff);
        if( cyg_hal_is_break( (char *)&ch , 1 ) )
            *__ctrlc = 1;

        res = CYG_ISR_HANDLED;
        
        HAL_WRITE_UINT32(uart_base + XDFUART_ISR_OFFSET, stat);

    }

    HAL_INTERRUPT_ACKNOWLEDGE(chan->isr_vector);

    CYGARC_HAL_RESTORE_GP();
    return res;
}

static channel_data_t xc7z_ser_channels[CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS] = {
#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 0
    { (cyg_uint8*)XC7Z_UART0_BASEADDR, 1000, CYGNUM_HAL_INTERRUPT_UART0, CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD},
#if CYGNUM_HAL_VIRTUAL_VECTOR_COMM_CHANNELS > 1
    { (cyg_uint8*)XC7Z_UART1_BASEADDR, 1000, CYGNUM_HAL_INTERRUPT_UART1, CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD}
#endif
#endif
};

/****************************************************************************/
/**
*
* Initialize diag functions. Init only UART1 for Zynq eval kit.
*
* @return   None
*
*****************************************************************************/
static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur;
    int chan_num;

#ifdef CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL
    chan_num = CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL;
#else
    chan_num = 1; // UART1 is default
#endif

    cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    cyg_hal_plf_serial_init_channel(&xc7z_ser_channels[chan_num]); 


    // Set channel
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &xc7z_ser_channels[chan_num]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);
    
    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

/****************************************************************************/
/**
*
* Comms initialization
*
*
* @return   None
*
*****************************************************************************/
void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();

}

/****************************************************************************/
/**
*
* Debug LED initialization
*
*
* @return   None
*
*****************************************************************************/
void
hal_diag_led(int mask)
{
    // Fixme: need to use DS12, DS23 LEDs
}

//-----------------------------------------------------------------------------
// End of hal_diag.c
