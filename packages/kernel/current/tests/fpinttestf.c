//==========================================================================
//
//        fpinttestf.c
//
//        Basic FPU integrity test
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
// Original code: nickg@calivar.com
// Contributors:
// Date:          2012-12-18
// Description:   Simple FPU test with a mix of threads that do and do not use
//                floating point.
//                This is a modification of the original FPU test, and presents
//                both "integer" and "float" threads in order to enforce LAZY
//                context switching. Single precision floating point is used.
//                This is not very sophisticated as far
//                as checking FPU performance or accuracy. It is more
//                concerned with checking that several threads doing FP
//                operations do not interfere with eachother's use of the
//                FPU.
//                Note: NONE context switching scheme should NOT pass this test with
//                      multiple FP threads. Look for fpinttestf1.c instead.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <pkgconf/kernel.h>
#include <pkgconf/hal.h>

#include <cyg/hal/hal_arch.h>

#include <cyg/kernel/kapi.h>

#include <cyg/infra/testcase.h>
#include <cyg/infra/diag.h>

#include <cyg/kernel/test/stackmon.h>
#include CYGHWR_MEMORY_LAYOUT_H

//==========================================================================

#ifndef FP_THREADS_N
#define FP_THREADS_N 3
#define FP_TEST_NAME "FP test"
#endif

#if defined(CYGFUN_KERNEL_API_C) &&                                    \
    defined(CYGSEM_KERNEL_SCHED_MLQUEUE) &&                            \
    (CYGNUM_KERNEL_SCHED_PRIORITIES > 12) &&                           \
    (CYGMEM_REGION_ram_SIZE >= (49152-4096)) &&                        \
    (!defined(CYGTST_KERNEL_SKIP_MULTI_THREAD_FP_TEST) || (FP_THREADS_N == 1))

//==========================================================================
// Base priority for all threads.

#define BASE_PRI        5

//==========================================================================
// Runtime
//
// This is the number of ticks that the program will run for. 3000
// ticks is equal to 30 seconds in the default configuration. For
// simulators we reduce the run time to 3 simulated seconds.

#define RUN_TICKS       3000
#define RUN_TICKS_SIM   300

//==========================================================================
// Thread parameters

#define STACK_SIZE CYGNUM_HAL_STACK_SIZE_MINIMUM
#define THREADS_N 8
static cyg_uint8 stacks[THREADS_N][STACK_SIZE] CYGBLD_ATTRIB_ALIGN_MAX;
static cyg_handle_t thread[THREADS_N];
static cyg_thread thread_struct[THREADS_N];
static unsigned long iter_n[THREADS_N];

//==========================================================================
// Alarm parameters.

static cyg_alarm alarm_struct;
static cyg_handle_t alarm;

static cyg_count8 cur_thread = 0;
static cyg_count32 alarm_ticks = 0;
static cyg_count32 run_ticks = RUN_TICKS;

//==========================================================================

static int errors = 0;

//==========================================================================
// Random number generator. Ripped out of the C library.

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


//==========================================================================
// Test calculation.
//
// Generates an array of random FP values and then repeatedly applies
// a calculation to them and checks that the same result is reached
// each time. The calculation, in the macro CALC, is intended to make
// maximum use of the FPU registers. However, the i386 compiler
// doesn't let this expression get very complex before it starts
// spilling values out to memory.

static void do_test( float *values,
                     int count,
                     int loops,
                     int test,
                     const char *name)
{
    unsigned int i, j;
    // volatiles necessary to force
    // values to 32 bits for comparison
    volatile float sum = 1.0;
    volatile float last_sum;
    unsigned int seed;

    cyg_uint32 iter = 0;
    int thread_i = name[__builtin_strlen(name)-1]-'1';

#define V(__i) (values[(__i)%count])
#define CALC ((V(i-1)*V(i+1))*(V(i-2)*V(i+2))*(V(i-3)*sum))

    seed = ((unsigned int)&i)*count;

    // Set up an array of values...
    for( i = 0; i < count; i++ )
        values[i] = (float)rand( &seed )/(float)0x7fffffff;

    // Now calculate something from them...
    for( i = 0; i < count; i++ )
        sum += CALC;

    last_sum = sum;

    // Now recalculate the sum in a loop and look for errors
    for( j = 0; j < loops ; j++ )
    {
        iter++;
        if(thread_i < 3)
            iter_n[thread_i]++;
        sum = 1.0;
        for( i = 0; i < count; i++ )
            sum += CALC;

        if( sum != last_sum )
        {
            union float_int_union {
                float d;
                cyg_uint32 i;
            } diu_sum, diu_lastsum;

            diu_sum.d = sum;
            diu_lastsum.d = last_sum;

            errors++;
            if (sizeof(float) != sizeof(cyg_uint32)) {
                diag_printf("Warning: sizeof(float) != sizeof(cyg_uint32), therefore next line may\n"
                            "have invalid sum/last_sum values\n");
            }
            diag_printf("%s: Sum mismatch! %d sum=[%08x] last_sum=[%08x]\n",
                        name,j, diu_sum.i, diu_lastsum.i );

        }
#if 0
        if( ((j*count)%1000000) == 0 )
            diag_printf("INFO:<%s: %2d calculations done>\n",name,j*count);
#endif
    }
#if 0
    if(thread_i < 3)
        diag_printf("INFO:<%s [%d]: %2d calculations done>\n",name,thread_i,j*count);
#endif
    if(thread_i < 3)
        iter_n[thread_i] = iter;
}


//==========================================================================
// Test calculation.
//
// Generates an array of random integer values and then repeatedly applies
// a calculation to them and checks that the same result is reached
// each time.

static void do_int_test( int *values,
                     int count,
                     int loops,
                     int test,
                     const char *name)
{
    unsigned int i, j;
    // volatiles necessary to force
    // values to 32 bits for comparison
    volatile int sum = 1;
    volatile int last_sum;
    unsigned int seed;
#if 0
    cyg_uint32 ctrlreg;
#endif
    cyg_uint32 iter = 0;
    int thread_i = name[__builtin_strlen(name)-1]-'1';

#define VI(__i) (values[(__i)%count])
#define CALCI ((VI(i-1)*VI(i+1))*(VI(i-2)*VI(i+2))*(VI(i-3)*sum))

    seed = ((unsigned int)&i)*count;

    // Set up an array of values...
    for( i = 0; i < count; i++ )
        values[i] = rand( &seed );

    // Now calculate something from them...
    for( i = 0; i < count; i++ )
        sum += CALC;

    last_sum = sum;

    // Now recalculate the sum in a loop and look for errors
    for( j = 0; j < loops ; j++ )
    {
        iter++;
        if(thread_i < 8)
            iter_n[thread_i]++;
        sum = 1;
        for( i = 0; i < count; i++ )
            sum += CALC;

        if( sum != last_sum )
        {
            errors++;
            diag_printf("%s: Sum mismatch! %d sum=[%08x] last_sum=[%08x]\n",
                        name,j, sum, last_sum );
        }
#if 0
        CYGARC_MRS(ctrlreg, control);
        if(ctrlreg & 0x04){
            diag_printf("%s: control = 0x%08x\n", name, ctrlreg);
        }
#endif
#if 0
        if( ((j*count)%1000000) == 0 )
            diag_printf("INFO:<%s: %2d calculations done>\n",name,j*count);
#endif
    }
}

//==========================================================================
// Alarm handler
//
// This is called every tick. It lowers the priority of the currently
// running thread and raises the priority of the next. Thus we
// implement a form of timelslicing between the threads at one tick
// granularity.

static void alarm_fn(cyg_handle_t alarm, cyg_addrword_t data)
{
    alarm_ticks++;
    unsigned long iter_sum;

    if( alarm_ticks >= run_ticks )
    {
        if( errors )
            CYG_TEST_FAIL("Errors detected");
        else
            CYG_TEST_PASS("OK");

        iter_sum = + iter_n[0] + iter_n[1] + iter_n[2] + iter_n[3]+ iter_n[4] + iter_n[5]+ iter_n[6] + iter_n[7];
        diag_printf("Iterations = %lu+%lu+%lu+%lu+%lu+%lu+%lu+%lu",
                iter_n[0], iter_n[1], iter_n[2], iter_n[3], iter_n[4], iter_n[5], iter_n[6], iter_n[7]);
        diag_printf("=%lu\n", iter_sum);
        CYG_TEST_FINISH("FP Test done");
    }
    else
    {
        cyg_thread_set_priority( thread[cur_thread], BASE_PRI );

        cur_thread = (cur_thread+1)%8;

        cyg_thread_set_priority( thread[cur_thread], BASE_PRI-1 );
    }
}


//==========================================================================
// Floating point threads
//
#define FP1_COUNT 1000

static float fpt1_values[FP1_COUNT];

void fptest1( CYG_ADDRWORD id )
{
    while(1)
        do_test( fpt1_values, FP1_COUNT, 2000000000, id, "fptest1" );
}

//==========================================================================
#if (CYGMEM_REGION_ram_SIZE / 8 / 2) < 10000
#define FP2_COUNT (CYGMEM_REGION_ram_SIZE / 8 / 2)
#else
#define FP2_COUNT 10000
#endif

static float fpt2_values[FP2_COUNT];

void fptest2( CYG_ADDRWORD id )
{
    while(1)
        do_test( fpt2_values, FP2_COUNT, 2000000000, id, "fptest2" );
}

//==========================================================================

#define FP3_COUNT 100

static float fpt3_values[FP3_COUNT];

void fptest3( CYG_ADDRWORD id )
{
    while(1)
        do_test( fpt3_values, FP3_COUNT, 2000000000, id, "fptest3" );
}

//==========================================================================
// Integral threads
//

#define INT1_COUNT 1000

static int int1_values[INT1_COUNT];

void inttest4( CYG_ADDRWORD id )
{
    while(1)
        do_int_test( int1_values, INT1_COUNT, 2000000000, id, "inttest4" );
}

#define INT2_COUNT 1000

static int int2_values[INT2_COUNT];

void inttest5( CYG_ADDRWORD id )
{
    while(1)
        do_int_test( int2_values, INT2_COUNT, 2000000000, id, "inttest5" );
}

#define INT3_COUNT 1000

static int int3_values[INT3_COUNT];

void inttest6( CYG_ADDRWORD id )
{
    while(1)
        do_int_test( int3_values, INT3_COUNT, 2000000000, id, "inttest6" );
}


#define INT4_COUNT 1000

static int int4_values[INT4_COUNT];

void inttest7( CYG_ADDRWORD id )
{
    while(1)
        do_int_test( int4_values, INT4_COUNT, 2000000000, id, "inttest7" );
}

#define INT5_COUNT 1000

static int int5_values[INT5_COUNT];

void inttest8( CYG_ADDRWORD id )
{
    while(1)
        do_int_test( int5_values, INT5_COUNT, 2000000000, id, "inttest8" );
}

//======================================================================================
// Main

void fptest_main( void )
{
    CYG_TEST_INIT();

    if( cyg_test_is_simulator )
    {
        run_ticks = RUN_TICKS_SIM;
    }
    CYG_TEST_INFO("Run fptest in cyg_start");
    do_test( fpt3_values, FP3_COUNT, 1000, 0, "start" );
    CYG_TEST_INFO( "cyg_start run done");

    cyg_thread_create( BASE_PRI-1,
                       fptest1,
                       0,
                       "fptest1",
                       &stacks[0][0],
                       STACK_SIZE,
                       &thread[0],
                       &thread_struct[0]);
#if FP_THREADS_N > 0
    cyg_thread_resume( thread[0] );
#endif
    cyg_thread_create( BASE_PRI,
                       fptest2,
                       1,
                       "fptest2",
                       &stacks[1][0],
                       STACK_SIZE,
                       &thread[1],
                       &thread_struct[1]);

#if FP_THREADS_N > 1
    cyg_thread_resume( thread[1] );
#endif
    cyg_thread_create( BASE_PRI,
                       fptest3,
                       2,
                       "fptest3",
                       &stacks[2][0],
                       STACK_SIZE,
                       &thread[2],
                       &thread_struct[2]);

#if FP_THREADS_N > 2
    cyg_thread_resume( thread[2] );
#endif

    cyg_thread_create( BASE_PRI,
                       inttest4,
                       3,
                       "inttest4",
                       &stacks[3][0],
                       STACK_SIZE,
                       &thread[3],
                       &thread_struct[3]);

    cyg_thread_resume( thread[3] );

    cyg_thread_create( BASE_PRI,
                       inttest5,
                       4,
                       "inttest5",
                       &stacks[4][0],
                       STACK_SIZE,
                       &thread[4],
                       &thread_struct[4]);

    cyg_thread_resume( thread[4] );

    cyg_thread_create( BASE_PRI,
                       inttest6,
                       5,
                       "inttest6",
                       &stacks[5][0],
                       STACK_SIZE,
                       &thread[5],
                       &thread_struct[5]);

    cyg_thread_resume( thread[5] );

    cyg_thread_create( BASE_PRI,
                       inttest7,
                       6,
                       "inttest7",
                       &stacks[6][0],
                       STACK_SIZE,
                       &thread[6],
                       &thread_struct[6]);

    cyg_thread_resume( thread[6] );

    cyg_thread_create( BASE_PRI,
                       inttest8,
                       7,
                       "inttest8",
                       &stacks[7][0],
                       STACK_SIZE,
                       &thread[7],
                       &thread_struct[7]);

    cyg_thread_resume( thread[7] );

    cyg_alarm_create( cyg_real_time_clock(),
                      alarm_fn,
                      0,
                      &alarm,
                      &alarm_struct );

    cyg_alarm_initialize( alarm, cyg_current_time()+1, 1 );

    cyg_scheduler_start();

}

//==========================================================================

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
    fptest_main();
}

//==========================================================================

#else // CYGFUN_KERNEL_API_C...

externC void
cyg_start( void )
{
    CYG_TEST_INIT();
    CYG_TEST_INFO(FP_TEST_NAME " requires:\n"
                "CYGFUN_KERNEL_API_C && \n"
                "CYGSEM_KERNEL_SCHED_MLQUEUE && \n"
                "(CYGNUM_KERNEL_SCHED_PRIORITIES > 12) &&\n"
                "(CYGMEM_REGION_ram_SIZE >= (49152-4096)) &&\n"
                "(!defined(CYGHWR_HAL_CORTEXM_FPU_SWITCH_NONE) || (FP_THREADS_N == 1))"
                );
    CYG_TEST_NA("FP test requirements");
}

#endif // CYGFUN_KERNEL_API_C, etc.

//==========================================================================
// EOF fpinttestf.c
