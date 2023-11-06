//
//
/*
    Copyright (C) 2000 PARAPET partners
    Copyright (C) 2000- 2010, Hammersmith Imanet Ltd
    This file is part of STIR.

    SPDX-License-Identifier: Apache-2.0 AND License-ref-PARAPET-license

	See STIR/LICENSE.txt for details
*/
#ifndef __stir_common_H__
#define __stir_common_H__

/*!
  \file 
  \ingroup buildblock 
  \brief basic configuration include file 

  \author Kris Thielemans
  \author Alexey Zverovich
  \author Darren Hague
  \author PARAPET project





 This include file defines some commonly used macros, templates 
 and functions in an attempt to smooth out some system dependencies.
 It also defines some functions which are used very often.

 <H3> Macros and system dependencies:</H3>

<UL>
 <LI> macros for namespace support: 
   \c \#defines \c START_NAMESPACE_STIR etc.
 </LI>

 <LI> preprocessor definitions which attempt to determine the 
   operating system this is going to run on.
   use as \c "#ifdef  __OS_WIN__ ... #elif ... #endif"
   Possible values are __OS_WIN__, __OS_MAC__, __OS_VAX__, __OS_UNIX__
   The __OS_UNIX__ case has 'subbranches'. At the moment we attempt to find
   out on __OS_AIX__, __OS_SUN__, __OS_OSF__, __OS_LINUX__.
   (If the attempts fail to determine the correct OS, you can pass
    the correct value as a preprocessor definition to the compiler)
 </LI>
 <LI> \c \#includes cstdio, cstdlib, cstring, cmath
 </LI>

 <LI> \c \#ifdef \c STIR_ASSERT, then define our own assert, else include &lt;cassert&gt;
 </LI>
</UL>

<H3> Speeding up std::copy</H3>

 <UL>
 <LI> For old compilers (check the source!), overloads of std::copy for built-in 
      types to use memmove (so it's faster)
 </LI>
 </UL>

<H3> stir namespace members declared here</H3>
  
 <UL>
 <LI> <tt>const double _PI</tt></li>

 <LI> <tt>inline template  &lt;class NUMBER&gt; NUMBER square(const NUMBER &x) </tt></li>

 </UL>

<h3> stir include files included here</H3>
  <UL>
  <li> <tt>stir/config.h</tt> sets various preprocessor defines (generated from STIRConfig.in)</li>
  </UL>
*/
#include "stir/config.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <boost/math/constants/constants.hpp>

//*************** namespace macros
# define START_NAMESPACE_STIR namespace stir {
# define END_NAMESPACE_STIR }
# define USING_NAMESPACE_STIR using namespace stir;
# define START_NAMESPACE_STD namespace std {
# define END_NAMESPACE_STD }
# define USING_NAMESPACE_STD using namespace std;


//*************** define __OS_xxx__

#if !defined(__OS_WIN__) && !defined(__OS_MAC__) && !defined(__OS_VAX__) && !defined(__OS_UNIX__) 
// if none of these macros is defined externally, we attempt to guess, defaulting to UNIX

#ifdef __MSL__
   // Metrowerks CodeWarrior
   // first set its own macro
#  if macintosh && !defined(__dest_os)
#    define __dest_os __mac_os
#  endif
#  if __dest_os == __mac_os
#    define __OS_MAC__
#  else
#    define __OS_WIN__
#  endif

#elif defined(_WIN32) || defined(WIN32) || defined(_WINDOWS) || defined(_DOS)
  // Visual C++, MSC, cygwin gcc and hopefully some others
# define __OS_WIN__

#elif defined(VAX)
   // Just in case anyone is still using VAXes...
#  define __OS_VAX__

#else // default

#  define __OS_UNIX__
   // subcases
#  if defined(_AIX)
#    define __OS_AIX__
#  elif defined(__sun)
     // should really branch on SunOS and Solaris...
#    define __OS_SUN__
#  elif defined(__linux__)
#    define __OS_LINUX__
#  elif defined(__osf__)
#    defined __OS_OSF__
#  endif

#endif  // __OS_UNIX_ case

#endif // !defined(__OS_xxx_)

//*************** overload std::copy for built-in types
/* If you have an older compiler, chages are that std::copy is 
   implemented in the obvious way of iterating and copying along 
   the way. However, for simple types (such as floats), calling 
   memmove (not memcpy as the ranges could overlap) is faster.
   So, we overload std::copy for some built-in types.
  
   However, newer compilers (in particular gcc from version 2.8)
   take care of this themselves. So, we only do this 
   conditionally.
*/
#ifdef STIR_SPEED_UP_STD_COPY
#include <algorithm>

START_NAMESPACE_STD
//! overloads std::copy for faster performance
template <>
inline double * 
copy(const double * first, const double * last, double * to)
{  memmove(to, first, (last-first)*sizeof(double)); return to+(last-first); }

template <>
inline float * 
copy(const float * first, const float * last, float * to)
{  memmove(to, first, (last-first)*sizeof(float)); return to+(last-first); }

template <>
inline unsigned long int * 
copy(const unsigned long int * first, const unsigned long int * last, unsigned long int * to)
{  memmove(to, first, (last-first)*sizeof(unsigned long int)); return to+(last-first); }

template <>
inline signed long int * 
copy(const signed long int * first, const signed long int * last, signed long int * to)
{  memmove(to, first, (last-first)*sizeof(signed long int)); return to+(last-first); }

template <>
inline unsigned int * 
copy(const unsigned int * first, const unsigned int * last, unsigned int * to)
{  memmove(to, first, (last-first)*sizeof(unsigned int)); return to+(last-first); }

template <>
inline signed int * 
copy(const signed int * first, const signed int * last, signed int * to)
{  memmove(to, first, (last-first)*sizeof(signed int)); return to+(last-first); }

template <>
inline unsigned short int * 
copy(const unsigned short int * first, const unsigned short int * last, unsigned short int * to)
{  memmove(to, first, (last-first)*sizeof(unsigned short int)); return to+(last-first); }

template <>
inline signed short int * 
copy(const signed short int * first, const signed short int * last, signed short int * to)
{  memmove(to, first, (last-first)*sizeof(signed short int)); return to+(last-first); }

template <>
inline unsigned char * 
copy(const unsigned char * first, const unsigned char * last, unsigned char * to)
{  memmove(to, first, (last-first)*sizeof(unsigned char)); return to+(last-first); }

template <>
inline signed char * 
copy(const signed char * first, const signed char * last, signed char * to)
{  memmove(to, first, (last-first)*sizeof(signed char)); return to+(last-first); }

template <>
inline char * 
copy(const char * first, const char * last, char * to)
{  memmove(to, first, (last-first)*sizeof(char)); return to+(last-first); }


template <>
inline bool * 
copy(const bool * first, const bool * last, bool * to)
{  memmove(to, first, (last-first)*sizeof(bool)); return to+(last-first); }

END_NAMESPACE_STD

#endif // #ifdef STIR_SPEED_UP_STD_COPY


//*************** assert

#ifndef STIR_ASSERT
#  include <cassert>
#else
  // use our own assert
#  ifdef assert
#    undef assert
#  endif
#  if !defined(NDEBUG)
#    define assert(x) {if (!(x)) { \
      fprintf(stderr,"Assertion \"%s\" failed in file %s:%d\n", # x,__FILE__, __LINE__); \
      abort();} }
#  else 
#     define assert(x)
#  endif
#endif // STIR_ASSERT

//*************** 
START_NAMESPACE_STIR

//! The constant pi to high precision.
/*! \ingroup buildblock */
#ifndef _PI
#define _PI  boost::math::constants::pi<double>()
#endif

//! Define the speed of light in mm / ps
const double _c_light = 0.299792458;
//! This ratio is used often.
const double _c_light_div2 = _c_light * 0.5;

//! returns the square of a number, templated.
/*! \ingroup buildblock */
template <class NUMBER> 
inline NUMBER square(const NUMBER &x) { return x*x; }

END_NAMESPACE_STIR

#endif 
