//===========================================================================
//
//      ef_scalb.c
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


/* ef_scalb.c -- float version of e_scalb.c.
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

#ifdef CYGFUN_LIBM_SVID3_scalb
#ifdef __STDC__
    float __ieee754_scalbf(float x, float fn)
#else
    float __ieee754_scalbf(float x, int fn)
#endif
#else
#ifdef __STDC__
	float __ieee754_scalbf(float x, float fn)
#else
	float __ieee754_scalbf(x,fn)
	float x, fn;
#endif
#endif
{
#ifdef CYGFUN_LIBM_SVID3_scalb
	return scalbnf(x,fn);
#else
	if (isnan(x)||isnan(fn)) return x*fn;
	if (!finitef(fn)) {
	    if(fn>(float)0.0) return x*fn;
	    else       return x/(-fn);
	}
	if (rintf(fn)!=fn) return (fn-fn)/(fn-fn);
#if INT_MAX > 65000
	if ( fn > (float)65000.0) return scalbnf(x, 65000);
	if (-fn > (float)65000.0) return scalbnf(x,-65000);
#else
	if ( fn > (float)32000.0) return scalbnf(x, 32000);
	if (-fn > (float)32000.0) return scalbnf(x,-32000);
#endif
	return scalbnf(x,(int)fn);
#endif
}

#endif // ifdef CYGPKG_LIBM

// EOF e_scalb.c
