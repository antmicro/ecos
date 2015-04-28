//===========================================================================
//
//      sf_logb.c
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

/* 2009 for Newlib:  Sun's sf_ilogb.c converted to be sf_logb.c.  */
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

/* float logb(float x)
 * return the binary exponent of non-zero x
 * logbf(0) = -inf, raise divide-by-zero floating point exception
 * logbf(+inf|-inf) = +inf (no signal is raised)
 * logbf(NaN) = NaN (no signal is raised)
 * Per C99 recommendation, a NaN argument is returned unchanged.
 */


#include "mathincl/fdlibm.h"

float
#ifdef __STDC__
logbf(float x)
#else
logbf(x)
float x;
#endif
{
	__int32_t hx,ix;

	hx = CYG_LIBM_WORD(x);
	hx &= 0x7fffffff;
	if(FLT_UWORD_IS_ZERO(hx))  {
		float  xx;
		/* arg==0:  return -inf and raise divide-by-zero exception */
		SET_FLOAT_WORD(xx,hx);	/* +0.0 */
		return -1./xx;	/* logbf(0) = -inf */
		}
	if(FLT_UWORD_IS_SUBNORMAL(hx)) {
	    for (ix = -126,hx<<=8; hx>0; hx<<=1) ix -=1;
	    return (float) ix;
	}
	else if (FLT_UWORD_IS_INFINITE(hx)) return HUGE_VALF;	/* x==+|-inf */
	else if (FLT_UWORD_IS_NAN(hx)) return x;
	else return (float) ((hx>>23)-127);
}

#endif // ifdef CYGPKG_LIBM

// EOF sf_logb.c
