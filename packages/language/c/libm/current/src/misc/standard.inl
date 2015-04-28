//===========================================================================
//
//      standard.inl
//
//      
//
//===========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2013 Free Software Foundation, Inc.
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
// Author(s):    jlarmour
// Contributors: ilijak
// Date:         1998-02-13
// Purpose:     
// Description:
//
// Usage:        This file is included by standard.c and standard_float.c
// Note:         The contents of this file originally existed in standard.c
//               with addition of single precision math library the body was
//               transfered here, while standard.c and standard_float.c include
//               and instantiate this file for double and single precision floats
//               respectively.
//
//####DESCRIPTIONEND####
//
//===========================================================================

/* @(#)k_standard.c 1.3 95/01/18 */
/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunSoft, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 *
 */

// CONFIGURATION

#include <pkgconf/libm.h>   // Configuration header

// Include the Math library?
#ifdef CYGPKG_LIBM

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <cyg/infra/cyg_trac.h>    // Tracing macros

#include <math.h>                  // Main header for math library
#include "mathincl/fdlibm.h"       // Internal header for math library

#include <cyg/error/codes.h>       // standard error codes

#ifndef CYGSEM_LIBM_COMPAT_IEEE_ONLY
#include <errno.h>
#else
static int errno; // this whole file won't be used if we're IEEE only, but
                  // doing this keeps the compiler happy
#endif

#ifdef CYGSEM_LIBM_USE_STDERR

#include <stdio.h>
#define WRITE2(u,v)     fputs(u, stderr)

#else

#define WRITE2(u,v)     0

#endif // ifdef CYGSEM_LIBM_USE_STDERR


// GLOBALS

static const CYG_STANDARD_FLOAT_T zero = (CYG_STANDARD_FLOAT_T)0.0;

// FUNCTIONS

/* 
 * Standard conformance (non-IEEE) on exception cases.
 * Mapping:
 *      1 -- acos(|x|>1)
 *      2 -- asin(|x|>1)
 *      3 -- atan2(+-0,+-0)
 *      4 -- hypot overflow
 *      5 -- cosh overflow
 *      6 -- exp overflow
 *      7 -- exp underflow
 *      8 -- y0(0)
 *      9 -- y0(-ve)
 *      10-- y1(0)
 *      11-- y1(-ve)
 *      12-- yn(0)
 *      13-- yn(-ve)
 *      14-- lgamma(finite) overflow
 *      15-- lgamma(-integer)
 *      16-- log(0)
 *      17-- log(x<0)
 *      18-- log10(0)
 *      19-- log10(x<0)
 *      20-- pow(0.0,0.0)
 *      21-- pow(x,y) overflow
 *      22-- pow(x,y) underflow
 *      23-- pow(0,negative) 
 *      24-- pow(neg,non-integral)
 *      25-- sinh(finite) overflow
 *      26-- sqrt(negative)
 *      27-- fmod(x,0)
 *      28-- remainder(x,0)
 *      29-- acosh(x<1)
 *      30-- atanh(|x|>1)
 *      31-- atanh(|x|=1)
 *      32-- scalb overflow
 *      33-- scalb underflow
 *      34-- j0(|x|>X_TLOSS)
 *      35-- y0(x>X_TLOSS)
 *      36-- j1(|x|>X_TLOSS)
 *      37-- y1(x>X_TLOSS)
 *      38-- jn(|x|>X_TLOSS, n)
 *      39-- yn(x>X_TLOSS, n)
 *      40-- gamma(finite) overflow
 *      41-- gamma(-integer)
 *      42-- pow(NaN,0.0)
 *      43-- ldexp overflow
 *      44-- ldexp underflow
 */

#ifdef FLOAT_SINGLE_PRECISION
float
__kernel_standard_float(float x, float y, int type) 
{
        struct exceptionf exc;

#else
double
__kernel_standard(double x, double y, int type) 
{
        struct exception exc;

#endif

#ifdef CYGSEM_LIBM_USE_STDERR
        (void) fflush(stdout);
#endif
        exc.arg1 = x;
        exc.arg2 = y;
        switch(type) {
            case 1:
                /* acos(|x|>1) */
                exc.type = DOMAIN;
                exc.name = "acos" CYGSTR_FLOAT_SUFFIX;
#ifdef CYGOPT_LIBM_COMPAT_POSIX_IEEE
                exc.retval = (x-x)/(x-x);
#else
                exc.retval = zero;
#endif
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("acos: DOMAIN error\n", 19);
                  }
                  errno = EDOM;
                }
                break;
            case 2:
                /* asin(|x|>1) */
                exc.type = DOMAIN;
                exc.name = "asin" CYGSTR_FLOAT_SUFFIX;
#ifdef CYGOPT_LIBM_COMPAT_POSIX_IEEE
                exc.retval = (x-x)/(x-x);
#else
                exc.retval = zero;
#endif
                if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("asin: DOMAIN error\n", 19);
                  }
                  errno = EDOM;
                }
                break;
            case 3:
                /* atan2(+-0,+-0) */
                exc.arg1 = y;
                exc.arg2 = x;
                exc.type = DOMAIN;
                exc.name = "atan2" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero;
                if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if(cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("atan2: DOMAIN error\n", 20);
                      }
                  errno = EDOM;
                }
                break;
            case 4:
                /* hypot(finite,finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "hypot" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 5:
                /* cosh(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "cosh" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 6:
                /* exp(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "exp" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 7:
                /* exp(finite) underflow */
                exc.type = UNDERFLOW;
                exc.name = "exp" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 8:
                /* y0(0) = -inf */
                exc.type = DOMAIN;      /* should be SING for IEEE */
                exc.name = "y0" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("y0: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 9:
                /* y0(x<0) = NaN */
                exc.type = DOMAIN;
                exc.name = "y0" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("y0: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 10:
                /* y1(0) = -inf */
                exc.type = DOMAIN;      /* should be SING for IEEE */
                exc.name = "y1" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("y1: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 11:
                /* y1(x<0) = NaN */
                exc.type = DOMAIN;
                exc.name = "y1" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("y1: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 12:
                /* yn(n,0) = -inf */
                exc.type = DOMAIN;      /* should be SING for IEEE */
                exc.name = "yn" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("yn: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 13:
                /* yn(x<0) = NaN */
                exc.type = DOMAIN;
                exc.name = "yn" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("yn: DOMAIN error\n", 17);
                      }
                  errno = EDOM;
                }
                break;
            case 14:
                /* lgamma(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "lgamma" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 15:
                /* lgamma(-integer) or lgamma(0) */
                exc.type = SING;
                exc.name = "lgamma" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("lgamma: SING error\n", 19);
                      }
                  errno = EDOM;
                }
                break;
            case 16:
                /* log(0) */
                exc.type = SING;
                exc.name = "log" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("log: SING error\n", 16);
                      }
                  errno = EDOM;
                }
                break;
            case 17:
                /* log(x<0) */
                exc.type = DOMAIN;
                exc.name = "log" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
#ifdef CYGOPT_LIBM_COMPAT_POSIX_IEEE
                  exc.retval = (x-x)/(x-x);
#else
                  exc.retval = -HUGE_VAL;
#endif
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("log: DOMAIN error\n", 18);
                      }
                  errno = EDOM;
                }
                break;
            case 18:
                /* log10(0) */
                exc.type = SING;
                exc.name = "log10" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("log10: SING error\n", 18);
                      }
                  errno = EDOM;
                }
                break;
            case 19:
                /* log10(x<0) */
                exc.type = DOMAIN;
                exc.name = "log10" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = -HUGE;
                else
#ifdef CYGOPT_LIBM_COMPAT_POSIX_IEEE
                  exc.retval = (x-x)/(x-x);
#else
                  exc.retval = -HUGE_VAL;
#endif
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("log10: DOMAIN error\n", 20);
                      }
                  errno = EDOM;
                }
                break;
            case 20:
                /* pow(0.0,0.0) */
                /* error only if cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID */
                exc.type = DOMAIN;
                exc.name = "pow" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() != CYGNUM_LIBM_COMPAT_SVID) exc.retval = 1.0;
                else if (!MATHERR(&exc)) {
                        (void) WRITE2("pow(0,0): DOMAIN error\n", 23);
                        errno = EDOM;
                }
                break;
            case 21:
                /* pow(x,y) overflow */
                exc.type = OVERFLOW;
                exc.name = "pow" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                  exc.retval = HUGE;
                  y *= 0.5;
                  if(x<zero&&rint(y)!=y) exc.retval = -HUGE;
                } else {
                  exc.retval = HUGE_VAL;
                  y *= 0.5;
                  if(x<zero&&rint(y)!=y) exc.retval = -HUGE_VAL;
                }
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 22:
                /* pow(x,y) underflow */
                exc.type = UNDERFLOW;
                exc.name = "pow" CYGSTR_FLOAT_SUFFIX;
                exc.retval =  zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 23:
                /* 0**neg */
                exc.type = DOMAIN;
                exc.name = "pow" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) 
                  exc.retval = zero;
                else
                  exc.retval = -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("pow(0,neg): DOMAIN error\n", 25);
                      }
                  errno = EDOM;
                }
                break;
            case 24:
                /* neg**non-integral */
                exc.type = DOMAIN;
                exc.name = "pow" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) 
                    exc.retval = zero;
                else 
                    exc.retval = zero/zero;     /* X/Open allow NaN */
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX) 
                   errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("neg**non-integral: DOMAIN error\n", 32);
                      }
                  errno = EDOM;
                }
                break;
            case 25:
                /* sinh(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "sinh" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = ( (x>zero) ? HUGE : -HUGE);
                else
                  exc.retval = ( (x>zero) ? HUGE_VAL : -HUGE_VAL);
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 26:
                /* sqrt(x<0) */
                exc.type = DOMAIN;
                exc.name = "sqrt" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = zero;
                else
                  exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("sqrt: DOMAIN error\n", 19);
                      }
                  errno = EDOM;
                }
                break;
            case 27:
                /* fmod(x,0) */
                exc.type = DOMAIN;
                exc.name = "fmod" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                    exc.retval = x;
                else
                    exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("fmod:  DOMAIN error\n", 20);
                  }
                  errno = EDOM;
                }
                break;
            case 28:
                /* remainder(x,0) */
                exc.type = DOMAIN;
                exc.name = "remainder" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("remainder: DOMAIN error\n", 24);
                  }
                  errno = EDOM;
                }
                break;
            case 29:
                /* acosh(x<1) */
                exc.type = DOMAIN;
                exc.name = "acosh" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("acosh: DOMAIN error\n", 20);
                  }
                  errno = EDOM;
                }
                break;
            case 30:
                /* atanh(|x|>1) */
                exc.type = DOMAIN;
                exc.name = "atanh" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero/zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("atanh: DOMAIN error\n", 20);
                  }
                  errno = EDOM;
                }
                break;
            case 31:
                /* atanh(|x|=1) */
                exc.type = SING;
                exc.name = "atanh" CYGSTR_FLOAT_SUFFIX;
                exc.retval = x/zero;    /* sign(x)*inf */
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                    (void) WRITE2("atanh: SING error\n", 18);
                  }
                  errno = EDOM;
                }
                break;
            case 32:
                /* scalb overflow; SVID also returns +-HUGE_VAL */
                exc.type = OVERFLOW;
                exc.name = "scalb" CYGSTR_FLOAT_SUFFIX;
                exc.retval = x > zero ? HUGE_VAL : -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 33:
                /* scalb underflow */
                exc.type = UNDERFLOW;
                exc.name = "scalb" CYGSTR_FLOAT_SUFFIX;
                exc.retval = copysign(zero,x);
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 34:
                /* j0(|x|>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "j0" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 35:
                /* y0(x>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "y0" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 36:
                /* j1(|x|>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "j1" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 37:
                /* y1(x>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "y1" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 38:
                /* jn(|x|>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "jn" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 39:
                /* yn(x>X_TLOSS) */
                exc.type = TLOSS;
                exc.name = "yn" CYGSTR_FLOAT_SUFFIX;
                exc.retval = zero;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                        errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                                (void) WRITE2(exc.name, 2);
                                (void) WRITE2(": TLOSS error\n", 14);
                        }
                        errno = ERANGE;
                }        
                break;
            case 40:
                /* gamma(finite) overflow */
                exc.type = OVERFLOW;
                exc.name = "gamma" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                  errno = ERANGE;
                }
                break;
            case 41:
                /* gamma(-integer) or gamma(0) */
                exc.type = SING;
                exc.name = "gamma" CYGSTR_FLOAT_SUFFIX;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID)
                  exc.retval = HUGE;
                else
                  exc.retval = HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = EDOM;
                else if (!MATHERR(&exc)) {
                  if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID) {
                        (void) WRITE2("gamma: SING error\n", 18);
                      }
                  errno = EDOM;
                }
                break;
            case 42:
                /* pow(NaN,0.0) */
                /* error only if cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_SVID & CYGNUM_LIBM_COMPAT_XOPEN */
                exc.type = DOMAIN;
                exc.name = "pow" CYGSTR_FLOAT_SUFFIX;
                exc.retval = x;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_IEEE ||
                    cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX) exc.retval = 1.0;
                else if (!MATHERR(&exc)) {
                        errno = EDOM;
                }
                break;
            case 43:
                /* ldexp overflow; SVID also returns +-HUGE_VAL */
                exc.type = OVERFLOW;
                exc.name = "ldexp" CYGSTR_FLOAT_SUFFIX;
                exc.retval = x > zero ? HUGE_VAL : -HUGE_VAL;
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
            case 44:
                /* ldexp underflow */
                exc.type = UNDERFLOW;
                exc.name = "ldexp" CYGSTR_FLOAT_SUFFIX;
                exc.retval = copysign(zero,x);
                if (cyg_libm_get_compat_mode() == CYGNUM_LIBM_COMPAT_POSIX)
                  errno = ERANGE;
                else if (!MATHERR(&exc)) {
                        errno = ERANGE;
                }
                break;
        }
        return exc.retval; 
}

#endif // ifdef CYGPKG_LIBM

// EOF standard.c
