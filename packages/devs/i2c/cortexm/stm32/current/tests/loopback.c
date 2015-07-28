//=============================================================================
//
//      loopback.c
//
//      Standalone I2C loopback test.
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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
// Author(s):   Michael Jones
// Date:        2013-03-31
// Purpose:     STM32 I2C loopback test
// Description: Standalone I2C loopback test.
// Usage:       Compile as a standalone application.
//
//####DESCRIPTIONEND####
//
//=============================================================================

//=============================================================================
// This is a quick loopback test for the STM32 I2C driver.  It works with the
// STM3240G-EVAL board with the JP26 shorted. There are still problems with 
// the hardware itself. It appears that some device is is performing clock
// stretching preventing fast mode operation. Some registers don't read properly
// although a 3 device PMBus board attached with cables using addresses 0x30 0x32
// and 0x33 seem to read and write properly. You can run this test with both
// interrupt and polled mode and should get the same results.
//
// Also note that the origonal code was for a STM321X board and this test case
// was never run against that board after modifying it for the latest codebase.
//=============================================================================

#include <cyg/io/i2c_stm32.h>

void write_byte(cyg_uint8 address, cyg_uint8 command, cyg_uint8 data)
{
    cyg_i2c_device device = {                        \
            .i2c_bus        = &i2c_bus1,		     \
            .i2c_address    = address,               \
            .i2c_flags      = 0,                     \
            .i2c_delay      = 100000                 \
        };
    cyg_uint8 buffer[2];
    buffer[0] = command;
    buffer[1] = data;

    cyg_i2c_transaction_begin(&device);
	if(!cyg_i2c_transaction_tx(&device, true, &buffer[0], 2, true)) {
        diag_printf("Write Byte: fail TX.\n");
	}
    cyg_i2c_transaction_end(&device);
}

cyg_uint8 read_byte(cyg_uint8 address, cyg_uint8 command)
{
    cyg_i2c_device device = {                        \
            .i2c_bus        = &i2c_bus1,		     \
            .i2c_address    = address,               \
            .i2c_flags      = 0,                     \
            .i2c_delay      = 100000                 \
    };
    cyg_uint8 buffer[1];
    cyg_uint8 input[1];
    buffer[0] = command;

    cyg_i2c_transaction_begin(&device);
	if(!cyg_i2c_transaction_tx(&device, true, &buffer[0], 1, false)) {
        diag_printf("Read Byte: fail TX.\n");
	} else if(!cyg_i2c_transaction_rx(&device, true, &input[0], 1, true, true)) {
        diag_printf("Read Byte: fail RX.\n");
    }
    cyg_i2c_transaction_end(&device);

    return input[0];
}

int main(void)
{
    int retval = 0;
    cyg_uint8 data;

    data = read_byte(0x41, 0x00);
    diag_printf("Touch Device Id Upper is 0x%x and should be 0x08\n", data);
    data = read_byte(0x41, 0x01);
    diag_printf("Touch Device Id Lower is 0x%x and should be 0x11\n", data);
    data = read_byte(0x41, 0x02);
    diag_printf("Touch Device Revision is 0x%x and should be 0x03\n", data);

    write_byte(0x41, 0x04, 0x00);
    data = read_byte(0x41, 0x04);
    diag_printf("Touch CTRL2 is 0x%x and should be 0x00\n", data);
    write_byte(0x41, 0x04, 0x05);
    data = read_byte(0x41, 0x04);
    diag_printf("Touch CTRL2 is 0x%x and should be 0x05\n", data);
    write_byte(0x41, 0x04, 0x0A);
    data = read_byte(0x41, 0x04);
    diag_printf("Touch CTRL2 is 0x%x and should be 0x0A\n", data); // Will fail and return 0x02
    write_byte(0x41, 0x04, 0x00);
    data = read_byte(0x41, 0x04);
    diag_printf("Touch CTRL2 is 0x%x and should be 0x00\n", data);

    return retval;
}
