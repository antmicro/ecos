//==========================================================================
//
//      xc7z_smp.c
//
//      HAL SMP implementation
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    nickg
// Contributors: nickg, Antmicro Ltd
// Date:         2001-08-03
// Purpose:      HAL SMP implementation
// Description:  This file contains SMP support functions.
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/var_intr.h>
#include <cyg/hal/xc7z_xslcr.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_cache.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memcpy */

__externC HAL_SMP_CPU_TYPE cyg_hal_smp_cpu(void) {
    return hal_cpu_get_current();
}

/* SMP message buffers.
 * SMP CPUs pass messages to eachother via a small circular buffer
 * protected by a spinlock. Each message is a single 32 bit word with
 * a type code in the top 4 bits and any argument in the remaining
 * 28 bits. */
#define SMP_MSGBUF_SIZE 4
static struct smp_msg_t {
    spinlock_t                  lock;           // protecting spinlock
    volatile CYG_WORD32         msgs[SMP_MSGBUF_SIZE]; // message buffer
    volatile CYG_WORD32         head;           // head of list
    volatile CYG_WORD32         tail;           // tail of list
    volatile CYG_WORD32         reschedule;     // reschedule request
    volatile CYG_WORD32         timeslice;      // timeslice request
    volatile CYG_WORD32         reschedule_count;  // number of reschedules
    volatile CYG_WORD32         timeslice_count;   // number of timeslices
} smp_msg[HAL_SMP_CPU_MAX];

static CYG_WORD32 cyg_hal_smp_cpu_running[HAL_SMP_CPU_MAX];

__externC void hal_scu_enable(void);
__externC void hal_scu_join_smp(void);

__externC void cyg_hal_smp_init(void) {
    cyg_int32 i;

    /* Enable snoop control unit */
    hal_scu_enable();

    for (i=0; i < HAL_SMP_CPU_MAX; i++)
        hal_spinlock_init(&(smp_msg[i].lock));
}

__externC void cyg_hal_cpu_message(HAL_SMP_CPU_TYPE cpu,
                                   CYG_WORD32 msg,
                                   CYG_WORD32 arg,
                                   CYG_WORD32 wait) {
    struct smp_msg_t *m = &smp_msg[cpu];
    CYG_INTERRUPT_STATE old_ints;

    /* This only works because we are assigning the vector by CPU number */
    HAL_DISABLE_INTERRUPTS(old_ints);

    /* Get access to the message buffer for the selected CPU */
    HAL_SPINLOCK_SPIN(m->lock);

    if (msg == HAL_SMP_MESSAGE_RESCHEDULE) {
        m->reschedule = true;
    } else if (msg == HAL_SMP_MESSAGE_TIMESLICE) {
        m->timeslice = true;
    } else {
        CYG_WORD32 next = (m->tail + 1) & (SMP_MSGBUF_SIZE-1);

        /* If the buffer is full, wait for space to appear in it.
         * This should only need to be done very rarely. */
        while (next == m->head)
        {
            HAL_SPINLOCK_CLEAR(m->lock);
            HAL_SPINLOCK_SPIN(m->lock);
        }

        m->msgs[m->tail] = msg | arg;

        m->tail = next;
    }

    /* Now send an interrupt to the CPU */
    if (cyg_hal_smp_cpu_running[cpu]) {
        cyg_uint32 sgir = 0x0;

        /* Set target list */
        sgir |= ((0x1 << cpu) << 16);

        /* Set vector */
        sgir |= (CYGNUM_HAL_SMP_CPU_INTERRUPT_VECTOR(cpu) & 0xF);

        /* Send the interrupt */
        HAL_WRITE_UINT32(XC7Z_ICD_SGIR_BASEADDR, sgir);

    }

    HAL_SPINLOCK_CLEAR(m->lock);

    /* If we are expected to wait for the command to complete, then
     * spin here until it does. We actually wait for the destination
     * CPU to empty its input buffer. So we might wait for messages
     * from other CPUs as well. But this is benign. */
    while (wait) {
        HAL_SPINLOCK_SPIN( m->lock );

        if (m->head == m->tail)
            wait = false;

        HAL_SPINLOCK_CLEAR(m->lock);
    }

    HAL_RESTORE_INTERRUPTS( old_ints );
}

__externC CYG_WORD32 cyg_hal_cpu_message_isr(CYG_WORD32 vector, CYG_ADDRWORD data) {
    struct smp_msg_t *m;

    m = &smp_msg[HAL_SMP_CPU_THIS()];

    cyg_drv_interrupt_mask(vector);
    HAL_SPINLOCK_SPIN( m->lock );

    cyg_drv_interrupt_acknowledge(vector);

    CYG_WORD32 ret = 1;

    if (m->reschedule)
        m->reschedule_count++;
    if (m->timeslice)
        m->timeslice_count++;
    if (m->reschedule || m->timeslice)
        ret |= 2; /* Call DSR */

    while (m->head != m->tail) {
        CYG_WORD32 msg = m->msgs[m->head];

        switch (msg & HAL_SMP_MESSAGE_TYPE) {
        case HAL_SMP_MESSAGE_RESCHEDULE:
            ret |= 2; /* Call DSR */
            break;
        case HAL_SMP_MESSAGE_MASK:
        case HAL_SMP_MESSAGE_UNMASK:
        case HAL_SMP_MESSAGE_REVECTOR:
            break;
        }

        /* Update the head pointer after handling the message, so that
         * the wait in cyg_hal_cpu_message() completes after the action
         * requested. */
        m->head = (m->head + 1) & (SMP_MSGBUF_SIZE-1);
    }
    HAL_SPINLOCK_CLEAR(m->lock);

    cyg_drv_interrupt_unmask(vector);

    return ret;
}

__externC void cyg_scheduler_set_need_reschedule(void);
__externC void cyg_scheduler_timeslice_cpu(void);

__externC CYG_WORD32 cyg_hal_cpu_message_dsr(CYG_WORD32 vector, CYG_ADDRWORD data) {

    struct smp_msg_t *m = &smp_msg[HAL_SMP_CPU_THIS()];
    CYG_WORD32 reschedule;

#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE
    CYG_WORD32 timeslice;
#endif

    cyg_drv_interrupt_mask(vector);
    HAL_SPINLOCK_SPIN(m->lock);

    reschedule = m->reschedule;
#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE
    timeslice = m->timeslice;
    m->reschedule = m->timeslice = false;
#else
    m->reschedule = false;
#endif

    HAL_SPINLOCK_CLEAR(m->lock);
    cyg_drv_interrupt_unmask(vector);

    if (reschedule) {
        cyg_scheduler_set_need_reschedule();
    }
#ifdef CYGSEM_KERNEL_SCHED_TIMESLICE
    if (timeslice) {
        cyg_scheduler_timeslice_cpu();
    }
#endif

    return 0;
}

static void zynq_slcr_lock(void) {
    HAL_WRITE_UINT32(XSLR_BASE + XSLR_SLCR_LOCK, XSLCR_LOCK_KEY);
}

static void zynq_slcr_unlock(void) {
    HAL_WRITE_UINT32(XSLR_BASE + XSLR_SLCR_UNLOCK, XSLCR_UNLOCK_KEY);
}

static cyg_uint32 zynq_slcr_read(cyg_uint32 reg) {
    cyg_uint32 value;
    HAL_READ_UINT32(XSLR_BASE + reg, value);
    return value;
}

static void zynq_slcr_write(cyg_uint32 reg, cyg_uint32 value) {
    HAL_WRITE_UINT32(XSLR_BASE + reg, value);
}

void zynq_cpu_stop(cyg_uint32 cpu) {
    cyg_uint32 reg;

    zynq_slcr_unlock();

    reg = zynq_slcr_read(XSLCR_A9_CPU_RST_CTRL_OFFSET);

    reg |= (XSLCR_A9_CPU_STOP | XSLCR_A9_CPU_RST) << cpu;
    zynq_slcr_write(XSLCR_A9_CPU_RST_CTRL_OFFSET, reg);

    zynq_slcr_lock();
}

static void zynq_cpu_start(cyg_uint32 cpu) {
    cyg_uint32 reg;

    zynq_slcr_unlock();

    reg = zynq_slcr_read(XSLCR_A9_CPU_RST_CTRL_OFFSET);

    reg &= ~(XSLCR_A9_CPU_RST << cpu);
    zynq_slcr_write(XSLCR_A9_CPU_RST_CTRL_OFFSET, reg);

    reg &= ~(XSLCR_A9_CPU_STOP << cpu);
    zynq_slcr_write(XSLCR_A9_CPU_RST_CTRL_OFFSET, reg);

    zynq_slcr_lock();
}

__externC void zynq_secondary_trampoline(void);
__externC void zynq_secondary_trampoline_jump(void);

__externC void cyg_kernel_smp_startup(void);
__externC void hal_interrupt_init_cpu(void);

__externC void hal_delay_us(cyg_int32);

static void cyg_hal_smp_start_secondary_cpu(void) {
    hal_interrupt_init_cpu();
    cyg_hal_smp_cpu_running[hal_cpu_get_current()] = 1;
    cyg_kernel_smp_startup();
}

__externC void cyg_hal_smp_cpu_start_first(void) {
    cyg_hal_cpu_start(0);
}

/* Starting another CPUs */
__externC void cyg_hal_cpu_start(HAL_SMP_CPU_TYPE cpu) {
    if (cyg_hal_smp_cpu_running[cpu] == 1)
        return;

    if (cpu == 0) {
        cyg_hal_smp_cpu_running[cpu] = 1;
        hal_scu_join_smp();
    } else {
        hal_delay_us(100);
        /* Flush cache */
        HAL_DCACHE_INVALIDATE_ALL();
        HAL_ICACHE_INVALIDATE_ALL();

        HAL_DCACHE_SYNC();
        HAL_ICACHE_SYNC();

        zynq_cpu_stop(cpu);
        cyg_uint32 trampoline_size =
            (cyg_uint32)&zynq_secondary_trampoline_jump -
            (cyg_uint32)&zynq_secondary_trampoline;

        memcpy(0x0, &zynq_secondary_trampoline, trampoline_size);
        HAL_WRITE_UINT32(0x0 + trampoline_size,
                         (cyg_uint32)&cyg_hal_smp_start_secondary_cpu);
        zynq_cpu_start(cpu);
    }
}
