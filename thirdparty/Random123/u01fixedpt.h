/*
Copyright 2011, D. E. Shaw Research.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions, and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions, and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

* Neither the name of D. E. Shaw Research nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _random123_ufixed01_dot_h_
#define _random123_ufixed01_dot_h_

#include "features/compilerfeatures.h"

/** @defgroup u01fixedpt The u01fixedpt conversion functions

    These functions convert unsigned W-bit integers to uniformly
    spaced real values (float or double) between 0.0 and 1.0 with
    mantissas of M bits.

    PLEASE THINK CAREFULLY BEFORE USING THESE FUNCTIONS.  THEY MAY
    NOT BE WHAT YOU WANT.  YOU MAY BE MUCH BETTER SERVED BY THE
    FUNCTIONS IN ./uniform.hpp.

    These functions produce a finite number *uniformly spaced* values
    in the range from 0.0 to 1.0 with uniform probability.  The price
    of uniform spacing is that they may not utilize the entire space
    of possible outputs.  E.g., u01fixedpt_closed_open_32_24 will never
    produce a non-zero value less than 2^-24, even though such values
    are representable in single-precision floating point.

    There are 12 functions, corresponding to the following choices:

     -  W = 32 or 64
     -  M = 24 (float) or 53 (double)
     -  open0 or closed0 : whether the output is open or closed at 0.0
     -  open1 or closed1 : whether the output is open or closed at 1.0 

    The W=64 M=24 cases are not implemented.  To obtain an M=24 float
    from a uint64_t, use a cast (possibly with right-shift and bitwise
    and) to convert some of the bits of the uint64_t to a uint32_t and
    then use u01fixedpt_x_y_32_float.  Note that the 64-bit random integers
    produced by the Random123 library are random in "all the bits", so
    with a little extra effort you can obtain two floats this way --
    one from the high bits and one from the low bits of the 64-bit
    value.

    If the output is open at one end, then the extreme
    value (0.0 or 1.0) will never be returned.  Conversely, if the output
    is closed at one end, then the extreme value is a possible
    return value.

    The values returned are as follows.  All values are returned
    with equal frequency, except as noted in the closed_closed case:

     closed_open:  Let P=min(M,W)
        there are 2^P possible output values:
        {0, 1, 2, ..., 2^P-1}/2^P

     open_closed:  Let P=min(M,W)
        there are 2^P possible values:
        {1, 2, ..., 2^P}/2^P

     open_open:   Let P=min(M, W+1) 
        there are 2^(P-1) possible values:
        {1, 3, 5, ..., 2^P-1}/2^P

     closed_closed:  Let P=min(M, W-1)
        there are 1+2^P possible values:
        {0, 1, 2, ... 2^P}/2^P
        The extreme values (0.0 and 1.0) are
        returned with half the frequency of
        all others.
    
    On x86 hardware, especially on 32bit machines, the use of
    internal 80bit x87-style floating point may result in
    'bonus' precision, which may cause closed intervals to not
    be really closed, i.e. the conversions below might not
    convert UINT{32,64}_MAX to 1.0.  This sort of issue is
    likely to occur when storing the output of a u01fixedpt_*_32_float
    function in a double, though one can imagine getting extra
    precision artifacts when going from 64_53 as well.  Other
    artifacts may exist on some GPU hardware.  The tests in
    kat_u01_main.h try to expose such issues, but caveat emptor.

    @cond HIDDEN_FROM_DOXYGEN
 */

/* Hex floats were standardized by C in 1999, but weren't standardized
   by C++ until 2011.  So, we're obliged to write out our constants in
   decimal, even though they're most naturally expressed in binary.
   We cross our fingers and hope that the compiler does the compile-time
   constant arithmetic properly.
*/
#define R123_0x1p_31f (1.f/(1024.f*1024.f*1024.f*2.f))
#define R123_0x1p_24f (128.f*R123_0x1p_31f)
#define R123_0x1p_23f (256.f*R123_0x1p_31f)
#define R123_0x1p_32  (1./(1024.*1024.*1024.*4.))
#define R123_0x1p_63 (2.*R123_0x1p_32*R123_0x1p_32)
#define R123_0x1p_53 (1024.*R123_0x1p_63)
#define R123_0x1p_52 (2048.*R123_0x1p_63)

/** @endcond */

#ifndef R123_USE_U01_DOUBLE
#define R123_USE_U01_DOUBLE 1
#endif

#ifdef __cplusplus
extern "C"{
#endif

/* narrowing conversions:  uint32_t to float */
R123_CUDA_DEVICE R123_STATIC_INLINE float u01fixedpt_closed_closed_32_float(uint32_t i){
    /* N.B.  we ignore the high bit, so output is not monotonic */
    return ((i&0x7fffffc0) + (i&0x40))*R123_0x1p_31f; /* 0x1.p-31f */
}

R123_CUDA_DEVICE R123_STATIC_INLINE float u01fixedpt_closed_open_32_float(uint32_t i){
    return (i>>8)*R123_0x1p_24f; /* 0x1.0p-24f; */
}

R123_CUDA_DEVICE R123_STATIC_INLINE float u01fixedpt_open_closed_32_float(uint32_t i){
    return (1+(i>>8))*R123_0x1p_24f; /* *0x1.0p-24f; */
}

R123_CUDA_DEVICE R123_STATIC_INLINE float u01fixedpt_open_open_32_float(uint32_t i){
    return (0.5f+(i>>9))*R123_0x1p_23f; /* 0x1.p-23f; */
}

#if R123_USE_U01_DOUBLE
/* narrowing conversions:  uint64_t to double */
R123_CUDA_DEVICE R123_STATIC_INLINE double u01fixedpt_closed_closed_64_double(uint64_t i){
    /* N.B.  we ignore the high bit, so output is not monotonic */
    return ((i&R123_64BIT(0x7ffffffffffffe00)) + (i&0x200))*R123_0x1p_63; /* 0x1.p-63; */
}

R123_CUDA_DEVICE R123_STATIC_INLINE double u01fixedpt_closed_open_64_double(uint64_t i){
    return (i>>11)*R123_0x1p_53; /* 0x1.0p-53; */
}

R123_CUDA_DEVICE R123_STATIC_INLINE double u01fixedpt_open_closed_64_double(uint64_t i){
    return (1+(i>>11))*R123_0x1p_53; /* 0x1.0p-53; */
}

R123_CUDA_DEVICE R123_STATIC_INLINE double u01fixedpt_open_open_64_double(uint64_t i){
    return (0.5+(i>>12))*R123_0x1p_52; /* 0x1.0p-52; */
}

/* widening conversions:  u32 to double */
R123_CUDA_DEVICE R123_STATIC_INLINE double u01fixedpt_closed_closed_32_double(uint32_t i){
    /* j = i+(i&1) takes on 2^31+1 possible values with a 'trapezoid' distribution:
      p_j =  1 0 2 0 2 .... 2 0 2 0 1
      j   =  0 1 2 3 4 ....        2^32
      by converting to double *before* doing the add, we don't wrap the high bit.
    */
    return (((double)(i&1)) + i)*R123_0x1p_32; /* 0x1.p-32; */
}

R123_CUDA_DEVICE R123_STATIC_INLINE double u01fixedpt_closed_open_32_double(uint32_t i){
    return i*R123_0x1p_32; /* 0x1.p-32; */
}

R123_CUDA_DEVICE R123_STATIC_INLINE double u01fixedpt_open_closed_32_double(uint32_t i){
    return (1.+i)*R123_0x1p_32; /* 0x1.p-32; */
}

R123_CUDA_DEVICE R123_STATIC_INLINE double u01fixedpt_open_open_32_double(uint32_t i){
    return (0.5+i)*R123_0x1p_32; /* 0x1.p-32; */
}
#endif /* R123_USE_U01_DOUBLE */

#ifdef __cplusplus
}
#endif

/** @} */
#endif
