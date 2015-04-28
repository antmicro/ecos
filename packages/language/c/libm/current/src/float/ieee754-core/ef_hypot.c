//===========================================================================
//
//      ef_hypot.c
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


/* ef_hypot.c -- float version of e_hypot.c.
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
	float __ieee754_hypotf(float x, float y)
#else
	float __ieee754_hypotf(x,y)
	float x, y;
#endif
{
	float a=x,b=y,t1,t2,y1,y2,w;
	__int32_t j,k,ha,hb;

	ha = CYG_LIBM_WORD(x);
	ha &= 0x7fffffffL;
	hb = CYG_LIBM_WORD(y);
	hb &= 0x7fffffffL;
	if(hb > ha) {a=y;b=x;j=ha; ha=hb;hb=j;} else {a=x;b=y;}
	SET_FLOAT_WORD(a,ha);	/* a <- |a| */
	SET_FLOAT_WORD(b,hb);	/* b <- |b| */
	if((ha-hb)>0xf000000L) {return a+b;} /* x/y > 2**30 */
	k=0;
	if(ha > 0x58800000L) {	/* a>2**50 */
	   if(!FLT_UWORD_IS_FINITE(ha)) {	/* Inf or NaN */
	       w = a+b;			/* for sNaN */
	       if(FLT_UWORD_IS_INFINITE(ha)) w = a;
	       if(FLT_UWORD_IS_INFINITE(hb)) w = b;
	       return w;
	   }
	   /* scale a and b by 2**-68 */
	   ha -= 0x22000000L; hb -= 0x22000000L;	k += 68;
	   SET_FLOAT_WORD(a,ha);
	   SET_FLOAT_WORD(b,hb);
	}
	if(hb < 0x26800000L) {	/* b < 2**-50 */
	    if(FLT_UWORD_IS_ZERO(hb)) {
	        return a;
	    } else if(FLT_UWORD_IS_SUBNORMAL(hb)) {
		SET_FLOAT_WORD(t1,0x7e800000L);	/* t1=2^126 */
		b *= t1;
		a *= t1;
		k -= 126;
	    } else {		/* scale a and b by 2^68 */
	        ha += 0x22000000;	/* a *= 2^68 */
		hb += 0x22000000;	/* b *= 2^68 */
		k -= 68;
		SET_FLOAT_WORD(a,ha);
		SET_FLOAT_WORD(b,hb);
	    }
	}
    /* medium size a and b */
	w = a-b;
	if (w>b) {
	    SET_FLOAT_WORD(t1,ha&0xfffff000L);
	    t2 = a-t1;
	    w  = __ieee754_sqrtf(t1*t1-(b*(-b)-t2*(a+t1)));
	} else {
	    a  = a+a;
	    SET_FLOAT_WORD(y1,hb&0xfffff000L);
	    y2 = b - y1;
	    SET_FLOAT_WORD(t1,ha+0x00800000L);
	    t2 = a - t1;
	    w  = __ieee754_sqrtf(t1*y1-(w*(-w)-(t1*y2+t2*b)));
	}
	if(k!=0) {
	    SET_FLOAT_WORD(t1,0x3f800000L+(k<<23));
	    return t1*w;
	} else return w;
}
#endif // ifdef CYGPKG_LIBM

// EOF ef_hypot.c
