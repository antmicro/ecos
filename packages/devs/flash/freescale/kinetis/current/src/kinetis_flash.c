//=================================================================
//
//        kinetis_flash.c
//
//        kinetis internal flash driver
//
//=================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2012 Free Software Foundation, Inc.
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     Nicolas Aujoux
// Date:         2012-03-19
//
//####DESCRIPTIONEND####


#include <pkgconf/devs_flash_kinetis.h>
#include <pkgconf/hal_cortexm_kinetis.h>

#include <cyg/io/kinetis_flash.h>

#include <cyg/io/flash.h>
#include <cyg/io/flash_dev.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>

#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_if.h>
#include <cyg/hal/hal_tables.h>

// Include memory access routines
#include <cyg/hal/hal_io.h>

// For tests
#include <cyg/infra/testcase.h>

#include <string.h>

#include CYGHWR_MEMORY_LAYOUT_H

#define LONGWORD_SIZE (CYGNUM_DEVS_KINETIS_FLASH_LONGWORD_SIZE)

// Flash command parameters
#define FLASH_ERASE_BLOCK           0x08
#define FLASH_ERASE_ALL_BLOCKS      0X44
#define FLASH_ERASE_SECTOR          0x09
#define FLASH_PROGRAM_LONGWORD      0x06
#define FLASH_PROGRAM_PHRASE        0x07

cyg_uint32 defaultRegisterPFB0CR, defaultRegisterPFB1CR;

static int __attribute__((__long_call__)) flash_command_sequence(void)
        __attribute__((section(".2ram.flashing")));

static void __attribute__((__long_call__)) enable_flash_acceleration (void)
            __attribute__((section(".2ram.flashing")));
static void __attribute__((__long_call__)) disable_flash_acceleration (void)
            __attribute__((section(".2ram.flashing")));

static void __attribute__((__long_call__)) cache_on(cyg_uint32 *cachestate) __attribute__((section(".2ram.flashing")));
static void __attribute__((__long_call__)) cache_off(cyg_uint32 *cachestate) __attribute__((section(".2ram.flashing")));

// ===================================================================
// Flash API functions
// ===================================================================

static int kinetis_flash_init (struct cyg_flash_dev* dev)
{
    HAL_READ_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB0CR, defaultRegisterPFB0CR);
    HAL_READ_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB1CR, defaultRegisterPFB1CR);

    // SIM_SCGC6: FTFL=1
    CYGHWR_HAL_KINETIS_SIM_P->scgc6 |= CYGHWR_HAL_KINETIS_SIM_SCGC6_FTFL_M;

    return CYG_FLASH_ERR_OK;
}

static size_t kinetis_flash_query (struct cyg_flash_dev* dev,
                          void* data, size_t len)
{
    static const char query[] = "Kinetis Flash";
    if (sizeof(query) < len)
        len = sizeof(query);
    memcpy( data, query, len);
    return sizeof(query);
}

static int kinetis_flash_erase_sector (struct cyg_flash_dev* dev,
                                cyg_flashaddr_t block_base)
{
    cyg_uint32 cachestate;
    cyghwr_hal_kinetis_flash_t* flashRegister = CYGHWR_HAL_KINETIS_FLASH_P;
    int returnValue = CYG_FLASH_ERR_OK;

    // Check if the block_base is sector aligned
    if((block_base % CYGNUM_DEVS_KINETIS_FLASH_BLOCK_SIZE) != 0)
        return CYG_FLASH_ERR_INVALID;

    // Check for valid range of the address
    if(block_base < dev->start || (block_base +
        CYGNUM_DEVS_KINETIS_FLASH_BLOCK_SIZE - 1) > dev->end)
        return CYG_FLASH_ERR_INVALID;

    cache_off(&cachestate);

    // Create parameters to erase a flash sector
    flashRegister->fccob0 = FLASH_ERASE_SECTOR;
    flashRegister->fccob1 = (cyg_uint8)(block_base >> 16);
    flashRegister->fccob2 = (cyg_uint8)((block_base >> 8) & 0xFF);
    flashRegister->fccob3 = (cyg_uint8)(block_base & 0xFF);
    returnValue = flash_command_sequence();

    cache_on(&cachestate);

    return returnValue;
}

static int kinetis_flash_program (struct cyg_flash_dev* dev,
                            cyg_flashaddr_t _base,
                            const void* _data,
                            size_t _len)
{
    cyghwr_hal_kinetis_flash_t* flashRegister = CYGHWR_HAL_KINETIS_FLASH_P;
    cyg_uint32 endAddress;
    int returnValue = CYG_FLASH_ERR_OK;
    cyg_flashaddr_t base = _base;
    const cyg_uint32* data = _data;
    size_t len = _len;
    cyg_uint32 currentData;
    cyg_uint8 fccob0;
    cyg_uint32 cachestate;

    endAddress = base + len;

    // Base and data have to be 16-bit aligned
    if( (((CYG_ADDRESS)base & 1) != 0) || (((CYG_ADDRESS)data  & 1) != 0))
        return CYG_FLASH_ERR_INVALID;

    // Check for valid range of the addresse
    if((base < dev->start) || ((endAddress - 1) > dev->end))
        return CYG_FLASH_ERR_INVALID;

    cache_off(&cachestate);

    while(len > 0)
    {
        flashRegister->fccob0 = fccob0 = (4 == LONGWORD_SIZE) ?
                                         FLASH_PROGRAM_LONGWORD :
                                         FLASH_PROGRAM_PHRASE;
        flashRegister->fccob1 = (cyg_uint8)((base >> 16) & 0xFF);
        flashRegister->fccob2 = (cyg_uint8)((base >> 8) & 0xFF);
        flashRegister->fccob3 = (cyg_uint8)(base & 0xFF);

        currentData = *data++;
        flashRegister->fccob4 = (cyg_uint8)((currentData >> 24) & 0xFF);
        flashRegister->fccob5 = (cyg_uint8)((currentData >> 16) & 0xFF);
        flashRegister->fccob6 = (cyg_uint8)((currentData >> 8) & 0xFF);
        flashRegister->fccob7 = (cyg_uint8)((currentData) & 0xFF);

        if(FLASH_PROGRAM_PHRASE == fccob0)
        {
            currentData = *data++;
            flashRegister->fccob8 = (cyg_uint8)((currentData >> 24) & 0xFF);
            flashRegister->fccob9 = (cyg_uint8)((currentData >> 16) & 0xFF);
            flashRegister->fccobA = (cyg_uint8)((currentData >> 8) & 0xFF);
            flashRegister->fccobB = (cyg_uint8)((currentData) & 0xFF);
        }

        returnValue = flash_command_sequence();
        if(returnValue != CYG_FLASH_ERR_OK)
        {
            break;
        }
        else
        {
            // Compute the next destination address
            base += LONGWORD_SIZE;

            // Compute size for the next command
            len -= LONGWORD_SIZE;
        }
    }

    cache_on(&cachestate);

    return returnValue;
}

static int __attribute__((__long_call__)) flash_command_sequence(void)
{
    cyghwr_hal_kinetis_flash_t* flashRegister = CYGHWR_HAL_KINETIS_FLASH_P;
    cyg_uint8 registerValue;
    int returnValue;

    while((flashRegister->fstat & CYGHWR_HAL_KINETIS_FLASH_FSTAT_CCIF_M)
        == false)
    {
        // We wait for the end of the previous command ->CCIF bit=0 when finished
    }

    // clear RDCOLERR & ACCERR & FPVIOL flag in flash status register
    flashRegister->fstat = (CYGHWR_HAL_KINETIS_FLASH_FSTAT_RDCOLERR_M |
                            CYGHWR_HAL_KINETIS_FLASH_FSTAT_ACCERR_M |
                            CYGHWR_HAL_KINETIS_FLASH_FSTAT_FPVIOL_M);

    // Load FFCOB registers with command and paramaters for flash operations
    // We clear the CCIF bit to start the command
    HAL_REORDER_BARRIER();
    flashRegister->fstat = CYGHWR_HAL_KINETIS_FLASH_FSTAT_CCIF_M;

    // We wait the end of the process
    while((flashRegister->fstat & CYGHWR_HAL_KINETIS_FLASH_FSTAT_CCIF_M)
        == false){}

    // Check if any error happened
    // First we read the flash status register
    registerValue = flashRegister->fstat;

    // Access error
    if((registerValue & CYGHWR_HAL_KINETIS_FLASH_FSTAT_ACCERR_M) != 0)
        returnValue = CYG_FLASH_ERR_PROGRAM;
    // Protection error
    else if((registerValue & CYGHWR_HAL_KINETIS_FLASH_FSTAT_FPVIOL_M) != 0)
        returnValue = CYG_FLASH_ERR_PROTECT;
    // MGSTAT0 non-correctable error
    else if ((registerValue & CYGHWR_HAL_KINETIS_FLASH_FSTAT_MGSTAT0_M) != 0)
        returnValue = CYG_FLASH_ERR_PROTOCOL;
    // No error detected
    else
        returnValue = CYG_FLASH_ERR_OK;

    return returnValue;
}

#if 0

static void kinetis_flash_read(struct cyg_flash_dev *dev,
                        const cyg_flashaddr_t base,
                        void *data,
                        size_t len)
{
    memcpy(data, (void*)base, len);
}

# define KINETIS_FLASH_READ

#endif

// ===================================================================
// Flash accelerator and cache control functions
// ===================================================================


static void __attribute__((__long_call__)) enable_flash_acceleration ()
{
    HAL_WRITE_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB0CR, defaultRegisterPFB0CR);
    HAL_WRITE_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB1CR, defaultRegisterPFB1CR);
    __asm__ volatile( "dsb" );
    __asm__ volatile( "isb" );
}

static void __attribute__((__long_call__)) disable_flash_acceleration ()
{
    cyg_uint32 registerValue;

    // Flash Bank 0 register
    HAL_READ_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB0CR, registerValue);
    // We disable data and instruction prefetch
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BIPE;
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BDPE;
    // We disable data and instruction cache and single entry buffer
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BSEBE;
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BICE;
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BDCE;
    HAL_WRITE_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB0CR, registerValue);
    __asm__ volatile( "dsb" );
    __asm__ volatile( "isb" );

    // Flash Bank 1 register
    HAL_READ_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB1CR, registerValue);
    // We disable data and instruction prefetch
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BIPE;
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BDPE;
    // We disable data and instruction cache and single entry buffer
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BSEBE;
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BICE;
    registerValue &= ~CYGHWR_HAL_KINETIS_FMC_PFBCR_BDCE;
    HAL_WRITE_UINT32(CYGHWR_HAL_KINETIS_FMC_PFB1CR, registerValue);
    __asm__ volatile( "dsb" );
    __asm__ volatile( "isb" );
}

static void __attribute__((__long_call__))
cache_off( cyg_uint32 *cachestate ) {
    cyg_uint32 intstate, dcachestate, icachestate;

    HAL_DISABLE_INTERRUPTS(intstate); 
    HAL_DCACHE_IS_ENABLED(dcachestate);
    HAL_ICACHE_IS_ENABLED(icachestate);
    *cachestate = (dcachestate ? 1 : 0) | (icachestate ? 2 : 0);
    if (dcachestate) {
        HAL_DCACHE_SYNC();
        HAL_DCACHE_DISABLE();
    }
    if (icachestate) {
        HAL_ICACHE_DISABLE();
    }
#ifdef CYGNUM_DEVS_KINETIS_FLASH_LOGIC_ERROR_BUG
    disable_flash_acceleration();
#endif
    HAL_RESTORE_INTERRUPTS(intstate);
}

static void __attribute__((__long_call__))
cache_on( cyg_uint32 *cachestate ) {
#ifdef CYGNUM_DEVS_KINETIS_FLASH_LOGIC_ERROR_BUG
    enable_flash_acceleration();
#endif
    if (*cachestate & 1)
        HAL_DCACHE_ENABLE();
    if (*cachestate & 2)
        HAL_ICACHE_ENABLE();
}

// ===================================================================
// Function table
// ===================================================================

const CYG_FLASH_FUNS (
                cyg_kinetis_flash_funs,
                &kinetis_flash_init,
                &kinetis_flash_query,
                &kinetis_flash_erase_sector,
                &kinetis_flash_program,
#ifdef KINETIS_FLASH_READ
                &kinetis_flash_read,
#else
                NULL,
#endif
                cyg_flash_devfn_lock_nop,
                cyg_flash_devfn_unlock_nop);

// Add the flash driver to the HAL TABLE cyg_flashdev

cyg_kinetis_flash_dev hal_kinetis_flash_priv;
static const cyg_flash_block_info_t cyg_flash_kinetis_block_info[1] = {{
    CYGNUM_DEVS_KINETIS_FLASH_BLOCK_SIZE,
    (CYGMEM_REGION_flash_SIZE) / CYGNUM_DEVS_KINETIS_FLASH_BLOCK_SIZE }};

CYG_FLASH_DRIVER(hal_kinetis_flash,
                 &cyg_kinetis_flash_funs,
                 0,
                 CYGMEM_REGION_flash,
                 (CYGMEM_REGION_flash + CYGMEM_REGION_flash_SIZE - 1),
                 1,                             //number of block info
                 cyg_flash_kinetis_block_info,
                 &hal_kinetis_flash_priv);

// ----------------------------------------------------------------------------
// End of kinetis_flash.c
