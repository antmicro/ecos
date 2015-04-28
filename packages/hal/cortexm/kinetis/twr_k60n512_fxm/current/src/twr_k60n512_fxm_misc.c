//==========================================================================
//
//      twr_k60n512_fxm_misc.c
//
//      Cortex-M3 TWR-K60N512 EVAL + TWR_FXM HAL functions
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2008 Free Software Foundation, Inc.
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
// Author(s):      ilijak
// Contributor(s):
// Date:           2011-02-05
// Description:
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <pkgconf/hal_cortexm.h>
#include <pkgconf/hal_cortexm_kinetis.h>
#include <pkgconf/hal_cortexm_kinetis_twr_k60n512_fxm.h>
#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>
#endif

#include <cyg/infra/diag.h>
#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // HAL header
#include <cyg/hal/hal_intr.h>           // HAL header

#ifdef CYGPKG_HAL_CORTEXM_KINETIS_FLEXBUS
#include <cyg/hal/hal_kinetis_flexbus.h>
#include <cyg/devs/ram_micron_cellularram.h>

// Dependent on FlexBus memory properties up to two flexbus setting stages
// may be needed

// Macro for final setting of CSCR
// Final settings take place when system clock is completely initialized and
// it is possible to program FlexBus memory with desired setting.
#define CYGHWR_HAL_FB_CSCR(__bits,__cs) \
            VALUE_(CYGHWR_HAL_FB_CSCR_##__bits##_S, \
                   CYGHWR_HAL_KINETIS_FB_CS##__cs##_CR_##__bits)

#define CYGHWR_HAL_KINETIS_FB_CS_CR(__cs) ( CYGHWR_HAL_FB_CSCR(SWS, __cs)  + \
         CYGHWR_HAL_FB_CSCR(SWSEN, __cs) + CYGHWR_HAL_FB_CSCR(EXALE, __cs) + \
         CYGHWR_HAL_FB_CSCR(ASET, __cs)  + CYGHWR_HAL_FB_CSCR(RDAH, __cs)  + \
         CYGHWR_HAL_FB_CSCR(WRAH, __cs)  + CYGHWR_HAL_FB_CSCR(WS, __cs)    + \
         CYGHWR_HAL_FB_CSCR(BLS, __cs)   + CYGHWR_HAL_FB_CSCR(AA, __cs)    + \
         CYGHWR_HAL_FB_CSCR(PS, __cs)    + CYGHWR_HAL_FB_CSCR(BEM, __cs)   + \
         CYGHWR_HAL_FB_CSCR(BSTR, __cs)  + CYGHWR_HAL_FB_CSCR(BSTW, __cs))

// Macros for initial settings of CSCR
// Initial settings are used immediately after boot and make it possible to
// utilize FlexBus memory with it's power-up setting.
#define CYGHWR_HAL_FB_CSCR_IS(__bits,__cs) \
            VALUE_(CYGHWR_HAL_FB_CSCR_##__bits##_S, \
                   CYGHWR_HAL_KINETIS_FB_CS##__cs##_CR_IS_##__bits)

#define CYGHWR_HAL_KINETIS_FB_CS_CR_IS(__cs) ( CYGHWR_HAL_FB_CSCR_IS(SWS, __cs)   + \
          CYGHWR_HAL_FB_CSCR_IS(SWSEN, __cs) + CYGHWR_HAL_FB_CSCR_IS(EXALE, __cs) + \
          CYGHWR_HAL_FB_CSCR_IS(ASET, __cs)  + CYGHWR_HAL_FB_CSCR_IS(RDAH, __cs)  + \
          CYGHWR_HAL_FB_CSCR_IS(WRAH, __cs)  + CYGHWR_HAL_FB_CSCR_IS(WS, __cs)    + \
          CYGHWR_HAL_FB_CSCR(BLS, __cs)      + CYGHWR_HAL_FB_CSCR_IS(AA, __cs)    + \
          CYGHWR_HAL_FB_CSCR(PS, __cs)       + CYGHWR_HAL_FB_CSCR_IS(BEM, __cs)   + \
          CYGHWR_HAL_FB_CSCR_IS(BSTR, __cs)  + CYGHWR_HAL_FB_CSCR_IS(BSTW, __cs))

// Functions for final and initial FlexBus setting
// Description is with the code below.
static inline void hal_flexbus_init_initial(void);
static inline void hal_flexbus_init_final(void);
#endif //  CYGPKG_HAL_CORTEXM_KINETIS_FLEXBUS

static inline void hal_misc_init(void);

// DATA and BSS locations
__externC cyg_uint32 __ram_data_start;
__externC cyg_uint32 __ram_data_end;
__externC cyg_uint32 __rom_data_start;
__externC cyg_uint32 __sram_data_start;
__externC cyg_uint32 __sram_data_end;
__externC cyg_uint32 __srom_data_start;
__externC cyg_uint32 __bss_start;
__externC cyg_uint32 __bss_end;

//==========================================================================
// System init
//
// This is run to set up the basic system, including GPIO setting,
// clock feeds, power supply, and memory initialization. This code
// runs before the DATA is copied from ROM and the BSS cleared, hence
// it cannot make use of static variables or data tables.

__externC void CYGOPT_HAL_KINETIS_MISC_FLASH_SECTION_ATTR
hal_system_init( void )
{
#if defined(CYG_HAL_STARTUP_ROM) || defined(CYG_HAL_STARTUP_SRAM)
    hal_wdog_disable();
    hal_misc_init();
# ifdef CYGPKG_HAL_CORTEXM_KINETIS_FLEXBUS
    {
        // This delay is needed for Micron RAM wake-up.
        cyg_uint32 busy_delay;
        for(busy_delay = 0x100000; busy_delay; busy_delay--)
            __asm__ volatile ("nop\n");

        hal_flexbus_init_initial();

        hal_start_clocks();

        for(busy_delay = 0x100000; busy_delay; busy_delay--)
            __asm__ volatile ("nop\n");

        hal_flexbus_init_final();
    }
# else
        hal_start_clocks();
# endif
#endif
}

//===========================================================================
// hal_misc_init
//===========================================================================
#define CYGHWR_HAL_KINETIS_SIM_SCGC5_PORT_M           \
            (CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTA_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTB_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTC_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTD_M |   \
             CYGHWR_HAL_KINETIS_SIM_SCGC5_PORTE_M)

static inline void CYGOPT_HAL_KINETIS_MISC_FLASH_SECTION_ATTR
hal_misc_init(void)
{
    cyghwr_hal_kinetis_sim_t *sim_p = CYGHWR_HAL_KINETIS_SIM_P;
    cyghwr_hal_kinetis_mpu_t *mpu_p = CYGHWR_HAL_KINETIS_MPU_P;

    // Enable some peripherals' clocks.
    sim_p->scgc5 |= CYGHWR_HAL_KINETIS_SIM_SCGC5_PORT_M;
    sim_p->scgc6 |= CYGHWR_HAL_KINETIS_SIM_SCGC6_RTC_M;

    // Disable MPU
    mpu_p->cesr = 0;
}

#ifdef CYGPKG_HAL_CORTEXM_KINETIS_FLEXBUS

// FlexBus

static const cyg_uint32 const flexbus_pins[] = {
    CYGHWR_KINETIS_FB_PIN_CLKOUT_OFF,

    CYGHWR_KINETIS_FB_PIN_CS0,
    CYGHWR_KINETIS_FB_PIN_OE,
    CYGHWR_KINETIS_FB_PIN_RW,
# if 0 //(CYGHWR_HAL_KINETIS_FB_CS_CR(0) & CYGHWR_HAL_FB_CSCR_BLS_M)
    CYGHWR_KINETIS_FB_PIN_BLS7_0,
    CYGHWR_KINETIS_FB_PIN_BLS15_8,
# else
    CYGHWR_KINETIS_FB_PIN_BE23_16,
    CYGHWR_KINETIS_FB_PIN_BE31_24,
# endif
    CYGHWR_KINETIS_FB_PIN_TS,

    CYGHWR_KINETIS_FB_PIN_AD0,
    CYGHWR_KINETIS_FB_PIN_AD1,
    CYGHWR_KINETIS_FB_PIN_AD2,
    CYGHWR_KINETIS_FB_PIN_AD3,
    CYGHWR_KINETIS_FB_PIN_AD4,
    CYGHWR_KINETIS_FB_PIN_AD5,
    CYGHWR_KINETIS_FB_PIN_AD6,
    CYGHWR_KINETIS_FB_PIN_AD7,
    CYGHWR_KINETIS_FB_PIN_AD8,
    CYGHWR_KINETIS_FB_PIN_AD9,
    CYGHWR_KINETIS_FB_PIN_AD10,
    CYGHWR_KINETIS_FB_PIN_AD11,
    CYGHWR_KINETIS_FB_PIN_AD12,
    CYGHWR_KINETIS_FB_PIN_AD13,
    CYGHWR_KINETIS_FB_PIN_AD14,
    CYGHWR_KINETIS_FB_PIN_AD15,
    CYGHWR_KINETIS_FB_PIN_AD16,
    CYGHWR_KINETIS_FB_PIN_AD17
# if CYGHWR_HAL_KINETIS_FB_CS0_SIZE > 0x00040000
    ,
    CYGHWR_KINETIS_FB_PIN_AD18,
    CYGHWR_KINETIS_FB_PIN_AD19,
    CYGHWR_KINETIS_FB_PIN_AD20,
    CYGHWR_KINETIS_FB_PIN_AD21
#  if CYGHWR_HAL_KINETIS_FB_CS0_SIZE > 0x00400000
    ,
    CYGHWR_KINETIS_FB_PIN_AD22,
    CYGHWR_KINETIS_FB_PIN_AD23
#   if 0 //!(CYGHWR_HAL_KINETIS_FB_CS_CR(0) & CYGHWR_HAL_FB_CSCR_BLS_M)
    ,
    CYGHWR_KINETIS_FB_PIN_AD24,
    CYGHWR_KINETIS_FB_PIN_AD25,
    CYGHWR_KINETIS_FB_PIN_AD26,
    CYGHWR_KINETIS_FB_PIN_AD27,
    CYGHWR_KINETIS_FB_PIN_AD28,
    CYGHWR_KINETIS_FB_PIN_AD29,
    CYGHWR_KINETIS_FB_PIN_AD30,
    CYGHWR_KINETIS_FB_PIN_AD31
#   endif
#  endif  // CYGHWR_HAL_KINETIS_FB_CS0_SIZE > 0x00400000
# endif // CYGHWR_HAL_KINETIS_FB_CS0_SIZE > 0x00040000
};

static const cyg_uint32 const flexbus_pins_final_diff[] = {
    CYGHWR_KINETIS_FB_PIN_CLKOUT
};

// Initialize FlexBus for use until we have main clock.
// Asynchronous mode.
static inline void CYGOPT_HAL_KINETIS_MISC_FLASH_SECTION_ATTR
hal_flexbus_init_initial(void)
{
    cyghwr_hal_kinetis_fb_t *fb_p = CYGHWR_HAL_KINETIS_FB_P;

    CYGHWR_IO_CLOCK_ENABLE(CYGHWR_HAL_KINETIS_SIM_SCGC_FLEXBUS);

# ifdef CYGHWR_HAL_KINETIS_FB_CS0
    fb_p->csel[0] = (cyghwr_hal_kinetis_fbcs_t) { CYGHWR_HAL_KINETIS_FB_CS0_AR,
              CYGHWR_HAL_KINETIS_FB_CS0_MR, CYGHWR_HAL_KINETIS_FB_CS_CR_IS(0) };
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS1
    fb_p->csel[1] = (cyghwr_hal_kinetis_fbcs_t) { CYGHWR_HAL_KINETIS_FB_CS1_AR,
              CYGHWR_HAL_KINETIS_FB_CS1_MR, CYGHWR_HAL_KINETIS_FB_CS_CR_IS(1) };
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS2
    fb_p->csel[2] = (cyghwr_hal_kinetis_fbcs_t) { CYGHWR_HAL_KINETIS_FB_CS2_AR,
              CYGHWR_HAL_KINETIS_FB_CS2_MR, CYGHWR_HAL_KINETIS_FB_CS_CR_IS(2) };
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS3
    fb_p->csel[3] = (cyghwr_hal_kinetis_fbcs_t) { CYGHWR_HAL_KINETIS_FB_CS3_AR,
              CYGHWR_HAL_KINETIS_FB_CS3_MR, CYGHWR_HAL_KINETIS_FB_CS_CR_IS(3) };
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS4
    fb_p->csel[4] = (cyghwr_hal_kinetis_fbcs_t) { CYGHWR_HAL_KINETIS_FB_CS4_AR,
              CYGHWR_HAL_KINETIS_FB_CS4_MR, CYGHWR_HAL_KINETIS_FB_CS_CR_IS(4) };
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS4
    fb_p->csel[5] = (cyghwr_hal_kinetis_fbcs_t) { CYGHWR_HAL_KINETIS_FB_CS5_AR,
              CYGHWR_HAL_KINETIS_FB_CS5_MR, CYGHWR_HAL_KINETIS_FB_CS_CR_IS(5) };
# endif

    fb_p->cspmcr = CYGHWR_HAL_FB_CSPMCR_SETSEL;

    HAL_SET_PINS(flexbus_pins);
}

// Initialize FlexBus to it's normal working condition
// Synchronous mode with burst support
static inline void CYGOPT_HAL_KINETIS_MISC_FLASH_SECTION_ATTR
hal_flexbus_init_final(void)
{
    cyghwr_hal_kinetis_fb_t *fb_p = CYGHWR_HAL_KINETIS_FB_P;

    // Enable RAM synchronous/burst mode
    ram_micron_reg_set(CYGHWR_DEVS_RAM_MICRON_BCR,
                           CYGHWR_RAM0_MICRON_BCR_SETTO,
                           (cyg_uint16 *)CYGHWR_HAL_KINETIS_FB_CS0_AR,
                       CYGHWR_RAM0_MICRON_CELLULAR_SIZE);

    // Apply FlexBus clock...
    HAL_SET_PINS(flexbus_pins_final_diff);

    // ...and final CS setting for burst, etc.
# ifdef CYGHWR_HAL_KINETIS_FB_CS0
    fb_p->csel[0].cscr = CYGHWR_HAL_KINETIS_FB_CS_CR(0);
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS1
    fb_p->csel[1].cscr = CYGHWR_HAL_KINETIS_FB_CS_CR(1);
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS2
    fb_p->csel[2].cscr = CYGHWR_HAL_KINETIS_FB_CS_CR(2);
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS3
    fb_p->csel[3].cscr = CYGHWR_HAL_KINETIS_FB_CS_CR(3);
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS4
    fb_p->csel[4].cscr = CYGHWR_HAL_KINETIS_FB_CS_CR(4);
# endif
# ifdef CYGHWR_HAL_KINETIS_FB_CS5
    fb_p->csel[5].cscr = CYGHWR_HAL_KINETIS_FB_CS_CR(5);
# endif
}

#endif // CYGHWR_HAL_CORTEXM_KINETIS_FLEXBUS

//==========================================================================

__externC void hal_platform_init( void )
{
}

//==========================================================================

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

#include CYGHWR_MEMORY_LAYOUT_H

//--------------------------------------------------------------------------
// Accesses to areas not backed by real devices or memory can cause
// the CPU to hang.
//
// The following table defines the memory areas that GDB is allowed to
// touch. All others are disallowed.
// This table needs to be kept up to date with the set of memory areas
// that are available on the board.

static struct {
    CYG_ADDRESS         start;          // Region start address
    CYG_ADDRESS         end;            // End address (last byte)
} hal_data_access[] =
{
    { CYGMEM_REGION_ram,        CYGMEM_REGION_ram+CYGMEM_REGION_ram_SIZE-1        }, // System bus RAM partition
#if 1 //def CYGMEM_REGION_sram
    { CYGMEM_REGION_sram,       CYGMEM_REGION_sram+CYGMEM_REGION_sram_SIZE-1      }, // On-chip SRAM
#elif defined CYGMEM_REGION_sram_l
    { CYGMEM_REGION_sram_l,     CYGMEM_REGION_sram_l+CYGMEM_REGION_sram_l_SIZE-1  }, // On-chip SRAM lower bank
#endif
#ifdef CYGMEM_REGION_ramcod
    { CYGMEM_REGION_ramcod,      CYGMEM_REGION_ramcod+CYGMEM_REGION_ramcod_SIZE-1 }, // Code bus RAM partition
#endif
#ifdef CYGMEM_REGION_ramnc
    { CYGMEM_REGION_ramnc,      CYGMEM_REGION_ramnc+CYGMEM_REGION_ramnc_SIZE-1    }, // Non cachable RAM partition
#endif
#ifdef CYGMEM_REGION_flash
    { CYGMEM_REGION_flash,      CYGMEM_REGION_flash+CYGMEM_REGION_flash_SIZE-1    }, // On-chip flash
#endif
#ifdef CYGMEM_REGION_rom
    { CYGMEM_REGION_rom,        CYGMEM_REGION_rom+CYGMEM_REGION_rom_SIZE-1        }, // External flash [currently none]
#endif
    { 0xE0000000,               0x00000000-1                                      }, // Cortex-M peripherals
    { 0x40000000,               0x60000000-1                                      }  // Chip specific peripherals
};

__externC int cyg_hal_stub_permit_data_access( void* addr, cyg_uint32 count )
{
    int i;
    for( i = 0; i < sizeof(hal_data_access)/sizeof(hal_data_access[0]); i++ ) {
        if( ((CYG_ADDRESS)addr >= hal_data_access[i].start) &&
            ((CYG_ADDRESS)addr+count) <= hal_data_access[i].end)
            return true;
    }
    return false;
}

#endif // CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS

//==========================================================================

#ifdef CYGPKG_REDBOOT
#include <redboot.h>
#include CYGHWR_MEMORY_LAYOUT_H

//--------------------------------------------------------------------------
// Memory layout
//
// We report the on-chip SRAM and external RAM.

void
cyg_plf_memory_segment(int seg, unsigned char **start, unsigned char **end)
{
    switch (seg) {
    case 0:
        *start = (unsigned char *)CYGMEM_REGION_ram;
        *end = (unsigned char *)(CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE);
        break;
#ifdef CYGMEM_REGION_sram
    case 1:
        *start = (unsigned char *)CYGMEM_REGION_sram;
        *end = (unsigned char *)(CYGMEM_REGION_sram + CYGMEM_REGION_sram_SIZE);
        break;
#endif
#ifdef CYGMEM_REGION_sram_l
# define CASE_CODE 3
# define CASE_RAMNC 4
    case 2:
        *start = (unsigned char *)CYGMEM_REGION_sram_l;
        *end = (unsigned char *)(CYGMEM_REGION_sram_l + CYGMEM_REGION_sram_l_SIZE);
        break;
#else
# define CASE_CODE 2
# define CASE_RAMNC 3
#endif

#ifdef CYGMEM_REGION_ramcod
    case CASE_CODE:
        *start = (unsigned char *)CYGMEM_REGION_ramcod;
        *end = (unsigned char *)(CYGMEM_REGION_ramcod + CYGMEM_REGION_ramcod_SIZE);
        break;
#endif

#ifdef CYGMEM_REGION_ramnc
    case CASE_RAMNC:
        *start = (unsigned char *)CYGMEM_REGION_ramnc;
        *end = (unsigned char *)(CYGMEM_REGION_ramnc + CYGMEM_REGION_ramnc_SIZE);
        break;
#endif
    default:
        *start = *end = NO_MEMORY;
        break;
    }
} // cyg_plf_memory_segment()

#endif // CYGPKG_REDBOOT

//==========================================================================
// EOF twr_k60n512_fxm_misc.c
