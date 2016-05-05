//=============================================================================
//
//      xc7z_cache.c
//
//      HAL cache control implementation
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
// ####DESCRITPIONBEGIN####
// Contributors: Antmicro <www.antmicro.com>
// Date:         05.2016
// Purpose:      cache control
//
//
// ####DESCRIPTIONEND####

#include <cyg/infra/cyg_type.h>
#include <cyg/hal/hal_cache.h>

#define CR_M                             (1 <<  0) // mmu enable bit
#define CR_C                             (1 <<  2) // dcache enable bit
#define CR_I                             (1 << 12) // icache enable bit

#define CCSIDR_LINE_SIZE_OFFSET          0
#define CCSIDR_LINE_SIZE_MASK            0x7
#define CCSIDR_WAYS_OFFSET               3
#define CCSIDR_WAYS_MASK                 (0x3FF << 3)
#define CCSIDR_SETS_OFFSET               13
#define CCSIDR_SETS_MASK                 (0x7FFF << 13)

// helper functions
static inline cyg_int32 log2(cyg_uint32 n)
{
    cyg_int32 res = -1;
    cyg_uint32 n_cp = n;

    while (n_cp) {
        res++;
        n_cp = n_cp >> 1;
    }

    if (n & (n - 1))
        return res + 1;
    else
        return res;
}

static inline unsigned int get_cr(void)
{
    unsigned int val;
    asm("mrc p15, 0, %0, c1, c0, 0" : "=r" (val) :: "cc");
    return val;
}

static inline void set_cr(unsigned int val)
{
    asm volatile("mcr p15, 0, %0, c1, c0, 0" :: "r" (val) : "cc");
    asm volatile("isb");
}

/* data cache functions */
void hal_dcache_cmd(cyg_uint32 cmd)
{
    cyg_uint32 ccsidr;
    cyg_uint32 num_sets, num_ways, log2_line_len, log2_num_ways;
    cyg_uint32 way_shift;

    int way, set;
    cyg_uint32 setway;

    // Get the cache dimensions
    asm("mrc p15, 1, %0, c0, c0, 0" : "=r" (ccsidr));

    // Extract the number of lines
    log2_line_len = (ccsidr & CCSIDR_LINE_SIZE_MASK) + 2;
    // convert to bytes
    log2_line_len += 2;

    // Extract the number of ways
    num_ways = ((ccsidr & CCSIDR_WAYS_MASK) >> CCSIDR_WAYS_OFFSET) + 1;

    // Extract the number of ways
    num_sets = ((ccsidr & CCSIDR_SETS_MASK) >> CCSIDR_SETS_OFFSET) + 1;

    // Calculate shift
    log2_num_ways = log2(num_ways);
    way_shift = (32 - log2_num_ways);

    // Perform given command on data
    for (way = num_ways - 1; way >= 0 ; way--) {
        for (set = num_sets - 1; set >= 0; set--) {
            setway = (set << log2_line_len) | (way << way_shift);
            switch(cmd) {
            case C_CMD_FLUSH:
            case C_CMD_SYNC:
            case C_CMD_STORE:
                asm volatile("mcr p15, 0, %0, c7, c14, 2" :: "r" (setway));
                break;
            case C_CMD_INVAL:
                asm volatile("mcr p15, 0, %0, c7, c6, 1" :: "r" (setway));
                break;
            }
        }
    }

    // Make sure the operation has completed
    asm volatile("dsb");
}

void hal_dcache_cmd_reg(cyg_uint32 cmd, volatile cyg_uint32 base,
                        cyg_uint32 size)
{
    cyg_uint32 addr = base;

    for (; addr < (base + size); addr += HAL_DCACHE_LINE_SIZE) {
        switch(cmd) {
        case C_CMD_FLUSH:
        case C_CMD_SYNC:
        case C_CMD_STORE:
            asm volatile("mcr p15, 0, %0, c7, c14, 2" :: "r" (addr));
            break;
        case C_CMD_INVAL:
            asm volatile("mcr p15, 0, %0, c7, c6, 1" :: "r" (addr));
            break;
        }
    }
}

void hal_dcache_enable(void)
{
    cyg_uint32 reg;

    reg = get_cr();

    set_cr(reg | (CR_C | CR_M));
}

void hal_dcache_disable(void)
{
    cyg_uint32 reg;

    reg = get_cr();

    // Check if it is really needed
    if ((reg & CR_C) != CR_C)
        return;

    hal_dcache_cmd(C_CMD_FLUSH);
    set_cr(reg & ~(CR_C | CR_M));
}

int hal_dcache_status(void)
{
    return (get_cr() & CR_C) != 0;
}

// instruction cache functions
void hal_icache_invalidate(void)
{
    asm volatile("mcr p15, 0, %0, c7, c5, 0" :: "r" (0));
    asm volatile("mcr p15, 0, %0, c7, c5, 6" :: "r" (0));
    asm volatile("dsb");
    asm volatile("isb");
}

void hal_icache_enable(void)
{
    cyg_uint32 reg;

    reg = get_cr();

    set_cr(reg | CR_I);
}

void hal_icache_disable(void)
{
    cyg_uint32 reg;

    reg = get_cr();

    set_cr(reg & ~CR_I);
}

int hal_icache_status(void)
{
    return (get_cr() & CR_I) != 0;
}
// End of xc7z_cache.c
