//========================================================================
//
//      wf_pow.c
//
//      Part of the standard mathematical function library
//
//========================================================================
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
//========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     jlarmour
// Contributors: visar, ilijak
// Date:          1998-02-13
// Purpose:
// Description:
// Usage:
//
//####DESCRIPTIONEND####
//
//========================================================================

// CONFIGURATION

#include <pkgconf/libm.h>   // Configuration header

// Include the Math library?
#ifdef CYGPKG_LIBM

// Derived from code with the following copyright


/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * wrapper powf(x,y) return x**y
 */

#include "mathincl/fdlibm.h"

	float powf(float x, float y)	/* wrapper powf */
{
#ifdef CYGSEM_LIBM_COMPAT_IEEE_ONLY
	return  __ieee754_powf(x,y);
#else
	float z;
    z=__ieee754_powf(x,y);
    if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_IEEE||
        isnan(y))
        return z;
    if(isnan(x)) {
        if(y==0.0)
            return __kernel_standard(x,y,42); /* pow(NaN,0.0) */
        else
            return z;
    }
    if(x==0.0){
        if(y==0.0)
            return __kernel_standard(x,y,20); /* pow(0.0,0.0) */
        if(finitef(y)&&y<0.0)
            return __kernel_standard(x,y,23); /* pow(0.0,negative) */
    return z;
    }
    if(!finitef(z)) {
        if(finitef(x)&&finitef(y)) {
            if(isnan(z))
                return __kernel_standard(x,y,24);/* pow neg**non-int */
            else
                return __kernel_standard(x,y,21); /* pow overflow */
        }
    }
    if(z==0.0&&finitef(x)&&finitef(y))
        return __kernel_standard(x,y,22); /* pow underflow */
    return z;
#endif
}

#endif // ifdef CYGPKG_LIBM

// EOF wf_pow.c
