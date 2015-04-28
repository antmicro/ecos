//==========================================================================
//
//      i2c_freescale.c
//
//      I2C driver for Freescale Kinetis and ColdFire+ microcontrollers
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2011, 2013 Free Software Foundation, Inc.
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
// Author(s):     Tomas Frydrych <tomas@sleepfive.com>
// Contributors:  Mike Jones <mike@proclivis.com>
//                Ilija Kocho <ilijak@siva.com.mk>
// Date:          2011-11-20
// Original:      ccoutand
//                I2C driver for Stellaris Cortex M3 microcontroller
// Description:   Driver for Freescale I2C controller
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/system.h>

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/cyg_ass.h>
#include <cyg/infra/diag.h>
#define CYGBLD_I2C_DEVICE_CONST
#include <cyg/io/i2c.h>
#include <cyg/io/i2c_freescale.h>
#include <cyg/hal/hal_arch.h>
#include <cyg/hal/hal_io.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>
#include <cyg/kernel/kapi.h>

#ifdef CYGPKG_DEVS_I2C_FREESCALE_I2C_TRACE
# define I2C_TRACE(args...) diag_printf(args)
#else
# define I2C_TRACE(args...)
#endif

#ifndef CYGNUM_DEV_I2C_SPIN_PRIO
#define CYGNUM_DEV_I2C_SPIN_PRIO (CYGNUM_KERNEL_SCHED_PRIORITIES - 2)
#endif

#ifndef CYGNUM_DEV_I2C_SPIN_TIMEOUT
#define CYGNUM_DEV_I2C_SPIN_TIMEOUT 30
#endif

#define CYGOPT_FREESCALE_I2C_FREQ_FAST_EXIT 0

#define I2C_STOP(__i2c)                   \
CYG_MACRO_START                           \
    __i2c->c1 &= ~FREESCALE_I2C_C1_MST_M; \
    __i2c->c1 &= ~FREESCALE_I2C_C1_TX_M;  \
CYG_MACRO_END

#define I2C_START(__i2c)                 \
CYG_MACRO_START                          \
    __i2c->c1 |= FREESCALE_I2C_C1_TX_M;  \
    __i2c->c1 |= FREESCALE_I2C_C1_MST_M; \
CYG_MACRO_END

#define I2C_REPEAT_START(__i2c)           \
CYG_MACRO_START                           \
    __i2c->c1 |= FREESCALE_I2C_C1_RSTA_M; \
CYG_MACRO_END


#define I2C_CLEAR(__i2c, __mask) \
CYG_MACRO_START                  \
    __i2c->s |= __mask;          \
CYG_MACRO_END

// Clock rate calculator

#define I2C_DEBUG_CLOCK 0

#define I2C_CLOCK_NORM 1000
#define I2C_CLOCK_NORMLL 1000LL

// dev_i2c_freescale_freq_table[] entries and
// dev_i2c_freescale_frequency_entry_t (typedef of cyg_uint16 or cyg_uint32)
// type are provided be provided by HAL
static const dev_i2c_freescale_frequency_entry_t dev_i2c_freescale_freq_table[]
                 = { CYGHWR_IO_FREESCALE_I2C_FREQUENCY_TABLE };
static const cyg_uint32 dev_i2c_freescale_freq_table_size =
                 sizeof(dev_i2c_freescale_freq_table) /
                 sizeof(dev_i2c_freescale_freq_table[0]);

// Frequency divider setting calculator
// Calculate F register setting. The input parameter period_sp is encodded:
// If most significant bit (31) is 1, than the least significant byte contains
// explicit F refister value. If most significant bit (31) is 0 then:
//    - bits 27-0: clock period in [ns];
//    - bits 30,29: fit (see CDL)
//    - bit 28:
//          1) aggressive look up - closest matching period
//          0) consrvative look up - closest matching period not shorter than setpoint

static cyg_uint8
freescale_i2c_freq_div_calc(cyg_uint32 period_sp)
{
    cyg_uint32 sys_clk_per;
    cyg_uint32 period, the_period = 0xffffffff, agr;
    cyg_uint32 mult, the_mult = 4;
    cyg_int32 icr_i, the_icr_i = dev_i2c_freescale_freq_table_size - 1;
    cyg_uint16 icr;
    cyg_uint8 i2cf = 0xff;

    cyg_uint32 fit;

#if I2C_DEBUG_CLOCK
        I2C_TRACE("Period setpont: 0x%08x\n", period_sp);
#endif
    // Extract period and fit
    fit = period_sp >> FREESCALE_I2C_DELAY_FIT_S;
    agr = FREEDCALE_I2C_DELAY_IS_AGRESIVE(period_sp);
    period_sp &= FREESCALE_I2C_DELAY_M;
    sys_clk_per = ((1000000000 * I2C_CLOCK_NORMLL) / CYGHWR_IO_I2C_FREESCALE_I2C_CLOCK);
    period_sp *= I2C_CLOCK_NORM;

#if I2C_DEBUG_CLOCK
    I2C_TRACE("Period %d.%d[ns] [0x%08x] fit %d agr 0x%08x\n", period_sp/I2C_CLOCK_NORM,
              period_sp%I2C_CLOCK_NORM, period_sp, fit, agr);
#endif

    for(mult = 4; mult > 0; mult /= 2) {

#if I2C_DEBUG_CLOCK >= 3
        I2C_TRACE("mult = %d icr_n=%d\n", mult, dev_i2c_freescale_freq_table_size);
#endif

        for(icr_i = dev_i2c_freescale_freq_table_size - 1; icr_i >= 0; icr_i--) {
            icr = dev_i2c_freescale_freq_table[icr_i];
            period = sys_clk_per * (icr * mult);

#if I2C_DEBUG_CLOCK >= 3
            I2C_TRACE("sys_clk_per=%d the_mult = %d icr[0x%02x]=%d, period = %d.%d (sp = %d.%d)\n",
                      sys_clk_per, the_mult, icr_i, icr,
                      period/I2C_CLOCK_NORM, period%I2C_CLOCK_NORM,
                      period_sp/I2C_CLOCK_NORM, period_sp%I2C_CLOCK_NORM);
#endif

            if (period == period_sp) {

                I2C_TRACE ("FreqDiv EQ: [0x%02x] the_mult = %d, icr[0x%02x] = %d\n",
                           i2cf, the_mult, the_icr_i, dev_i2c_freescale_freq_table[the_icr_i]);

                return FREESCALE_I2C_F(mult / 2, icr_i);
            } else if (period < period_sp) {
                if(0 == fit) {
                    if(agr && ((period_sp - period) < period - the_period )) {
                        the_mult = mult;
                        the_icr_i = icr_i;
                        the_period = period;
                    }
                    i2cf = FREESCALE_I2C_F(the_mult / 2, the_icr_i);

#ifdef CYGPKG_DEVS_I2C_FREESCALE_I2C_TRACE
                    I2C_TRACE ("FreqDiv LT: [0x%02x] the_mult = %d, icr[0x%02x] = %d\n",
                               i2cf, the_mult, the_icr_i, dev_i2c_freescale_freq_table[the_icr_i]);
                    icr = dev_i2c_freescale_freq_table[the_icr_i];
                    period = (sys_clk_per * (icr * the_mult));
                    I2C_TRACE ("    Actual: period = %d.%d[ns], frequency = %d[Hz]\n",
                               period/I2C_CLOCK_NORM, period%I2C_CLOCK_NORM,
                               (cyg_uint32)((1000000000 * I2C_CLOCK_NORMLL) / period));
#endif

                    return i2cf;
                    break;
                } else {

#if I2C_DEBUG_CLOCK >= 1
                    icr = dev_i2c_freescale_freq_table[the_icr_i];
                    period = (sys_clk_per * (icr * mult));

                    I2C_TRACE ("    Fits left %d: period = %d.%d[ns], frequency = %d[Hz]\n",
                               fit, period/I2C_CLOCK_NORM, period%I2C_CLOCK_NORM,
                               (cyg_uint32)((1000000000 * I2C_CLOCK_NORMLL) / period));
#endif

                    fit --;
                    break;
                }
            } else { // period > period_sp
                the_period = period;
                the_mult = mult;
                the_icr_i = icr_i;
                i2cf = FREESCALE_I2C_F(mult / 2, the_icr_i);

#if I2C_DEBUG_CLOCK >= 2
                I2C_TRACE ("FreqDiv FIT %d: [0x%02x] the_mult = %d, icr[0x%02x] = %d\n", 3-fit,
                           i2cf, the_mult, the_icr_i, dev_i2c_freescale_freq_table[the_icr_i]);
#endif

            }
        }
    }

    I2C_TRACE ("FreqDiv EX: [0x%02x] the_mult = %d, icr[0x%02x] = %d\n",
               i2cf, the_mult, the_icr_i, dev_i2c_freescale_freq_table[the_icr_i]);

#ifdef CYGPKG_DEVS_I2C_FREESCALE_I2C_TRACE
                icr = dev_i2c_freescale_freq_table[the_icr_i];
                period = (sys_clk_per * (icr * the_mult));
                I2C_TRACE ("    Actual: period = %d[ns], frequency = %d[Hz]\n",
                           period, (cyg_uint32)((1000000000 * I2C_CLOCK_NORMLL) / period));
#endif

    return i2cf;
}

// Set frequency divider.
// If frequency divider setting is cached simply return the value else
// calculate one by means of freescale_i2c_freq_div_calc() and cache it.

static inline cyg_uint8
freescale_i2c_freq_div(cyg_uint32 *period_p)
{
    cyg_uint32 period;
    cyg_uint8 freq;

    if((period = *period_p) & FREESCALE_I2C_DELAY_F_IS_CACHED_M) {
#if I2C_DEBUG_CLOCK
        I2C_TRACE("Cached/pre-calculated divider setting: 0x%08x\n", period);
#endif
        freq = period & FREESCALE_I2C_DELAY_F_CACHE_M;
    } else {
        freq = freescale_i2c_freq_div_calc(period);
        *period_p = freq | FREESCALE_I2C_DELAY_F_IS_CACHED_M;
    }
    return freq;
}

// Wait untill the bus is free or timeout expires
// This function performes busy-wait In order to prevent blocking
// other processes it lowers thread priority prior to entering the spin loop.
// After the spin loop the original thread priority is restored.

static bool
wait_bus_ready(freescale_i2c_extra* extra)
{
    freescale_i2c_t *i2c_s;
    bool bus_ready;
    cyg_priority_t i2c_prio;
    cyg_tick_count_t time;
    cyg_handle_t i2c_thread;
#ifdef CYGPKG_DEVS_I2C_FREESCALE_I2C_TRACE
    cyg_priority_t i2c_cur_prio;
    cyg_uint8 __sr;

#endif

    if (!extra->i2c_owner) {
        I2C_TRACE ("I2C bus ready: 0:\n");
        i2c_s = FREESCALE_I2C_P (extra->i2c_base);
        if((
#ifdef CYGPKG_DEVS_I2C_FREESCALE_I2C_TRACE
            __sr =
#endif
            i2c_s->s) & FREESCALE_I2C_S_BUSY_M)
        {
            bus_ready = false;
            // Decrease priority before busy-waiting.
            i2c_thread = cyg_thread_self();
            i2c_prio = cyg_thread_get_priority(i2c_thread);
#ifdef CYGPKG_DEVS_I2C_FREESCALE_I2C_TRACE
            i2c_cur_prio = cyg_thread_get_current_priority(i2c_thread);
#endif
            I2C_TRACE ("I2C bus ready: 1: priority %d, current priority %d\n",
                       i2c_prio, i2c_cur_prio);
            // FIXME: What about current thread priority?
            I2C_TRACE ("I2C bus ready: 2: sr 0x%02x, cr 0x%02x\n", __sr, i2c_s->c1);

            cyg_thread_set_priority(i2c_thread, CYGNUM_DEV_I2C_SPIN_PRIO);

            I2C_TRACE ("I2C bus ready: 3: priority %d, current priority %d\n",
                       cyg_thread_get_priority(i2c_thread),
                       cyg_thread_get_current_priority(i2c_thread));
            for (time = cyg_current_time();
                 (cyg_current_time() - time) < CYGNUM_DEV_I2C_SPIN_TIMEOUT;)
            {
                if(!(i2c_s->s & FREESCALE_I2C_S_BUSY_M)) {
                    bus_ready = true;
                    break;
                }
            }
            cyg_thread_set_priority(i2c_thread, i2c_prio);

            I2C_TRACE ("I2C bus ready: 4: priority %d, current priority %d\n",
                       cyg_thread_get_priority(i2c_thread),
                       cyg_thread_get_current_priority(i2c_thread));
        } else {
            bus_ready = true;
        }
        if(bus_ready) {
            extra->i2c_got_nack = 0;
            extra->i2c_owner = 1;
        }
    } else {
        bus_ready = true;
    }

    I2C_TRACE ("I2C bus ready: Z bus_ready = %d\n", bus_ready);

    return bus_ready;
}

// Interrupt service routine
static cyg_uint32
freescale_i2c_isr(cyg_vector_t vec, cyg_addrword_t data)
{
    freescale_i2c_extra *extra = (freescale_i2c_extra *) data;
    freescale_i2c_t     *i2c_s = FREESCALE_I2C_P (extra->i2c_base);
    cyg_uint8            sr;
    cyg_uint32           result = CYG_ISR_HANDLED;
    cyg_uint8            tx_data = *extra->i2c_data.i2c_tx_data;

    I2C_TRACE ("I2C ISR\n");

    // Read current status
    sr = i2c_s->s;

    // clear interrupt
    i2c_s->s |= FREESCALE_I2C_S_IICIF_M;

    if (FREESCALE_I2C_XFER_MODE_TX == extra->i2c_mode) {
        I2C_TRACE("I2C: TX IRQ handling, sr 0x%02x\n", sr);

        // check for error conditions
        if (sr & FREESCALE_I2C_S_ARBL_M) {
            /*
             * Arbitration lost
             *
             * Lost the bus, abort the transfer. count has already been
             * decremented. Assume the byte did not actually arrive.
             */
            extra->i2c_count += 1;

            I2C_STOP(i2c_s);
            I2C_CLEAR(i2c_s, FREESCALE_I2C_S_ARBL_M);

            result = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
            I2C_TRACE("I2C: TX, bus arbitration error\n");
        } else if (sr & FREESCALE_I2C_S_RXAK_M) {
            /*
             * NACK
             * Transfer failed. Handled upon exit from the dsr.
             */
            extra->i2c_got_nack = 1;
            I2C_STOP(i2c_s);
            result = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
            I2C_TRACE("I2C: TX, got NACK\n");
        } else if (0 == extra->i2c_count) {
            // No more bytes to send.
            I2C_TRACE("I2C: TX, no more bytes\n");

            // Last byte
            if ((0 == extra->i2c_count) && extra->send_stop)
                I2C_STOP(i2c_s);
            result = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
        } else {
            // Send byte
            I2C_TRACE("I2C: TX, sending 0x%02x\n", tx_data);
            i2c_s->d = tx_data;
            extra->i2c_data.i2c_tx_data += 1;
            extra->i2c_count -= 1;

        }
    } else if (FREESCALE_I2C_XFER_MODE_RX == extra->i2c_mode) {
        I2C_TRACE("I2C: RX IRQ handling, sr 0x%02x\n", sr);

        /*
         * check the actual bus mode, RX transaction starts with initial TX
         * to transmit the slave address.
         */
        if (i2c_s->c1 & FREESCALE_I2C_C1_TX_M) {
            /*
             * In TX mode, should be getting an acknowledgement of the slave
             * address write
             */
            if (sr & FREESCALE_I2C_S_ARBL_M) {
                // Arbitration  lost, stop
                I2C_STOP(i2c_s);
                I2C_CLEAR(i2c_s, FREESCALE_I2C_S_ARBL_M);

                result = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
                I2C_TRACE("I2C: RX, initial bus arbitration error\n");
            } else if (sr & FREESCALE_I2C_S_RXAK_M) {
                // nack
                extra->i2c_got_nack = 1;
                I2C_STOP(i2c_s);
                result = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
                I2C_TRACE("I2C: RX, got initial NACK\n");
            } else {
                /*
                 * The slave address was transmitted successfully, now switch
                 * the bus to RX
                 */
                volatile cyg_uint8 dummy_read __attribute__((unused));

                I2C_TRACE("I2C: RX, switching bus to RX\n");
                i2c_s->c1 &= ~FREESCALE_I2C_C1_TX_M;

                // ACK if more bytes
                if (extra->i2c_count != 1)
                    i2c_s->c1 &= ~FREESCALE_I2C_C1_TXAK_M;
                // NACK if asked to
                else if (extra->i2c_send_nack)
                    i2c_s->c1 |= FREESCALE_I2C_C1_TXAK_M;
                // ACK if last btye but not asked to NACK,
                // meaning there will be another read
                else
                    i2c_s->c1 &= ~FREESCALE_I2C_C1_TXAK_M;

                // do a dummy read to get things going
                dummy_read = i2c_s->d;
            }
        }
        /*
         * Already in RX mode ...
         */
        else if (sr & FREESCALE_I2C_S_ARBL_M) {
            // Lost the bus? Maybe a spurious stop
            I2C_CLEAR(i2c_s, FREESCALE_I2C_S_ARBL_M);
            I2C_TRACE("I2C: RX, bus arbitration error\n");
            result = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
        } else {
            I2C_TRACE("I2C: RX, got some data, count %d\n", extra->i2c_count);
            if (2 == extra->i2c_count) {
                // One more byte to go, and it should be NACKed.
                I2C_TRACE("I2C: RX, penultimate byte\n");
                if (extra->i2c_send_nack)
                    i2c_s->c1 |= FREESCALE_I2C_C1_TXAK_M;
            } else if (1 == extra->i2c_count) {
                // Received the last byte.
                I2C_TRACE("I2C: RX, last byte\n");

                if (extra->send_stop)
                    I2C_STOP(i2c_s);

                result = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;
            } else if (0 == extra->i2c_count) {
                /*
                 * Data beyond last byte ... this should never happen, but see
                 * the comment in freescale_i2c_handle_xfer()
                 */
                CYG_FAIL("I2C: RX sender sending data beyond last byte\n");

                // Forcefully stop the bus and quit
                I2C_STOP(i2c_s);

                result = CYG_ISR_HANDLED | CYG_ISR_CALL_DSR;

                HAL_INTERRUPT_ACKNOWLEDGE (extra->i2c_isr_id);

                return result;
            }

            // extract data
            *(extra->i2c_data.i2c_rx_data) = i2c_s->d;
            I2C_TRACE("I2C: RX, data: 0x%02x\n",
                      *(extra->i2c_data.i2c_rx_data));

            extra->i2c_data.i2c_rx_data += 1;
            extra->i2c_count -= 1;
        }
    } else {
        /*
         * Invalid state? Some kind of spurious interrupt? Ignore it.
         */
        I2C_TRACE("I2C spurious interrupt\n");
    }

    HAL_INTERRUPT_ACKNOWLEDGE (extra->i2c_isr_id);

    return result;
}

// Deferred interrupt service routine
static void
freescale_i2c_dsr(cyg_vector_t vec, cyg_ucount32 count, cyg_addrword_t data)
{
    freescale_i2c_extra *extra = (freescale_i2c_extra *) data;

    I2C_TRACE ("I2C DSR -- finishing off\n");

    extra->i2c_completed = 1;
    cyg_drv_cond_signal(&(extra->i2c_wait));
}

// A transfer has been started. Wait for completion
static inline void
freescale_i2c_doit(freescale_i2c_extra * extra)
{
    cyg_drv_mutex_lock(&(extra->i2c_lock));
    cyg_drv_dsr_lock();

    while (!extra->i2c_completed) {
        I2C_TRACE ("Waiting ...\n");
        cyg_drv_cond_wait(&(extra->i2c_wait));
    }

    cyg_drv_dsr_unlock();
    cyg_drv_mutex_unlock(&(extra->i2c_lock));
}

static inline void
freescale_i2c_stopit(freescale_i2c_extra * extra)
{
    extra->i2c_lost_arb = 0;
    extra->i2c_owner = 0;
    extra->i2c_mode = FREESCALE_I2C_XFER_MODE_INVALID;
}

void
freescale_i2c_stop(const cyg_i2c_device * dev)
{
    freescale_i2c_extra *extra = (freescale_i2c_extra *) dev->i2c_bus->i2c_extra;
    freescale_i2c_t     *i2c_s = FREESCALE_I2C_P (extra->i2c_base);

    I2C_STOP(i2c_s);

    freescale_i2c_stopit(extra);
}

static cyg_bool
freescale_i2c_handle_xfer(cyg_i2c_device * dev, int address)
{
    freescale_i2c_extra  *extra = (freescale_i2c_extra *) dev->i2c_bus->i2c_extra;
    cyg_uint8             data = *extra->i2c_data.i2c_tx_data;
    freescale_i2c_t      *i2c_s = FREESCALE_I2C_P (extra->i2c_base);
    cyg_uint8 freq;

    // Nothing to do
    if (extra->i2c_count == 0)
        return false;

    // Take the bus ownership
    if(!wait_bus_ready (extra))
        return false;

    // set the bus frequency
    freq = freescale_i2c_freq_div(dev->i2c_delay ?
                                  &dev->i2c_delay :
                                  &extra->i2c_delay);

    i2c_s->f = freq;
    I2C_TRACE("Set f = 0x%02x\n", i2c_s->f);

    if (extra->send_start) {
        I2C_TRACE ("Doing start: restart: %s\n",
                   i2c_s->c1 & FREESCALE_I2C_C1_MST_M ? "yes" : "no");

        /*
         * If we are the current master, use repeat start
         */
        if (i2c_s->c1 & FREESCALE_I2C_C1_MST_M) {
            I2C_REPEAT_START(i2c_s);
        } else {
#if 0
            i2c_s->c1 &= ~0x3fu;
#endif
            I2C_START(i2c_s);
        }

        I2C_TRACE ("STATUS: sr 0x%02x, cr 0x%02x\n", i2c_s->s, i2c_s->c1);

        /*
         * Write the address out and wait for interrupt
         */
        I2C_TRACE ("Address: %d [0x%x]\n", address, address);
        i2c_s->d = address;
        return true;
    } else if (extra->i2c_mode == FREESCALE_I2C_XFER_MODE_TX) {
        // TX transfer without a start
        I2C_TRACE ("TX transfer without start\n");

        extra->i2c_data.i2c_tx_data += 1;
        extra->i2c_count -= 1;
        i2c_s->d = data;

        // Single byte transfer
        if (extra->send_stop && (extra->i2c_count == 0))
            I2C_STOP(i2c_s);
    } else {
        // RX transfer without a start
#if 0
        /*
         * This is how it would be done in theory; in practice, because the
         * actual reading is IRQ driven, we are stuck in the isr handler until
         * such time the sender will stop sending data, but since the sender
         * will continue sending data until it receives the stop signal from us,
         * if someone makes an RX call with the 'don't send stop' flag, the
         * incoming data stream will never stop (until there is an error).
         *
         * And since we can never allow an RX call with the 'don't send stop'
         * flag, we can never do send without explicit start either.
         */
        I2C_TRACE ("RX transfer without start\n");
        // Single byte transfer, set the STOP bit
        if (extra->send_stop && (extra->i2c_count == 1))
            I2C_STOP(i2c_s);

        // Do not ACK last byte per user request
        if (!extra->i2c_send_nack && (extra->i2c_count == 1)) {
            I2C_TRACE ("RX: NACK for last byte in single byte transfer\n");
            i2c_s->c1 |= FREESCALE_I2C_C1_TXAK_M;
        }
#else
        CYG_FAIL( "I2C driver does not support piecemeal RX\n" );
#endif
    }

    return true;
}

cyg_uint32
freescale_i2c_tx(cyg_i2c_device * dev, cyg_bool send_start,
                 const cyg_uint8 *tx_data, cyg_uint32 count, cyg_bool send_stop)
{
    freescale_i2c_extra *extra = (freescale_i2c_extra *) dev->i2c_bus->i2c_extra;
    extra->send_stop = send_stop;
    extra->send_start = send_start;
    extra->i2c_count = count;

    if (!extra->i2c_lost_arb) {
        extra->i2c_completed = 0;
        extra->i2c_mode = FREESCALE_I2C_XFER_MODE_TX;

        if (send_start || !extra->i2c_got_nack) {
            I2C_TRACE
                  ("I2C: TX to %2x, data %2x, count: %4d, START flag: %s\n",
                   dev->i2c_address, *tx_data, count,
                   (send_start == true) ? "true" : "false");
            extra->i2c_data.i2c_tx_data = tx_data;
            if (!freescale_i2c_handle_xfer (dev, FREESCALE_I2C_A1_AD (dev->i2c_address,
                                                                      FREESCALE_I2C_MODE_MW))
                )
            {
                return 0;
            }

            /*
             * Now block till we are done
             */
            freescale_i2c_doit(extra);
        }

    }

    if (send_stop) {
        I2C_TRACE("I2C: TX send stop\n");
        freescale_i2c_stopit(extra);
    }

    I2C_TRACE("I2C: TX count %d\n", extra->i2c_count);

    /*
     * tx() should return the number of bytes actually transmitted.
     * ISR() increments extra->count after a failure, which leads to
     * an edge condition when send_start and there is no acknowledgment
     * of the address byte.
     */
    if (extra->i2c_count > count)
        return 0;

    return count - extra->i2c_count;
}

cyg_uint32
freescale_i2c_rx(cyg_i2c_device * dev, cyg_bool send_start,
                 cyg_uint8 *rx_data, cyg_uint32 count, cyg_bool send_nack,
                 cyg_bool send_stop)
{
    freescale_i2c_extra *extra = (freescale_i2c_extra *) dev->i2c_bus->i2c_extra;

    /*
     * see comments in freescale_i2c_handle_xfer() as to why this assert.
     */
    CYG_ASSERT( send_start && send_stop,
                "I2C driver does not support piecemeal RX, i.e., the "
                "send_start and send_stop parameters must always be set to "
                "true\n");

    extra->i2c_count = count;
    extra->i2c_send_nack = send_nack;
    extra->send_stop = send_stop;
    extra->send_start = send_start;

    if (!extra->i2c_lost_arb) {
        extra->i2c_completed = 0;
        extra->i2c_data.i2c_rx_data = rx_data;
        extra->i2c_mode = FREESCALE_I2C_XFER_MODE_RX;
        I2C_TRACE("I2C: RX to %2x, count: %4d, START flag: %s\n",
                  dev->i2c_address, count,
                  (send_start == true) ? "true" : "false");

        if (!freescale_i2c_handle_xfer (dev, FREESCALE_I2C_A1_AD (dev->i2c_address,
                                                                  FREESCALE_I2C_MODE_MR))
            )
        {
            return 0;
        }

        /*
         * Now block till we are done
         */
        freescale_i2c_doit(extra);
    }

    if (send_stop) {
        I2C_TRACE("I2C: RX send stop\n");
        freescale_i2c_stopit(extra);
    }

    return count - extra->i2c_count;
}

// ----------------------------------------------------------------------------
// The functions needed for all I2C devices.

void
freescale_i2c_init(struct cyg_i2c_bus *bus)
{
    freescale_i2c_extra *extra = (freescale_i2c_extra *) bus->i2c_extra;
    freescale_i2c_t     *i2c_s = FREESCALE_I2C_P (extra->i2c_base);
    cyg_uint32 freq;

    I2C_TRACE("I2C INIT0 bus base 0x%08x Pins: 0x%08x, 0x%08x Clock 0x%08x\n",
              extra->i2c_base, extra->hal_p->pin_sda, extra->hal_p->pin_scl,
              extra->hal_p->clock);

    // Bring clock to the sevice
    CYGHWR_IO_CLOCK_ENABLE(extra->hal_p->clock);
    // setup SDA/SCL pins, the I2C driver to take care of the rest.
    CYGHWR_IO_FREESCALE_I2C_PIN(extra->hal_p->pin_sda);
    CYGHWR_IO_FREESCALE_I2C_PIN(extra->hal_p->pin_scl);

    cyg_drv_mutex_init(&extra->i2c_lock);
    cyg_drv_cond_init(&extra->i2c_wait, &extra->i2c_lock);
    cyg_drv_interrupt_create(extra->i2c_isr_id,
                             extra->i2c_isr_pri,
                             (cyg_addrword_t)extra,
                             &freescale_i2c_isr,
                             &freescale_i2c_dsr,
                             &(extra->i2c_interrupt_handle),
                             &(extra->i2c_interrupt_data));
    cyg_drv_interrupt_attach(extra->i2c_interrupt_handle);

    if (!extra->i2c_owner) {
#ifdef CYGPKG_DEVS_I2C_FREESCALE_I2C_TRACE
        cyg_uint8 cr;
#endif
        cyg_uint8 sr;

        extra->i2c_got_nack = 0;
        extra->i2c_owner = 1;
        sr = i2c_s->s;
#ifdef CYGPKG_DEVS_I2C_FREESCALE_I2C_TRACE
        cr = i2c_s->c1;
#endif

        I2C_TRACE ("BUS: INIT sr 0x%02x, cr 0x%02x\n", sr, cr);

        while (sr & FREESCALE_I2C_S_BUSY_M) {
            sr = i2c_s->s;
        }
    }

#if 1
    i2c_s->c2 = extra->c2_cfg;    // some options set in CDL
    i2c_s->flt = extra->flt_cfg;  // glitch filter
#endif

    /*
     * bus speed: set the frequency divider
     *
     * The bus can run up to clock/20, but can only achieve full loads
     * under 100kHz
     */
    I2C_TRACE("I2C INIT1: extra->i2c_delay = %d\n", extra->i2c_delay);

    freq = freescale_i2c_freq_div(&extra->i2c_delay);

    I2C_TRACE("I2C INIT2: -> extra->i2c_delay = 0x%08x\n", extra->i2c_delay);

    i2c_s->f = freq;

    /*
     * Signal start -- selects master mode
     * Enable I2C
     * Enable Interrupt
     */
    i2c_s->c1 |= (FREESCALE_I2C_C1_IICEN_M | FREESCALE_I2C_C1_IICIE_M);

    // Interrupts can now be safely unmasked
    HAL_INTERRUPT_UNMASK(extra->i2c_isr_id);
}

//-----------------------------------------------------------------------------
// Bus instances.

#include <cyg/io/i2c_freescale_buses.inl>

//---------------------------------------------------------------------------
// EOF i2c_freescale.c
