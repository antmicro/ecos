//==========================================================================
//
//      vybrid_clocking.c
//
//      Cortex-M Vybrid HAL functions
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
// Author(s):    Antmicro Ltd <contact@antmicro.com>
// Based on:	 {...}/hal/packages/cortexm/kinetis/var/current/include/kinetis_clocking.h
// Date:         2014-03-28
// Description:
//
//####DESCRIPTIONEND####
//
//========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm.h>
#include <pkgconf/hal_cortexm_vybrid.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/cortexm_endian.h>
#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL header
#include <cyg/hal/hal_if.h>             // HAL header

#include <cyg/io/ser_freescale_uart.h>

//===========================================================================
// Forward declarations
//===========================================================================

cyg_uint32 hal_cortexm_systick_clock;
cyg_uint32 hal_vybrid_sysclk;
cyg_uint32 hal_vybrid_busclk;

cyg_uint32 hal_get_cpu_clock(void);

cyg_uint32
hal_get_cpu_clock(void)
{
    cyghwr_hal_vybrid_ccm_t *ccm = CYGHWR_HAL_VYBRID_CCM_P;
    cyg_uint32 sys_clk_sel;
    cyg_uint32 pfd_sel, pfd;
    cyg_uint32 mfi, mfn, mfd;
    cyg_uint32 freq = 0, arm_clk_div, bus_clk_div;

    sys_clk_sel = (ccm->ccsr & CYGHWR_HAL_VYBRID_CCM_CCSR_SYS_CLK_SEL_M);
    switch(sys_clk_sel) {
        case 0: // Fast clock o/p defined by CCM_CCSR[FAST_CLK_SEL]
            /* 24MHz clock. It might be internal RC or external OSC*/
            freq = 24000000;
            break;
        case 1: // Slow clock o/p defined by CCM_CCSR[SLOW_CLK_SEL]
            freq = 32000;
            break;
        case 2: // PLL2 PFD o/p clock defined by CCM_CCSR[PLL2_PFD_CLK_SEL]
            pfd_sel = (ccm->ccsr & CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD_CLK_SEL_M);
            pfd_sel = (pfd_sel >> CYGHWR_HAL_VYBRID_CCM_CCSR_PLL2_PFD_CLK_SEL_S);
            if(pfd_sel) {
                HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL2_NUM, mfn);
                HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL2_DENOM, mfd);
                HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL, mfi);
                // check the value of frequency multiplier
                mfi &= CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DIV_SELECT_M;
                // if CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DIV_SELECT is set, then mfi is 22
                mfi = (mfi ? 22 : 20);
                // calculate the PLL! frequency
                freq = (24000000 * (mfi + (mfn / mfd)));
                HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL2_PFD, pfd);
                pfd = pfd >> (8*(pfd_sel-1));
                pfd &= 0x3f;
                //PFDout = PLLput * (18 / PFD_FRAC)
                freq /= pfd;
                freq *= 18;
                break;
            }
            // Fall down as the pfd_sel == 0 selects PLL2 main clock
        case 3: // PLL2 main clock
            // check if PLL2 is bypassed
            HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL, mfi);
            mfi &= CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_BYPASS_M;
            if (mfi) {
                freq = 24000000;
                break;
            }
            // get pll2 coefficients
            HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL2_NUM, mfn);
            HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL2_DENOM, mfd);
            HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL, mfi);
            // check the value of frequency multiplier
            mfi &= CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DIV_SELECT_M;
            // if CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DIV_SELECT is set, then mfi is 22
            mfi = (mfi ? 22 : 20);
            // calculate the PLL2 frequency
            freq = (24000000 * (mfi + (mfn / mfd)));
            break;
        case 4: // PLL1 PFD o/p clock defined by CCM_CCSR[PLL1_PFD_CLK_SEL]
            pfd_sel = (ccm->ccsr & CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD_CLK_SEL_M);
            pfd_sel = (pfd_sel >> CYGHWR_HAL_VYBRID_CCM_CCSR_PLL1_PFD_CLK_SEL_S);
            HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL1_NUM, mfn);
            HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL1_DENOM, mfd);
            HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL, mfi);
            // check the value of frequency multiplier
            mfi &= CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_DIV_SELECT_M;
            // if CYGHWR_HAL_VYBRID_ANADIG_PLL1_CTRL_DIV_SELECT is set, then mfi is 22
            mfi = 22;//(mfi ? 22 : 20); -> uncertain in VRM p. 726 (11.21.26 ANADIG PLL1_PFD definition register (ANADIG_PLL1_PFD))
            // calculate the PLL1 frequency PLLout = Fref*(MFI + MFN/MFD) where Fref=24MHz
            freq = (24000000 * (mfi + (mfn / mfd)));
            if(pfd_sel) {
                HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL1_PFD, pfd);
                pfd = pfd >> (8*(pfd_sel-1));
                pfd &= 0x3f;
                freq /= pfd;
                freq *= 18;
            }
            break;
        case 5: // PLL3 main clock
            HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL, mfi);
            mfi &= CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL_BYPASS_M;
            if (mfi) {
                freq = 24000000;
                break;
            }
            HAL_READ_UINT32(CYGHWR_HAL_VYBRID_ANADIG_PLL3_CTRL, mfi);
            // check the value of frequency multiplier
            mfi &= CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DIV_SELECT_M;
            // if CYGHWR_HAL_VYBRID_ANADIG_PLL2_CTRL_DIV_SELECT is set, then mfi is 22
            // freq is fixed (MFN and MFD not available) and the only variable is MFI
            freq = (mfi ? 480000000 : 440000000);
            break;
        default: // Other values are not allowed
            freq = 0;
    }

    arm_clk_div = ccm->cacrr & CYGHWR_HAL_VYBRID_CCM_CACRR_ARM_CLK_DIV_M;
    arm_clk_div = (arm_clk_div >> CYGHWR_HAL_VYBRID_CCM_CACRR_ARM_CLK_DIV_S) + 1;
    bus_clk_div = ccm->cacrr & CYGHWR_HAL_VYBRID_CCM_CACRR_BUS_CLK_DIV_M;
    bus_clk_div = (bus_clk_div >> CYGHWR_HAL_VYBRID_CCM_CACRR_BUS_CLK_DIV_S) + 1;

    freq /= arm_clk_div; // now we have a CA5 clock
    freq /= bus_clk_div; // and now we have a CM4 clock

    return freq;
}

//==========================================================================
// UART baud rate
//
// Set the baud rate divider of a UART based on the requested rate and
// the current clock settings.


void 
hal_freescale_uart_setbaud(cyg_uint32 uart_p, cyg_uint32 baud)
{
    cyg_uint32 sbr, brfa;
    cyg_uint32 regval;

    sbr = hal_vybrid_busclk / (16 * baud); //VYBRID: only this is allowed as all uarts run on BUS Clock

    if(sbr) {
        HAL_READ_UINT8(uart_p + CYGHWR_DEV_FREESCALE_UART_BDH, regval);
        regval &= 0xE0;
        regval |= sbr >> 8;
        HAL_WRITE_UINT8(uart_p + CYGHWR_DEV_FREESCALE_UART_BDH, regval);
        HAL_WRITE_UINT8(uart_p + CYGHWR_DEV_FREESCALE_UART_BDL, (sbr & 0xFF));
        brfa = (((32*hal_vybrid_busclk)/(16*baud))-(32*sbr));
        HAL_READ_UINT8(uart_p + CYGHWR_DEV_FREESCALE_UART_C4, regval);
        regval &= 0xE0;
        regval |= brfa & 0x1f;
        HAL_WRITE_UINT8(uart_p + CYGHWR_DEV_FREESCALE_UART_C4, regval);
    }
}


void
hal_update_clock_var(void)
{
    cyghwr_hal_vybrid_ccm_t *ccm = CYGHWR_HAL_VYBRID_CCM_P;
    cyg_uint32 ipg_clk_div;

    ipg_clk_div = (ccm->cacrr & CYGHWR_HAL_VYBRID_CCM_CACRR_IPG_CLK_DIV_M);
    ipg_clk_div = (ipg_clk_div >> CYGHWR_HAL_VYBRID_CCM_CACRR_IPG_CLK_DIV_S);

    hal_vybrid_sysclk = hal_get_cpu_clock();
    hal_vybrid_busclk = hal_vybrid_sysclk / (ipg_clk_div + 1);
    hal_cortexm_systick_clock = hal_vybrid_sysclk;
}


cyg_uint32
hal_get_peripheral_clock(void)
{
    return hal_vybrid_busclk;
}

//==========================================================================
// EOF vybrid_clocking.c
