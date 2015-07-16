//==========================================================================
//
//      redboot_linux_boot.c
//
//      RedBoot command to boot Linux on ARM platforms
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
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
//####OTHERCOPYRIGHTBEGIN####
//
//  The structure definitions below are taken from include/asm-arm/setup.h in
//  the Linux kernel, Copyright (C) 1997-1999 Russell King. Their presence
//  here is for the express purpose of communication with the Linux kernel
//  being booted and is considered 'fair use' by the original author and
//  are included with his permission.
//
//####OTHERCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Piotr Skrzypek
// Date:         2012-06-08
// Purpose:      
// Description:  Based on ARM code
//              
// This code is part of RedBoot (tm).
//
//####DESCRIPTIONEND####
//
//==========================================================================

#include <pkgconf/hal.h>
#include <redboot.h>
#include <cyg/hal/hal_intr.h>
#include <cyg/hal/hal_cache.h>
#include <cyg/hal/hal_io.h>

#ifdef CYGPKG_IO_ETH_DRIVERS
#include <cyg/io/eth/eth_drv.h>
#endif

// Exported CLI function(s)
static void do_exec(int argc, char *argv[]);
RedBoot_cmd("exec", 
            "Execute an image", 
            "[-w timeout] [-b <load addr> [-l <length>]]\n"
            "        [-t <target> ] [<entry_point>]",
            do_exec
    );

static void do_exec(int argc, char *argv[]) {

	cyg_uint32 wait_time;
	cyg_bool wait_time_set;
	cyg_uint32 base_addr;
	cyg_bool base_addr_set;
	cyg_uint32 length;
	cyg_bool length_set;
	cyg_uint32 target;
	cyg_bool target_set;
	cyg_uint32 entry;

	extern char __tramp_start__[], __tramp_end__[];
	cyg_uint32 oldints;

	// Parse parameters
	struct option_info opts[4];
	init_opts(&opts[0], 'w', true, OPTION_ARG_TYPE_NUM, (void **)&wait_time, (bool *)&wait_time_set, "wait timeout");
	init_opts(&opts[1], 'b', true, OPTION_ARG_TYPE_NUM, (void **)&base_addr, (bool *)&base_addr_set, "base address");
	init_opts(&opts[2], 'l', true, OPTION_ARG_TYPE_NUM, (void **)&length, (bool *)&length_set, "length");
	init_opts(&opts[3], 't', true, OPTION_ARG_TYPE_NUM, (void **)&target, (bool *)&target_set, "target address");
	int num_opts = 4;

	if(!scan_opts(argc, argv, 1, opts, num_opts, (void *)&entry, OPTION_ARG_TYPE_NUM, "starting address")) {
		return;
	}

	// Timeout is optional
	wait_time = wait_time_set ? wait_time : 0;

	// Base address is mandatory
	if(!base_addr_set) {
		diag_printf("Use -b option to provide base address\n");
		return;
	}

	// Length is mandatory
	if(!length_set) {
		diag_printf("Use -l option to provide size of the binary\n");
		return;
	}
	length = (length + 3) & ~0x03;

	// Target address is mandatory
	if(!target_set) {
		diag_printf("Use -t option to provide target address\n");
		return;
	}

	// Optionally wait
	if(wait_time_set) {
		int script_timeout_ms = wait_time * 1000;
#ifdef CYGFUN_REDBOOT_BOOT_SCRIPT
		unsigned char *hold_script = script;
		script = (unsigned char *) 0;
#endif
		diag_printf("About to start execution of image at %p, entry point %p - abort with ^C within %d seconds\n",
				(void *)target, (void *)entry, wait_time);
		while(script_timeout_ms >= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT) {
			char line[8];
			int res = _rb_gets(line, sizeof(line), CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT);
			if(res == _GETS_CTRLC) {
#ifdef CYGFUN_REDBOOT_BOOT_SCRIPT
				script = hold_script;
#endif
				return;
			}
			script_timeout_ms -= CYGNUM_REDBOOT_CLI_IDLE_TIMEOUT;
		}
	}

#ifdef CYGPKG_IO_ETH_DRIVERS
	eth_drv_stop();
#endif

	HAL_DISABLE_INTERRUPTS(oldints);
	HAL_DCACHE_SYNC();
	HAL_ICACHE_DISABLE();
	HAL_DCACHE_DISABLE();
	HAL_DCACHE_SYNC();
	HAL_ICACHE_INVALIDATE_ALL();
	HAL_DCACHE_INVALIDATE_ALL();

	// Copy the trampoline code below
	memcpy((char*)CYGPKG_REDBOOT_OPENRISC_TRAMPOLINE_ADDRESS, __tramp_start__, __tramp_end__ - __tramp_start__);

	// Jump to the trampoline. Pass arguments according to ABI
	void (*trampoline)(cyg_uint32 bin_start, cyg_uint32 bin_end, cyg_uint32 dest_base, cyg_uint32 entry);
	trampoline = CYGPKG_REDBOOT_OPENRISC_TRAMPOLINE_ADDRESS;
	trampoline(base_addr, base_addr + length, target, entry);

	// This code is executed from the trampoline address. Trampoline never returns.
	asm volatile (
		"__tramp_start__:\n"
		"1: l.sfeq  r3, r4\n"
		"   l.bf    2f\n"
		"   l.lwz   r13,0x00(r3)\n"
		"   l.sw    0x00(r5), r13\n"
		"   l.addi  r3, r3, 4\n"
		"   l.j     1b\n"
		"   l.addi  r5, r5, 4\n"
		"2: l.jr    r6\n"
		"   l.nop\n"
		"__tramp_end__:\n"
		: /* no output registers */
		: /* no input registers */
		: /* no clobbered registers */
	);


}

// EOF redboot_linux_exec.c
