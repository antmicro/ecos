#ifndef CYGONCE_VAR_SMP_H
#define CYGONCE_VAR_SMP_H

//=============================================================================
//
//      var_smp.h
//
//      SMP support
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
// Author(s):   Mike Jones <mike@proclivis.com>
// Contributors:  nickg
// Date:        2013-08-08
// Purpose:     Define SMP support abstractions
// Usage:       #include <cyg/hal/hal_smp.h>
//              
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGPKG_HAL_SMP_SUPPORT

#include <cyg/infra/cyg_type.h>

#include <cyg/hal/hal_arch.h>

#include <cyg/hal/var_spinlock.h>

//=============================================================================

//-----------------------------------------------------------------------------
// CPU numbering macros

#define HAL_SMP_CPU_TYPE        cyg_int32

#define HAL_SMP_CPU_MAX         CYGPKG_HAL_SMP_CPU_MAX

#define HAL_SMP_CPU_COUNT()     CYGPKG_HAL_SMP_CPU_MAX

#define HAL_SMP_CPU_NONE        (CYGPKG_HAL_SMP_CPU_MAX+1)

__externC HAL_SMP_CPU_TYPE cyg_hal_smp_cpu(void);

#define HAL_SMP_CPU_THIS()      cyg_hal_smp_cpu()

//-----------------------------------------------------------------------------
// SMP startup

__externC void cyg_hal_smp_init(void);

//-----------------------------------------------------------------------------
// CPU startup

__externC void cyg_hal_smp_cpu_start_all(void);
__externC void cyg_hal_cpu_start(HAL_SMP_CPU_TYPE cpu);
__externC void cyg_hal_smp_cpu_start_first(void);

#define HAL_SMP_CPU_START( __cpu ) cyg_hal_cpu_start( __cpu );

#define HAL_SMP_CPU_RESCHEDULE_INTERRUPT( __cpu, __wait ) \
        cyg_hal_cpu_message( __cpu, HAL_SMP_MESSAGE_RESCHEDULE, 0, __wait);

#define HAL_SMP_CPU_TIMESLICE_INTERRUPT( __cpu, __wait ) \
        cyg_hal_cpu_message( __cpu, HAL_SMP_MESSAGE_TIMESLICE, 0, __wait);

//-----------------------------------------------------------------------------
// CPU message exchange

__externC void cyg_hal_cpu_message( HAL_SMP_CPU_TYPE cpu,
                                    CYG_WORD32 msg,
                                    CYG_WORD32 arg,
                                    CYG_WORD32 wait);

#define HAL_SMP_MESSAGE_TYPE            0xF0000000
#define HAL_SMP_MESSAGE_ARG             (~HAL_SMP_MESSAGE_TYPE)

#define HAL_SMP_MESSAGE_RESCHEDULE      0x10000000
#define HAL_SMP_MESSAGE_MASK            0x20000000
#define HAL_SMP_MESSAGE_UNMASK          0x30000000
#define HAL_SMP_MESSAGE_REVECTOR        0x40000000
#define HAL_SMP_MESSAGE_TIMESLICE       0x50000000

//-----------------------------------------------------------------------------
// Spinlock support.

// The double pointer is not efficient, but allows the hal to initialize
// the struct from a static.

#define HAL_SPINLOCK_TYPE       spinlock_t

__externC void hal_spinlock_spin(spinlock_t *lock);
__externC cyg_bool hal_spinlock_tryspin(spinlock_t *lock);
__externC void hal_spinlock_clear(spinlock_t *lock);
__externC void hal_spinlock_init_clear(spinlock_t *lock);
__externC void hal_spinlock_init_set(spinlock_t *lock);
__externC cyg_bool hal_spinlock_try(spinlock_t *lock);
//__externC void hal_spinlock_test(HAL_SPINLOCK_TYPE lock, cyg_bool result);

#define HAL_SPINLOCK_CLEAR( _lock_ ) hal_spinlock_clear(&_lock_)
#define HAL_SPINLOCK_SPIN( _lock_ ) hal_spinlock_spin(&_lock_)
#define HAL_SPINLOCK_TRYSPIN( _lock_ ) hal_spinlock_tryspin(&_lock_)
#define HAL_SPINLOCK_INIT_CLEAR( _lock_ ) hal_spinlock_init_clear(&_lock_)
#define HAL_SPINLOCK_INIT_SET( _lock_ ) hal_spinlock_init_set(&_lock_)
#define HAL_SPINLOCK_SET( _lock_ ) hal_spinlock_spin(&_lock_)
#define HAL_SPINLOCK_TRY( _lock_, _val_ ) _val_ = hal_spinlock_try(&_lock_)
//#define HAL_SPINLOCK_TEST( _lock_, _val_ )      
//    (_val_) = (((_lock_.owner) & kLocked) != 0)
#define HAL_SPINLOCK_TEST( _lock_, _val_ )      \
    _val_ = hal_spinlock_is_locked(&_lock_)

//-----------------------------------------------------------------------------

#endif // CYGPKG_HAL_SMP_SUPPORT

//-----------------------------------------------------------------------------
#endif // CYGONCE_HAL_SMP_H
// End of hal_smp.h
