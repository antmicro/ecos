// CONFIGURATION

#include <pkgconf/libm.h>   // Configuration header


// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/testcase.h>    // Test infrastructure
#include <math.h>                  // Header for this package
#include <sys/ieeefp.h>            // Cyg_libm_ieee_double_shape_type
#include "vectors/vector_support_float.h"// extra support for math tests

#include "vectors/hypotf.h"

// FUNCTIONS

static void
test( CYG_ADDRWORD data )
{
    cyg_ucount32 vec_size;
    cyg_bool ret;

    vec_size = sizeof(hypotf_vec) / sizeof(Cyg_libm_test_float_vec_t);
    ret = doTestVecFloat( (CYG_ADDRWORD) &hypotf, CYG_LIBM_TEST_VEC_FLOAT,
                     CYG_LIBM_TEST_VEC_FLOAT, CYG_LIBM_TEST_VEC_FLOAT,
                     &hypotf_vec[0], vec_size );

    if (ret==true)
    {
        CYG_TEST_PASS("hypotf() is stable");
    } // if
    else
    {
        CYG_TEST_FAIL("hypotf() failed tests");
    } // else

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for Math "
                    "library hypotf() function");
} // test()


int
main(int argc, char *argv[])
{
    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for Math library "
                  "hypotf() function");

    START_TEST( test );

    CYG_TEST_PASS_FINISH("Testing is not applicable to this configuration");

} // main()


// EOF log10.c
