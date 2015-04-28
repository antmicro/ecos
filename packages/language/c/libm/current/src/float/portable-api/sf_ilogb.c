//===========================================================================
//
//      sf_ilogb.c
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


/* sf_ilogb.c -- float version of s_ilogb.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

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


#include <limits.h>
#include "mathincl/fdlibm.h"

#ifdef __STDC__
	int ilogbf(float x)
#else
	int ilogbf(x)
	float x;
#endif
{
	__int32_t hx,ix;

	hx = CYG_LIBM_WORD(x);
	hx &= 0x7fffffff;
	if(FLT_UWORD_IS_ZERO(hx))
	    return 0x80000001;	/* ilogb(0) = special case error */
	if(FLT_UWORD_IS_SUBNORMAL(hx)) {
	    for (ix = -126,hx<<=8; hx>0; hx<<=1) ix -=1;
	    return ix;
	}
	#if FP_ILOGBNAN != INT_MAX
	else if (FLT_UWORD_IS_NAN(hx)) return FP_ILOGBNAN;	/* NAN */
	#endif
	else if (!FLT_UWORD_IS_FINITE(hx)) return INT_MAX;
	else
		return (hx>>=23)-127;
}

#endif // ifdef CYGPKG_LIBM

// EOF sf_ilogb.c
