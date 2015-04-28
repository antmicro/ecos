//===========================================================================
//
//      sf_rint.c
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


/* sf_rint.c -- float version of s_rint.c.
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


#include "mathincl/fdlibm.h"

#ifdef __STDC__
static const float
#else
static float
#endif
TWO23[2]={
  8.3886080000e+06, /* 0x4b000000 */
 -8.3886080000e+06, /* 0xcb000000 */
};

#ifdef __STDC__
	float rintf(float x)
#else
	float rintf(x)
	float x;
#endif
{
	__int32_t i0,j0,sx;
	__uint32_t i,i1,ix;
	float t;
	volatile float w;
	i0 = CYG_LIBM_WORD(x);
	sx = (i0>>31)&1;
	ix = (i0&0x7fffffff);
	j0 = (ix>>23)-0x7f;
	if(j0<23) {
	    if(FLT_UWORD_IS_ZERO(ix))
	        return x;
	    if(j0<0) {
		i1 = (i0&0x07fffff);
		i0 &= 0xfff00000;
		i0 |= ((i1|-i1)>>9)&0x400000;
		SET_FLOAT_WORD(x,i0);
	        w = TWO23[sx]+x;
	        t =  w-TWO23[sx];
		i0 = CYG_LIBM_WORD(t);
		SET_FLOAT_WORD(t,(i0&0x7fffffff)|(sx<<31));
	        return t;
	    } else {
		i = (0x007fffff)>>j0;
		if((i0&i)==0) return x; /* x is integral */
		i>>=1;
		if((i0&i)!=0) i0 = (i0&(~i))|((0x200000)>>j0);
	    }
	} else {
	    if(!FLT_UWORD_IS_FINITE(ix)) return x+x; /* inf or NaN */
	    else
	      return x;		/* x is integral */
	}
	SET_FLOAT_WORD(x,i0);
	w = TWO23[sx]+x;
	return w-TWO23[sx];
}

#endif // ifdef CYGPKG_LIBM

// EOF sf_rint.c
