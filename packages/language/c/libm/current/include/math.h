#ifndef CYGONCE_LIBM_MATH_H
#define CYGONCE_LIBM_MATH_H
//===========================================================================
//
//      math.h
//
//      Standard mathematical functions conforming to ANSI and other standards
//
//===========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2012, 2013 Free Software Foundation, Inc.
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
// Author(s):   jlarmour
// Contributors:  jlarmour, visar, ilijak
// Date:        1998-02-13
// Purpose:
// Description: Standard mathematical functions. These can be
//              configured to conform to ANSI section 7.5. There are also
//              a number of extensions conforming to IEEE-754 and behaviours
//              compatible with other standards
// Usage:       #include <math.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================

// CONFIGURATION

#include <pkgconf/system.h> // System configuration header
#include <pkgconf/libm.h>   // Configuration header

// Include the Math library?
#ifdef CYGPKG_LIBM

#ifdef CYGBLD_HAL_LIBM_H
# include CYGBLD_HAL_LIBM_H
#endif

// Inline IEEE754 API macros
#define CYGBLD_LIBM_INLINE CYGBLD_FORCE_INLINE

#ifdef CYGSEM_LIBM_IEEE_API_INLINE

#define CYGDCL_LIBM_IEEE_API_INLINE_F(_fun)        \
externC double __ieee754_##_fun(double);           \
CYGBLD_LIBM_INLINE double _fun(double x)           \
{                                                  \
    return __ieee754_##_fun(x);                    \
}                                                  \
externC float __ieee754_##_fun##f(float);          \
CYGBLD_LIBM_INLINE float _fun##f(float x)          \
{                                                  \
    return __ieee754_##_fun##f(x);                 \
}

#define CYGDCL_LIBM_IEEE_API_INLINE_FF(_fun)       \
externC double __ieee754_##_fun(double, double);   \
CYGBLD_LIBM_INLINE double _fun(double x, double y) \
{                                                  \
    return __ieee754_##_fun(x, y);                 \
}                                                  \
externC float __ieee754_##_fun##f(float, float);   \
CYGBLD_LIBM_INLINE float _fun##f(float x, float y) \
{                                                  \
    return __ieee754_##_fun##f(x, y);              \
}

#define CYGDCL_LIBM_IEEE_API_INLINE_IF(_fun)       \
externC double __ieee754_##_fun(int, double);      \
CYGBLD_LIBM_INLINE double _fun(int x, double y)    \
{                                                  \
    return __ieee754_##_fun(x, y);                 \
}                                                  \
externC float __ieee754_##_fun##f(int, float);     \
CYGBLD_LIBM_INLINE float _fun##f(int x, float y)   \
{                                                  \
    return __ieee754_##_fun##f(x, y);              \
}

#define CYGDCL_LIBM_IEEE_API_INLINE_FIP(_fun)      \
externC double __ieee754_##_fun(double, int *);    \
CYGBLD_LIBM_INLINE double _fun(double x, int *y)   \
{                                                  \
    return __ieee754_##_fun(x, y);                 \
}                                                  \
externC float __ieee754_##_fun##f(float, int*);    \
CYGBLD_LIBM_INLINE float _fun##f(float x, int *y)  \
{                                                  \
    return __ieee754_##_fun##f(x, y);              \
}

#else

#define CYGDCL_LIBM_IEEE_API_INLINE_F(_fun) \
externC inline double _fun(double);         \
externC float inline _fun##f(float);

#define CYGDCL_LIBM_IEEE_API_INLINE_FF(_fun) \
externC inline double _fun(double, double);  \
externC inline float _fun##f(float, float);

#define CYGDCL_LIBM_IEEE_API_INLINE_IF(_fun) \
externC inline double _fun(int, double);     \
externC inline float _fun##f(int, float);

#define CYGDCL_LIBM_IEEE_API_INLINE_FIP(_fun) \
externC inline double _fun(double, int *);    \
externC inline float _fun##f(float, int *);

#endif

// INCLUDES

#include <cyg/infra/cyg_type.h>    // Common type definitions and support
#include <float.h>                 // Properties of FP representation on this
                                   // platform
#include <sys/ieeefp.h>            // Cyg_libm_ieee_[double|float]_shape_type

// CONSTANT DEFINITIONS

// Useful constants.

#define MAXFLOAT    3.40282347e+38F

#define M_E         2.7182818284590452354
#define M_LOG2E     1.4426950408889634074
#define M_LOG10E    0.43429448190325182765
#define M_LN2       _M_LN2
#define M_LN10      2.30258509299404568402
#define M_PI        3.14159265358979323846
#define M_TWOPI     (M_PI * 2.0)
#define M_PI_2      1.57079632679489661923
#define M_PI_4      0.78539816339744830962
#define M_3PI_4     2.3561944901923448370E0
#define M_SQRTPI    1.77245385090551602792981
#define M_1_PI      0.31830988618379067154
#define M_2_PI      0.63661977236758134308
#define M_2_SQRTPI  1.12837916709551257390
#define M_SQRT2     1.41421356237309504880
#define M_SQRT1_2   0.70710678118654752440
#define M_LN2LO     1.9082149292705877000E-10
#define M_LN2HI     6.9314718036912381649E-1
#define M_SQRT3     1.73205080756887719000
#define M_IVLN10    0.43429448190325182765   /* 1 / log(10) */
#define M_LOG2_E    _M_LN2
#define M_INVLN2    1.4426950408889633870E0  /* 1 / log(2) */

// HUGE_VAL is a positive double (not necessarily representable as a float)
// representing infinity as specified in ANSI 7.5. cyg_libm_infinity is
// defined further down
#if 1
#define HUGE_VAL        (cyg_libm_infinity.value)
#define HUGE_VALF       (cyg_libm_float_infinity.value)

#else
 /* gcc >= 3.3 implicitly defines builtins for HUGE_VALx values.  */

# ifndef HUGE_VAL
#  define HUGE_VAL (__builtin_huge_val())
# endif

# ifndef HUGE_VALF
#  define HUGE_VALF (__builtin_huge_valf())
# endif

#endif /* !gcc >= 3.3  */

#ifndef CYGSYM_LIBM_NO_XOPEN_SVID_NAMESPACE_POLLUTION
// HUGE is defined in System V Interface Definition 3 (SVID3) as the largest
// finite single precision number
#define HUGE            FLT_MAX    // from float.h

// Values used in the type field of struct exception below

#define DOMAIN          1
#define SING            2
#define OVERFLOW        3
#define UNDERFLOW       4
#define TLOSS           5
#define PLOSS           6

#ifndef FP_ILOGBNAN
# define FP_ILOGBNAN INT_MAX
#endif

// TYPE DEFINITIONS

// Things required to support matherr() ( see comments in <pkgconf/libm.h>)

struct exception {
    int type;       // One of DOMAIN, SING, OVERFLOW, UNDERFLOW, TLOSS, PLOSS
    char *name;     // Name of the function generating the exception
    double arg1;    // First argument to the function
    double arg2;    // Second argument to the function
    double retval;  // Value to be returned - can be altered by matherr()
};

struct exceptionf {
    int type;       // One of DOMAIN, SING, OVERFLOW, UNDERFLOW, TLOSS, PLOSS
    char *name;     // Name of the function generating the exception
    float arg1;     // First argument to the function
    float arg2;     // Second argument to the function
    float retval;   // Value to be returned - can be altered by matherr()
};

#endif // ifndef CYGSYM_LIBM_NO_XOPEN_SVID_NAMESPACE_POLLUTION

// GLOBALS

externC const Cyg_libm_ieee_double_shape_type cyg_libm_infinity;
externC const Cyg_libm_ieee_float_shape_type cyg_libm_float_infinity;

//===========================================================================
// FUNCTION PROTOTYPES

// Functions not part of a standard

// This retrieves a pointer to the current compatibility mode of the Math
// library. See <pkgconf/libm.h> for the definition of Cyg_libm_compat_t

#ifdef CYGSEM_LIBM_THREAD_SAFE_COMPAT_MODE

externC Cyg_libm_compat_t
cyg_libm_get_compat_mode( void );

externC Cyg_libm_compat_t
cyg_libm_set_compat_mode( Cyg_libm_compat_t );

#else

externC Cyg_libm_compat_t cygvar_libm_compat_mode;

// Defined as static inline as it is unlikely that anyone wants to take the
// address of these functions.
//
// This returns the current compatibility mode

static inline Cyg_libm_compat_t
cyg_libm_get_compat_mode( void )
{
    return cygvar_libm_compat_mode;
}

// This sets the compatibility mode, and returns the previous mode
static inline Cyg_libm_compat_t
cyg_libm_set_compat_mode( Cyg_libm_compat_t math_compat_mode)
{
    Cyg_libm_compat_t oldmode;

    oldmode = cygvar_libm_compat_mode;
    cygvar_libm_compat_mode = math_compat_mode;
    return oldmode;
}

#endif // ifdef CYGSEM_LIBM_THREAD_SAFE_COMPAT_MODE

#ifdef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS

// FIXME: these need to be documented and signgam mentioned as non-ISO
// This returns the address of the signgam variable used by the gamma*() and
// lgamma*() functions
externC int *
cyg_libm_get_signgam_p( void );

#define signgam (*cyg_libm_get_signgam_p())

#else

externC int signgam;

#endif // ifdef CYGSEM_LIBM_THREAD_SAFE_GAMMA_FUNCTIONS

//===========================================================================
// Standard ANSI functions. Angles are always in radians

// Trigonometric functions - ANSI 7.5.2

// arc cosine i.e. inverse cos
CYGDCL_LIBM_IEEE_API_INLINE_F(acos)

// arc sine i.e. inverse sin
CYGDCL_LIBM_IEEE_API_INLINE_F(asin)

externC double
atan( double );            // arc tan i.e. inverse tan

externC float
atanf ( float );

// arc tan of (first arg/second arg) using signs
// of args to determine quadrant
CYGDCL_LIBM_IEEE_API_INLINE_FF(atan2)

externC double
cos( double );             // cosine

externC float
cosf( float );

externC double
sin( double );             // sine

externC float
sinf( float );

externC double
tan( double );             // tangent

externC float
tanf ( float );

// Hyperbolic functions - ANSI 7.5.3

// hyperbolic cosine
CYGDCL_LIBM_IEEE_API_INLINE_F(cosh)

// hyperbolic sine
CYGDCL_LIBM_IEEE_API_INLINE_F(sinh)

externC double
tanh( double );            // hyperbolic tangent

externC float
tanhf ( float );

// Exponential and Logarithmic Functions - ANSI 7.5.4

CYGDCL_LIBM_IEEE_API_INLINE_F(exp)

externC double
frexp( double, int * );    // break number into normalized fraction (returned)
                           // and integral power of 2 (second arg)
externC float
frexpf( float, int * );

externC double
ldexp( double, int );      // multiples number by integral power of 2

externC float
ldexpf( float, int );

// natural logarithm
CYGDCL_LIBM_IEEE_API_INLINE_F(log)

// base ten logarithm
CYGDCL_LIBM_IEEE_API_INLINE_F(log10)

externC double
modf( double, double * );  // break number into integral and fractional
                           // parts, each of which has same sign as arg.
externC float              // It returns signed fractional part, and
modff ( float, float *);   // puts integral part in second arg

// Power Functions - ANSI 7.5.5

// (1st arg) to the power of (2nd arg)
CYGDCL_LIBM_IEEE_API_INLINE_FF(pow)

// square root
CYGDCL_LIBM_IEEE_API_INLINE_F(sqrt)

// Nearest integer, absolute value and remainder functions - ANSI 7.5.6

externC double
ceil( double );            // smallest integer >= arg
externC float
ceilf ( float );

externC double
fabs( double );            // absolute value
externC float
fabsf ( float );

externC double
floor( double );           // largest integer <= arg
externC float
floorf ( float );

CYGDCL_LIBM_IEEE_API_INLINE_FF(fmod)

//===========================================================================
// Other standard functions

#ifndef CYGSYM_LIBM_NO_XOPEN_SVID_NAMESPACE_POLLUTION
externC int
matherr( struct exception * );    // User-overridable error handling - see
                                  // <pkgconf/libm.h> for a discussion
externC int
matherrf( struct exceptionf * );
#endif

// FIXME: from here needs to be documented and mentioned as non-ISO
// Arc Hyperbolic trigonometric functions

// Arc hyperbolic cos i.e. inverse cosh
CYGDCL_LIBM_IEEE_API_INLINE_F(acosh)

// Arc hyperbolic sin i.e. inverse sinh
CYGDCL_LIBM_IEEE_API_INLINE_F(asinh)

// Arc hyperbolic tan i.e. inverse tanh
CYGDCL_LIBM_IEEE_API_INLINE_F(atanh)

// Error functions

externC double                    // Error function, such that
erf( double );                    // erf(x) = 2/sqrt(pi) * integral from
                                  // 0 to x of e**(-t**2) dt
externC float
erff ( float );

externC double                    // Complementary error function - simply
erfc( double );                   // 1.0 - erf(x)

externC float
erfcf ( float );

// Gamma functions

// Logarithm of the absolute value of the
// gamma function of the argument. The
// integer signgam is used to store the
// sign of the gamma function of the arg
CYGDCL_LIBM_IEEE_API_INLINE_F(lgamma)

// Re-entrant version of the above, where
// the user passes the location of signgam
// as the second argument
CYGDCL_LIBM_IEEE_API_INLINE_FIP(lgamma_r)

// Identical to lgamma()!
// The reasons for this are historical,
// and may be changed in future standards
//
// To get the real gamma function, you should
// use: l=lgamma(x); g=signgam*exp(l);
//
// Do not just do signgam*exp(lgamma(x))
// as lgamma() modifies signgam
CYGDCL_LIBM_IEEE_API_INLINE_F(gamma)

externC double
gamma_r( double, int * );         // Identical to lgamma_r(). See above.

externC float
gammaf_r( float, int * );


// Bessel functions

// Zero-th order Bessel function of the
// first kind at the ordinate of the argument
CYGDCL_LIBM_IEEE_API_INLINE_F(j0)

// First-order Bessel function of the
// first kind at the ordinate of the argument
CYGDCL_LIBM_IEEE_API_INLINE_F(j1)

// Bessel function of the first kind of the
// order of the first argument at the
// ordinate of the second argument
CYGDCL_LIBM_IEEE_API_INLINE_IF(jn)

externC double                    // Zero-th order Bessel function of the
y0( double );                     // second kind at the ordinate of the

externC float
y0f( float );

externC double                    // First-order Bessel function of the
y1( double );                     // second kind at the ordinate of the
                                  // argument
externC float
y1f( float );

externC double                    // Bessel function of the second kind of the
yn( int, double );                // order of the first argument at the
                                  // ordinate of the second argument
externC float
ynf( int, float );

// scalb*()

externC double                    // scalbn(x,n) returns x*(2**n)
scalbn( double, int );

externC float
scalbnf ( float, int);

#ifdef CYGFUN_LIBM_SVID3_scalb

// as above except n is a floating point arg
CYGDCL_LIBM_IEEE_API_INLINE_FF(scalb)

#else
externC double
scalb( double, int );             // as scalbn()

externC float
scalbf( float, int );

#endif // ifdef CYGFUN_LIBM_SVID3_scalb

// And the rest

externC double
cbrt( double );                   // Cube Root

externC float
cbrtf( float );

// hypotenuse function, defined such that:
// hypotf(x,y)==sqrt(x**2 + y**2)

CYGDCL_LIBM_IEEE_API_INLINE_FF(hypot)

externC int                       // whether the argument is NaN
isnan( double );

externC int
isnanf ( float );

externC int                       // wheteher the argument is infinite
isinf(double);

externC int
isinff(float);

externC int
finite( double );                 // whether the argument is finite

externC int
finitef ( float );

externC double                    // logb returns the binary exponent of its
logb( double );                   // argument as an integral value
                                  // This is not recommended - use ilogb
externC float                     // instead
logbf ( float );

externC int                       // As for logb, but has the more correct
ilogb( double );                  // return value type of int

externC int
ilogbf ( float );

externC double                    // nextafter(x,y) returns the next
nextafter( double, double );      // representable floating point number
                                  // adjacent to x in the direction of y
externC float                     // i.e. the next greater FP if y>x, the next
nextafterf ( float, float );      // less FP if y<x, or just x if y==x

// when x is divided by y
CYGDCL_LIBM_IEEE_API_INLINE_FF(remainder)

externC double                    // IEEE Test Vector
significand( double );            // significand(x) computes:
                                  //   scalb(x, (double) -ilogb(x))
externC float
significandf( float );

//===========================================================================
// Non-standard functions

externC double                    // copysign(x,y) returns a number with
copysign ( double, double );      // the absolute value of x and the sign of y

externC float
copysignf ( float, float );

externC double                    // rounds to an integer according to the
rint( double );                   // current rounding mode

externC float
rintf ( float );


// BSD functions

externC double                    // expm1(x) returns the equivalent of
expm1( double );                  // (exp(x) - 1) but more accurately when
                                  // x tends to zero
externC float
expm1f ( float );

externC double                    // log1p(x) returns the equivalent of
log1p( double );                  // log(1+x) but more accurately when
                                  // x tends to zero
externC float
log1pf ( float );

#endif // ifdef CYGPKG_LIBM

#endif // CYGONCE_LIBM_MATH_H multiple inclusion protection

// EOF math.h
