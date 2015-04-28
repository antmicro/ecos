//=============================================================================
//
//      spi_loopback.c
//
//      Standalone SPI loopback test.
//
//=============================================================================
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
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   Ilija Kocho
// Original:    Chris Holgate
// Date:        2012-12-27
// Purpose:     Freescale DSPI loopback test
// Description: Standalone SPI loopback test.
// Usage:       Compile as a standalone application.
//
//####DESCRIPTIONEND####
//
//=============================================================================

//=============================================================================
// This is a quick loopback test for the Freescale DSPI SPI driver.  It only checks
// the data transfer functionality - chip select handling will require
// testing with external devices.  In order to run the test, the MOSI and
// MISO pins for the test port need to be shorted together to provide an
// external loopback.  Don't do this on a bus which has external devices
// attached unless you first make sure that none of them are connected to
// the chip select used by the test harness.
// The default port and chip select used for this test are SPI bus 1,
// chip select 0.  These can be changed by editing the loopback_device
// data structure directly.
// Note that this is intended to be run as a standalone test and not as part
// of the standard board tests since it requires a hardware modification.
//=============================================================================

#include <cyg/infra/cyg_type.h>
#include <cyg/infra/testcase.h>         // Test macros

#include <cyg/infra/cyg_ass.h>          // Assertion macros
#include <cyg/infra/diag.h>             // Diagnostic output

#include <cyg/hal/hal_arch.h>           // CYGNUM_HAL_STACK_SIZE_TYPICAL
#include <cyg/kernel/kapi.h>

#include <cyg/io/spi.h>                 // Common SPI API
#include <cyg/io/spi_freescale_dspi.h>  // Freescale DSPI data structures

#include <string.h>

//---------------------------------------------------------------------------
// Thread data structures.

cyg_uint8 stack [CYGNUM_HAL_STACK_SIZE_TYPICAL];
cyg_thread thread_data;
cyg_handle_t thread_handle;

externC cyg_spi_freescale_dspi_bus_t cyg_spi_dspi_bus1;

//---------------------------------------------------------------------------
// SPI loopback device driver data structures.

CYG_DEVS_SPI_FREESCALE_DSPI_DEVICE(
        loopback_device, // Device name
        1, //SPI bus
        0, // Dev num
        8, // Frame size
        0, // Clock pol
        0, // Clock phase
        6000000, // Clock speed
        1, // CS assert delay
        1, // CS negate delay
        1, // Delay between transfers
        1000, // Delay unit [ns]
        0 // Double baud rate
);

//---------------------------------------------------------------------------

static int errors = 0;

const char tx_data0[] = "0123456789a123456789b123456789c123456789d123456789e";
const char tx_data1[] = "Performing extended API test first transaction..|";
const char tx_data2[] = "Testing extended API for a second transaction!";

char rx_data [256];
char rx_data1 [256];
char rx_data2 [256];

static void memclr(char *dest_p, cyg_uint32 byte_n)
{
    while(byte_n--) {
        *dest_p++ = 0;
    }
}


//---------------------------------------------------------------------------
// Run single loopback transaction using simple transfer API call.

void run_test_tick (cyg_bool polled, cyg_uint32 count)
{
    diag_printf ("Test 0 : Tick  (polled = %d).\n", polled ? 1 : 0);
    cyg_spi_tick(&loopback_device, polled, count);
    diag_printf ("    Tick end\n");
}

void run_test_1 (cyg_bool polled, const char* tx_data_p, cyg_uint32 count)
{
    diag_printf ("Test 1 : Simple transfer test polled = %d, count=%d\n",
                 polled ? 1 : 0, count);
    memclr(rx_data1, sizeof(rx_data1));
    memclr(rx_data2, sizeof(rx_data1));
    cyg_spi_transfer (&loopback_device, polled, count,
        (const cyg_uint8*) tx_data_p, (cyg_uint8*) &rx_data[0]);
    diag_printf ("    Tx data : %s\n", tx_data_p);
    diag_printf ("    Rx data : %s 0x%02x\n", rx_data, rx_data[0]);

    if (memcmp (tx_data_p, rx_data, count) != 0) {
        errors++;
        diag_printf("Simple transfer loopback failed - mismatched data.\n");
    }
}

//---------------------------------------------------------------------------
// Run two loopback transactions using extended transfer API.

void run_test_2 (cyg_bool polled)
{
    diag_printf ("Test 2 : Extended API test (polled = %d).\n", polled ? 1 : 0);
    memclr(rx_data1, sizeof(rx_data1));
    memclr(rx_data2, sizeof(rx_data1));
    cyg_spi_transaction_begin (&loopback_device);
    cyg_spi_transaction_transfer (&loopback_device, polled, sizeof (tx_data1),
        (const cyg_uint8*) &tx_data1[0], (cyg_uint8*) &rx_data1[0], false);
    cyg_spi_transaction_transfer (&loopback_device, polled, sizeof (tx_data2),
        (const cyg_uint8*) &tx_data2[0], (cyg_uint8*) &rx_data2[0], true);
    cyg_spi_transaction_end (&loopback_device);

    diag_printf ("    Tx data 1 (%u Bytes): %s\n", sizeof(tx_data1), tx_data1);
    diag_printf ("    Rx data 1 : %s\n", rx_data1);
    diag_printf ("    Tx data 2 (%u Bytes): %s\n", sizeof(tx_data2), tx_data2);
    diag_printf ("    Rx data 2 : %s\n", rx_data2);
    if (memcmp (tx_data1, rx_data1, sizeof (tx_data1)) != 0) {
        errors++;
        diag_printf("Simple transfer loopback failed - mismatched data (transfer 1).\n");
    }
    if (memcmp (tx_data2, rx_data2, sizeof (tx_data2)) != 0) {
        errors++;
        diag_printf("Simple transfer loopback failed - mismatched data (transfer 2).\n");
    }
}

void run_test_3 (cyg_bool polled)
{
    diag_printf ("Test 3 : Extended API test (polled = %d).\n", polled ? 1 : 0);
    memclr(rx_data1, sizeof(rx_data1));
    memclr(rx_data2, sizeof(rx_data1));

    cyg_spi_transaction_begin (&loopback_device);
    cyg_spi_transaction_transfer (&loopback_device, polled, sizeof (tx_data1),
                                  (const cyg_uint8*) &tx_data1[0], (cyg_uint8*) &rx_data1[0], false);
    cyg_spi_transaction_transfer (&loopback_device, polled, sizeof (tx_data2),
                                  (const cyg_uint8*) &tx_data2[0], (cyg_uint8*) &rx_data2[0], true);
    cyg_spi_transaction_end (&loopback_device);
    diag_printf ("    Tx data 1 (%u Bytes): %s\n", sizeof(tx_data1), tx_data1);
    diag_printf ("    Rx data 31 : %s\n", rx_data1);
    diag_printf ("    Tx data 2 (%u Bytes): %s\n", sizeof(tx_data2), tx_data2);
    diag_printf ("    Rx data 32 : %s\n", rx_data2);

    if (memcmp (tx_data1, rx_data1, sizeof (tx_data1)) != 0) {
        errors++;
        diag_printf("Simple transfer loopback failed - mismatched data (transfer 1).\n");
    }
    if (memcmp (tx_data2, rx_data2, sizeof (tx_data2)) != 0) {
        errors++;
        diag_printf("Simple transfer loopback failed - mismatched data (transfer 2).\n");
    }
}

void run_test_4 (cyg_bool polled)
{
    diag_printf ("Test 4 : Extended API test NULL transfer (polled = %d).\n", polled ? 1 : 0);
    memclr(rx_data1, sizeof(rx_data1));
    memclr(rx_data2, sizeof(rx_data1));

    cyg_spi_transaction_begin (&loopback_device);
    cyg_spi_transaction_transfer (&loopback_device, polled, sizeof (tx_data1),
                                  (const cyg_uint8*) NULL, NULL, true);
    cyg_spi_transaction_end (&loopback_device);
    diag_printf ("    Tx data 41 (%u Bytes): %s\n", sizeof(tx_data1), tx_data1);
    diag_printf ("    Rx data 1 : %s\n", rx_data1);
    diag_printf ("    Tx data 42 (%u Bytes): %s\n", sizeof(tx_data2), tx_data2);
    diag_printf ("    Rx data 2 : %s\n", rx_data2);

    if (memcmp (rx_data1, rx_data2, sizeof (tx_data1)) != 0) {
        errors++;
        diag_printf("Simple transfer loopback failed - mismatched data.\n");
        errors++;
    }
}


//---------------------------------------------------------------------------
// Run all SPI interface loopback tests.


void run_tests (void)
{
    bool polled = true;
    diag_printf ("Running Freescale Kinetis/MPC5xxx DSPI driver loopback tests.\n");

    diag_printf ("\nPolled\n");
    run_test_tick (polled, 1024);
    run_test_1 (polled, &tx_data0[3], 4);
    run_test_2 (polled);
    run_test_3 (polled);
    run_test_4 (polled);

    polled = false;
    diag_printf ("\nInterrupt driven.\n");
    run_test_tick (polled,2048);
    run_test_1 (polled, &tx_data0[7], 10);
    run_test_2 (polled);
    run_test_3 (polled);
    run_test_4 (polled);

    if(errors)
        CYG_TEST_FAIL("Errors detected");
    else
        CYG_TEST_PASS_FINISH ("Loopback tests ran OK");
}

//---------------------------------------------------------------------------
// User startup - tests are run in their own thread.

void cyg_user_start(void)
{
    CYG_TEST_INIT();
    cyg_thread_create(
        10,                                   // Arbitrary priority
        (cyg_thread_entry_t*) run_tests,      // Thread entry point
        0,                                    //
        "test_thread",                        // Thread name
        &stack[0],                            // Stack
        CYGNUM_HAL_STACK_SIZE_TYPICAL,        // Stack size
        &thread_handle,                       // Thread handle
        &thread_data                          // Thread data structure
    );
    cyg_thread_resume(thread_handle);
    cyg_scheduler_start();
}

//=============================================================================
