//===========================================================================
//
//      sqrtf.c
//
//      Test of sqrtf() math library function
//
//===========================================================================
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
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):
// Contributors:  visar, ilijak
// Date:        1998-02-13
// Purpose:
// Description:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

// Declarations for test system:
//
// TESTCASE_TYPE=CYG_TEST_MODULE


// CONFIGURATION

#include <pkgconf/libm.h>   // Configuration header


// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/testcase.h>    // Test infrastructure
#include <math.h>                  // Header for this package
#include <sys/ieeefp.h>            // Cyg_libm_ieee_double_shape_type
#include "vectors/vector_support_float.h"// extra support for math tests

#include "vectors/sqrtf.h"

// FUNCTIONS

#ifdef CYGSEM_LIBM_IEEE_API_INLINE
float Sqrtf(float x)
{
    return sqrtf(x);
}
#endif

static void
test( CYG_ADDRWORD data )
{
    cyg_ucount32 vec_size;
    cyg_bool ret;

    vec_size = sizeof(sqrtf_vec) / sizeof(Cyg_libm_test_float_vec_t);
    ret = doTestVecFloat(
#ifndef CYGSEM_LIBM_IEEE_API_INLINE
                     (CYG_ADDRWORD) &sqrtf,
#else
                     (CYG_ADDRWORD) &Sqrtf,
#endif
                     CYG_LIBM_TEST_VEC_FLOAT,
                     CYG_LIBM_TEST_VEC_NONE, CYG_LIBM_TEST_VEC_FLOAT,
                     &sqrtf_vec[0], vec_size );

    if (ret==true)
    {
        CYG_TEST_PASS("sqrtf() is stable");
    } // if
    else
    {
        CYG_TEST_FAIL("sqrtf() failed tests");
    } // else

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for Math "
                    "library sqrtf() function");
} // test()


int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for Math library "
                  "sqrtf() function");

    START_TEST( test );

    CYG_TEST_PASS_FINISH("Testing is not applicable to this configuration");

} // main()

// EOF sqrtf.c
