/*************************************************************************
 *                                                                       *
 * Open Dynamics Engine, Copyright (C) 2001,2002 Russell L. Smith.       *
 * All rights reserved.  Email: russ@q12.org   Web: www.q12.org          *
 *                                                                       *
 * This library is free software; you can redistribute it and/or         *
 * modify it under the terms of EITHER:                                  *
 *   (1) The GNU Lesser General Public License as published by the Free  *
 *       Software Foundation; either version 2.1 of the License, or (at  *
 *       your option) any later version. The text of the GNU Lesser      *
 *       General Public License is included with this library in the     *
 *       file LICENSE.TXT.                                               *
 *   (2) The BSD-style license that is included with this library in     *
 *       the file LICENSE-BSD.TXT.                                       *
 *                                                                       *
 * This library is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files    *
 * LICENSE.TXT and LICENSE-BSD.TXT for more details.                     *
 *                                                                       *
 *************************************************************************/


/* per-machine configuration */



#ifndef _ODE_CONFIG_H_

#define _ODE_CONFIG_H_

#ifdef __cplusplus

extern "C" {

#endif



#include <stdio.h>

#include <stdarg.h>

#include <malloc.h>		// for alloca under windows

#include <string.h>

#include <math.h>



#if defined(WIN32) && (defined(MSVC) || defined(MINGW))

static union { unsigned char __c[4]; float __f; } __ode_huge_valf =

  {{0,0,0x80,0x7f}};

#define _INFINITY4 (__ode_huge_valf.__f)

static union { unsigned char __c[8]; double __d; } __ode_huge_val =

  {{0,0,0,0,0,0,0xf0,0x7f }};

#define _INFINITY8 (__ode_huge_val.__d)

#else

#define _INFINITY8 HUGE_VAL

#define _INFINITY4 HUGE_VALF

#endif



#if defined(dSINGLE)

#define dInfinity _INFINITY4

#else

#define dInfinity _INFINITY8

#endif



#define SHAREDLIBIMPORT //__declspec (dllimport)

#define SHAREDLIBEXPORT //__declspec (dllexport)



/* some types. assume `int' >= 32 bits */

typedef unsigned int    uint;

typedef int             int32;

typedef unsigned int    uint32;

typedef short           int16;

typedef unsigned short  uint16;

// typedef char            int8;

typedef unsigned char   uint8;





/* an integer type that we can safely cast a pointer to and from without

 * loss of bits.

 */

typedef uintptr_t intP;





/* if we're compiling on a pentium, we may need to know the clock rate so

 * that the timing function can report accurate times. this number only needs

 * to be set accurately if we're doing performance tests - otherwise just

 * ignore this. i have not worked out how to determine this number

 * automatically yet.

 */



#ifdef PENTIUM

#ifndef PENTIUM_HZ

#define PENTIUM_HZ (496.318983e6)

#endif

#endif





/* the efficient alignment. most platforms align data structures to some

 * number of bytes, but this is not always the most efficient alignment.

 * for example, many x86 compilers align to 4 bytes, but on a pentium it is

 * important to align doubles to 8 byte boundaries (for speed), and the 4

 * floats in a SIMD register to 16 byte boundaries. many other platforms have

 * similar behavior. setting a larger alignment can waste a (very) small

 * amount of memory. NOTE: this number must be a power of two.

 */



#define EFFICIENT_ALIGNMENT 16





/* for unix, define this if your system supports anonymous memory maps

 * (linux does).

 */



#define MMAP_ANONYMOUS





#ifdef __cplusplus

}

#endif



#endif

