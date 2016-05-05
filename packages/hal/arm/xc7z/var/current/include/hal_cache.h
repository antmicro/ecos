#ifndef CYGONCE_HAL_CACHE_H
#define CYGONCE_HAL_CACHE_H

//=============================================================================
//
//      hal_cache.h
//
//      HAL cache control API
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
#include <cyg/infra/cyg_ass.h>

#define HAL_DCACHE_SIZE          32768
#define HAL_DCACHE_LINE_SIZE     32
#define HAL_DCACHE_WAYS          4
#define HAL_DCACHE_SETS          256

#define C_CMD_FLUSH         0
#define C_CMD_SYNC          1
#define C_CMD_STORE         2
#define C_CMD_INVAL         3

// Function prototypes
void hal_dcache_enable(void);
void hal_dcache_disable(void);
int hal_dcache_status(void);
void hal_dcache_cmd(cyg_uint32);
void hal_dcache_cmd_reg(cyg_uint32, volatile cyg_uint32, cyg_uint32);

void hal_icache_enable(void);
void hal_icache_disable(void);
void hal_icache_invalidate(void);
int hal_icache_status(void);
// End of prototypes

#define HAL_DCACHE_ENABLE()                hal_dcache_enable()
#define HAL_DCACHE_DISABLE()               hal_dcache_disable()
#define HAL_DCACHE_IS_ENABLED(state)       state = hal_dcache_status()

#define HAL_DCACHE_INVALIDATE_ALL_DEFINED
#define HAL_DCACHE_INVALIDATE_ALL()        hal_dcache_cmd(C_CMD_INVAL)

#define HAL_DCACHE_INVALIDATE(addr, size)  hal_dcache_cmd_reg(C_CMD_INVAL, \
                                                              addr, size)

#define HAL_DCACHE_SYNC_DEFINED
#define HAL_DCACHE_SYNC()                  hal_dcache_cmd(C_CMD_SYNC)

#define HAL_DCACHE_STORE_DEFINED
#define HAL_DCACHE_STORE(addr, size)       hal_dcache_cmd_reg(C_CMD_STORE, \
                                                              addr, size)

#define HAL_DCACHE_FLUSH_DEFINED
#define HAL_DCACHE_FLUSH(addr, size)       hal_dcache_cmd_reg(C_CMD_FLUSH, \
                                                              addr, size)

#define HAL_ICACHE_IS_ENABLED(state)       state = hal_icache_status()

#define HAL_ICACHE_ENABLE()                hal_icache_enable()
#define HAL_ICACHE_DISABLE()               hal_icache_disable()

#define HAL_ICACHE_INVALIDATE_ALL()        hal_icache_invalidate()

#endif // ifndef CYGONCE_HAL_CACHE_H
// End of hal_cache.h
