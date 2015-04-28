//===========================================================================
//
//      wf_scalb.c
//
//      Part of the standard mathematical function library
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
// Contributors: visar, ilijak
// Date:        2012-03-08
// Purpose:
// Description:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

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
 * wrapper scalbf(float x, float fn) is provide for
 * passing various standard test suite. One
 * should use scalbn() instead.
 */

// CONFIGURATION


#include "mathincl/fdlibm.h"
#ifndef CYGSEM_LIBM_COMPAT_IEEE_ONLY
# include <errno.h>
#endif

#ifdef __STDC__
#ifdef CYGFUN_LIBM_SVID3_scalb
	float scalbf(float x, float fn)		/* wrapper scalbf */
#else
	float scalbf(float x, int fn)		/* wrapper scalbf */
#endif
#else
	float scalbf(x,fn)			/* wrapper scalbf */
#ifdef CYGFUN_LIBM_SVID3_scalb
	float x,fn;
#else
	float x; int fn;
#endif
#endif
{
#ifdef CYGSEM_LIBM_COMPAT_IEEE_ONLY
	return __ieee754_scalbf(x,fn);
#else
    double z;
    z = __ieee754_scalbf(x,fn);
    if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_IEEE) return z;
    if(!(finitef(z)||isnanf(z))&&finitef(x)) {
        return __kernel_standard(x,(double)fn,32); /* scalb overflow */
    }
    if(z==0.0&&z!=x) {
        return __kernel_standard(x,(double)fn,33); /* scalb underflow */
    }
#ifdef CYGFUN_LIBM_SVID3_scalb
    if(!finitef(fn)) errno = ERANGE;
#endif
    return z;
#endif
}

#endif // ifdef CYGPKG_LIBM

// EOF wf_scalb.c
