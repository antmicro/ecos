//==========================================================================
//
//        fpint_thread_switch.cxx
//
//        Thread switch delay measurement
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     ilijak
// Original code: gthomas (tm_basic.cxx from kernel tests)
// Date:          2012-12-18
// Description:   Switch delay measurement between threads that do/don't use
//                floating point operations.
//####DESCRIPTIONEND####

#include <pkgconf/kernel.h>
#include <pkgconf/hal.h>

#include <cyg/kernel/sched.hxx>
#include <cyg/kernel/thread.hxx>
#include <cyg/kernel/thread.inl>
#include <cyg/kernel/mutex.hxx>
#include <cyg/kernel/sema.hxx>
#include <cyg/kernel/flag.hxx>
#include <cyg/kernel/sched.inl>
#include <cyg/kernel/clock.hxx>
#include <cyg/kernel/clock.inl>
#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>

#include <cyg/kernel/test/stackmon.h>
#include CYGHWR_MEMORY_LAYOUT_H


#ifndef HAL_CLOCK_READ_NS
#define HAL_CLOCK_READ_NS(__pvalue) \
CYG_MACRO_START                     \
    HAL_CLOCK_READ(__pvalue);       \
    *__pvalue *= 1000;              \
CYG_MACRO_END
#endif

//==========================================================================
// Random number generator. Ripped out of the C fptest.c.

static int CYGBLD_ATTRIB_NO_INLINE rand( unsigned int *seed )
{
// This is the code supplied in Knuth Vol 2 section 3.6 p.185 bottom

#define RAND_MAX 0x7fffffff
#define MM 2147483647    // a Mersenne prime
#define AA 48271         // this does well in the spectral test
#define QQ 44488         // (long)(MM/AA)
#define RR 3399          // MM % AA; it is important that RR<QQ

    *seed = AA*(*seed % QQ) - RR*(unsigned int)(*seed/QQ);
    if (*seed < 0)
        *seed += MM;

    return (int)( *seed & RAND_MAX );
}

// Define this to see the statistics with the first sample datum removed.
// This can expose the effects of caches on the speed of operations.
#undef STATS_WITHOUT_FIRST_SAMPLE
#define STATS_WITHOUT_FIRST_SAMPLE 1

#if defined(CYGFUN_KERNEL_API_C) &&             \
    defined(CYGSEM_KERNEL_SCHED_MLQUEUE) &&     \
    defined(CYGVAR_KERNEL_COUNTERS_CLOCK) &&    \
    !defined(CYGDBG_INFRA_DIAG_USE_DEVICE) &&   \
    (CYGNUM_KERNEL_SCHED_PRIORITIES > 12)

#define NTHREADS 4
#include "testaux.hxx"

// Structure used to keep track of times
typedef struct fun_times {
    cyg_uint32 start;
    cyg_uint32 end;
} fun_times;

#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_TYPICAL

#define NTEST_THREADS    2

#define NSAMPLES         2

#define NTHREAD_SWITCHES 128
#define NTHREAD_SWITCHES_TS 16

#define NSAMPLES_SIM 2
#define NTHREAD_SWITCHES_SIM 4
#define NTHREAD_SWITCHES_TS_SIM 4

static int nsamples;
static int ntest_threads;
static int nthread_switches;
static int nthread_switches_ts;

static char stacks[NTEST_THREADS][STACK_SIZE] CYGBLD_ATTRIB_ALIGN_MAX;
static cyg_thread test_threads[NTEST_THREADS];
static cyg_handle_t threads[NTEST_THREADS];
static cyg_sem_t test_sem[NTEST_THREADS];
static int overhead;
static cyg_sem_t synchro;

static fun_times test2_ft[4][NTHREAD_SWITCHES];


static long rtc_resolution[] = CYGNUM_KERNEL_COUNTERS_RTC_RESOLUTION;
static long ns_per_system_clock;

void run_thread_switch_test(int);

#ifndef min
#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#endif



// Wait until a clock tick [real time clock] has passed.  This should keep it
// from happening again during a measurement, thus minimizing any fluctuations
void
wait_for_tick(void)
{
    cyg_tick_count_t tv0, tv1;
    tv0 = cyg_current_time();
    while (true) {
        tv1 = cyg_current_time();
        if (tv1 != tv0) break;
    }
}

// Display a number of ticks as nanoseconds
void
show_ticks_in_ns(cyg_uint32 ticks)
{
    long long ns;
    ns = (ns_per_system_clock * (long long)ticks) / CYGNUM_KERNEL_COUNTERS_RTC_PERIOD;
    diag_printf("%8d", (int)(ns));
}


void
show_times_hdr(void)
{
    diag_printf("\n");
    diag_printf("                                 Confidence\n");
    diag_printf("     Ave     Min     Max Max-Min  Ave  Min  Samp  Function\n");
    diag_printf("  ======  ======  ======  ====== ==== ===== ===== ========\n");
}

void
show_times_detail(fun_times ft[], int nsamples, char *title, bool ignore_first)
{
    int i, delta, min, max, con_ave, con_min, ave_dev;
    int start_sample, total_samples;
    cyg_int32 total, ave;

    if (ignore_first) {
        start_sample = 1;
        total_samples = nsamples-1;
    } else {
        start_sample = 0;
        total_samples = nsamples;
    }
    total = 0;
    min = 0x7FFFFFFF;
    max = 0;
    for (i = start_sample;  i < nsamples;  i++) {
        if (ft[i].end < ft[i].start) {
            // Clock wrapped around (timer tick)
            delta = (ft[i].end+CYGNUM_KERNEL_COUNTERS_RTC_PERIOD*1000) - ft[i].start;
        } else {
            delta = ft[i].end - ft[i].start;
        }
        delta -= overhead;
        if (delta < 0) delta = 0;
        total += delta;
        if (delta < min) min = delta;
        if (delta > max) max = delta;
    }
    ave = total / total_samples;
    total = 0;
    ave_dev = 0;
    for (i = start_sample;  i < nsamples;  i++) {
        if (ft[i].end < ft[i].start) {
            // Clock wrapped around (timer tick)
            delta = (ft[i].end+CYGNUM_KERNEL_COUNTERS_RTC_PERIOD*1000) - ft[i].start;
        } else {
            delta = ft[i].end - ft[i].start;
        }
        delta -= overhead;
        if (delta < 0) delta = 0;
        delta = delta - ave;
        if (delta < 0) delta = -delta;
        ave_dev += delta;
    }
    ave_dev /= total_samples;
    con_ave = 0;
    con_min = 0;
    for (i = start_sample;  i < nsamples;  i++) {
        if (ft[i].end < ft[i].start) {
            // Clock wrapped around (timer tick)
            delta = (ft[i].end+CYGNUM_KERNEL_COUNTERS_RTC_PERIOD*1000) - ft[i].start;
        } else {
            delta = ft[i].end - ft[i].start;
        }
        delta -= overhead;
        if (delta < 0) delta = 0;
        if ((delta <= (ave+ave_dev)) && (delta >= (ave-ave_dev))) con_ave++;
        if ((delta <= (min+ave_dev)) && (delta >= (min-ave_dev))) con_min++;
    }
    con_ave = (con_ave * 100) / total_samples;
    con_min = (con_min * 100) / total_samples;
    show_ticks_in_ns(ave);
    show_ticks_in_ns(min);
    show_ticks_in_ns(max);
    show_ticks_in_ns(max-min);
    diag_printf("  %3d%% %3d%%", con_ave, con_min);
    diag_printf("  %4d", total_samples);
    diag_printf(" %s\n", title);
}

void
show_times(fun_times ft[], int nsamples, char *title)
{
#ifndef STATS_WITHOUT_FIRST_SAMPLE
    show_times_detail(ft, nsamples, title, false);
#else// STATS_WITHOUT_FIRST_SAMPLE
    show_times_detail(ft, nsamples, title, true);
#endif
}

void
show_test_parameters(void)
{
    diag_printf("\nTesting parameters:\n");
    diag_printf("   Thread switches:            %5d\n", nthread_switches);
    diag_printf("   Time unit:         nanoseconds [ns]\n");
}

void
end_of_test_group(void)
{
    diag_printf("\n");
}

// Compute a name for a thread
char *
thread_name(char *basename, int indx) {
    return "<<NULL>>";  // Not currently used
}

// test0 - null test, never executed
void
test0(cyg_uint32 indx)
{
#ifndef CYGPKG_KERNEL_SMP_SUPPORT
    // In SMP, somw of these threads will execute
    diag_printf("test0.%d executed?\n", indx);
#endif
    cyg_thread_exit();
}

// test1 - empty test, simply exit.  Last thread signals parent.
void
test1(cyg_uint32 indx)
{
    if (indx == (cyg_uint32)(ntest_threads-1)) {
        cyg_semaphore_post(&synchro);  // Signal that last thread is dying
    }
    cyg_thread_exit();
}

char title[128];
char* thread_title[2];

volatile int ires __attribute__((unused));

// This thread does not use floating point
void
thread_int(cyg_uint32 indx_fp)
{
    int i;
    int fp_i, indx;
    unsigned int seed;
    int iop1, iop2;

    fp_i = indx_fp & 0x3;
    indx = (indx_fp & 0x4) >> 2;
    thread_title[indx] = "int";

    // Just for symmetry with thread_fp.
    seed = indx_fp;
    iop1 = rand(&seed);
    iop2 = rand(&seed);
    ires = iop1 + iop2;
    cyg_semaphore_wait(&test_sem[indx]);

    for (i = 0;  i < nthread_switches;  i++) {
        ires = iop1 + iop2;
        if (indx == 0) {
            HAL_CLOCK_READ_NS(&test2_ft[fp_i][i].start);
        } else {
            HAL_CLOCK_READ_NS(&test2_ft[fp_i][i].end);
        }
        cyg_thread_yield();
    }
    if (indx) {
        cyg_semaphore_post(&synchro);
    }
    cyg_thread_exit();
}

volatile float fres __attribute__((unused));

// This thread does use floating point
void
thread_fp(cyg_uint32 indx_fp)
{
    int i;
    int fp_i, indx;
    unsigned int seed;
    float fop1, fop2;

    fp_i = indx_fp & 0x3;
    indx = (indx_fp & 0x4) >> 2;
    thread_title[indx] = "fpu";

    // In LAZY mode we need some FP usage to enforce
    // FPU context saving.
    seed = indx_fp;
    fop1 = (float)rand(&seed) / (float)0x7fffffff;
    fop2 = (float)rand(&seed) / (float)0x7fffffff;
    fres = fop1 + fop2;
    cyg_semaphore_wait(&test_sem[indx]);

    for (i = 0;  i < nthread_switches;  i++) {
        fres = fop1 + fop2;
        if (indx == 0) {
            HAL_CLOCK_READ_NS(&test2_ft[fp_i][i].start);
        } else {
            HAL_CLOCK_READ_NS(&test2_ft[fp_i][i].end);
        }
        cyg_thread_yield();
    }
    if (indx) {
        cyg_semaphore_post(&synchro);
    }
    cyg_thread_exit();
}

void
run_tests(CYG_ADDRESS id)
{
    int tst_i;

    ns_per_system_clock = 1000000/rtc_resolution[1];

    diag_printf("Thread switch test\n");
    show_test_parameters();
    // Set my priority lower than any I plan to create
    cyg_thread_set_priority(cyg_thread_self(), 3);

    // Set up the end-of-threads synchronizer
    cyg_semaphore_init(&synchro, 0);
    show_times_hdr();
#if CYGNUM_TESTS_RUN_COUNT < 0
    while (1)
#else
    int i;
    for (i = 0;  i < CYGNUM_TESTS_RUN_COUNT;  i++)
#endif
        for(tst_i = 0; tst_i < 4; tst_i ++){
            run_thread_switch_test(tst_i);
        }
    CYG_TEST_PASS_FINISH("Thread switching OK");
}


void
run_thread_switch_test(int fp_i)
{
    int i;
    cyg_thread_entry_t* thr_ent[2];

    for (i = 0;  i < 2;  i++) {
        cyg_semaphore_init(&test_sem[i], 0);
    }
    // Set up for thread context switch
    thr_ent[0] = !(fp_i & 0x2) ? thread_int : thread_fp;
    thr_ent[1] = !(fp_i & 0x1) ? thread_int : thread_fp;

    for (i = 0;  i < 2;  i++) {
        cyg_thread_create(10,         // Priority - just a number
                thr_ent[i],           // entry
                (i << 2) | fp_i,      // index
                thread_name("thread", i),     // Name
                &stacks[i][0],   // Stack
                STACK_SIZE,      // Size
                &threads[i],     // Handle
                &test_threads[i] // Thread data structure
            );
    }
    for (i = 0;  i < 2;  i++) {
        cyg_thread_resume(threads[i]);
    }
    cyg_thread_delay(2);
    wait_for_tick(); // Wait until the next clock tick to minimize aberations
    for (i = 0;  i < 2;  i++) {
        cyg_semaphore_post(&test_sem[i]);
    }
    cyg_semaphore_wait(&synchro);
    __builtin_strcpy(title, "Thread switch: ");
    __builtin_strcat(title, thread_title[0]);
    __builtin_strcat(title,"-");
    __builtin_strcat(title, thread_title[1]);
    show_times(test2_ft[fp_i], nthread_switches, title);
    // Clean up
    for (i = 0;  i < 2;  i++) {
        cyg_thread_delete(threads[i]);
    }
}

void thread_switch_main( void )
{
    CYG_TEST_INIT();

    if (cyg_test_is_simulator) {
        nsamples = NSAMPLES_SIM;
        nthread_switches = NTHREAD_SWITCHES_SIM;
        nthread_switches_ts = NTHREAD_SWITCHES_TS_SIM;
    } else {
        nsamples = NSAMPLES;
        nthread_switches = NTHREAD_SWITCHES;
        nthread_switches_ts = NTHREAD_SWITCHES_TS;
    }

    new_thread(run_tests, 0);

    Cyg_Scheduler::scheduler.start();

}

#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
externC void
cyg_hal_invoke_constructors();
#endif

externC void
cyg_start( void )
{
#ifdef CYGSEM_HAL_STOP_CONSTRUCTORS_ON_FLAG
    cyg_hal_invoke_constructors();
#endif
    thread_switch_main();
}

#else // CYGFUN_KERNEL_API_C

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO("Timing tests require:\n"
                "CYGFUN_KERNEL_API_C && \n"
                "CYGSEM_KERNEL_SCHED_MLQUEUE &&\n"
                "CYGVAR_KERNEL_COUNTERS_CLOCK &&\n"
                "!CYGDBG_INFRA_DIAG_USE_DEVICE &&\n"
                "(CYGNUM_KERNEL_SCHED_PRIORITIES > 12)\n");
    CYG_TEST_NA("Timing tests requirements");
}
#endif // CYGFUN_KERNEL_API_C, etc.

// EOF fpint_thread_switch.cxx
