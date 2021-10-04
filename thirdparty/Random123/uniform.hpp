/*
Copyright 2010-2011, D. E. Shaw Research.
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

#ifndef __r123_uniform_dot_hpp
#define __r123_uniform_dot_hpp

/** @defgroup uniform Uniform distribution scalar conversion functions

This file provides some simple functions that can be used to convert
integers of various widths to floats and doubles with various
characteristics.  It can be used to generate real-valued, uniformly
distributed random variables from the random integers produced by
the Random123 CBRNGs.

There are three templated functions:

 - u01:  output is as dense as possible in (0,1}, never 0.0.  May
    return 1.0 if and only if the number of output mantissa bits
    is less than the width of the input.

 - uneg11:  output is as dense as possible in {-1,1}, never 0.0.  May
    return 1.0 or -1.0 if and only if the number of output mantissa bits
    is less than the width of the input.

 - u01fixedpt:  output is "fixed point", equispaced, open at both ends, 
     and is never 0.0, 0.5 nor 1.0.

The behavior of u01 and uneg11 depend on the pre-processor symbol:
R123_UNIFORM_FLOAT_STORE.  When #defined to a non-zero value, u01
and uneg11 declare a volatile intermediate result, with the
intention of forcing architectures that have "extra bits" in their
floating point registers to more closely conform to IEEE
arithmetic.  When compiled this way, u01 and uneg11 will be
significantly slower, as they will incur a memory write and read on
every call.  Without it, they may fail the "known answer test"
implemented in ut_uniform_IEEEkat.cpp even though they perform
perfectly reasonable int to float conversions.  We have used
this option to get 32-bit x86 to produce the same results as
64-bit x86-64 code, but we do not recommend it for normal
use.

Three additional functions are defined when C++11 or newer is in use:

 - u01all
 - uneg11all
 - u01fixedptall

These functions apply the corresponding conversion to every
element of their argument, which must be a staticly sized
array, e.g., an r123array or a std::array of an integer type.

This file may not be as portable, and has not been tested as
rigorously as other files in the library, e.g., the generators.
Nevertheless, we hope it is useful and we encourage developers to
copy it and modify it for their own use.  We invite comments and
improvements.
*/

#include <Random123/features/compilerfeatures.h>
#include <limits>
#if R123_USE_CXX11_TYPE_TRAITS
#include <type_traits>
#endif
#if __cplusplus >= 201103L
#include <array>
#endif

namespace r123{
/**
@{
@cond HIDDEN_FROM_DOXYGEN
*/

#if R123_USE_CXX11_TYPE_TRAITS
using std::make_signed;
using std::make_unsigned;
#else
// Sigh... We could try to find another <type_traits>, e.g., from
// boost or TR1.  Or we can do it ourselves in the r123 namespace.
// It's not clear which will cause less headache...
template <typename T> struct make_signed{};
template <typename T> struct make_unsigned{};
#define R123_MK_SIGNED_UNSIGNED(ST, UT)                 \
template<> struct make_signed<ST>{ typedef ST type; }; \
template<> struct make_signed<UT>{ typedef ST type; }; \
template<> struct make_unsigned<ST>{ typedef UT type; }; \
template<> struct make_unsigned<UT>{ typedef UT type; }

R123_MK_SIGNED_UNSIGNED(int8_t, uint8_t);
R123_MK_SIGNED_UNSIGNED(int16_t, uint16_t);
R123_MK_SIGNED_UNSIGNED(int32_t, uint32_t);
R123_MK_SIGNED_UNSIGNED(int64_t, uint64_t);
#if R123_USE_GNU_UINT128
R123_MK_SIGNED_UNSIGNED(__int128_t, __uint128_t);
#endif
#undef R123_MK_SIGNED_UNSIGNED
#endif

#if defined(__CUDACC__) || defined(_LIBCPP_HAS_NO_CONSTEXPR)
// Amazing! cuda thinks numeric_limits::max() is a __host__ function, so
// we can't use it in a device function.  
//
// The LIBCPP_HAS_NO_CONSTEXP test catches situations where the libc++
// library thinks that the compiler doesn't support constexpr, but we
// think it does.  As a consequence, the library declares
// numeric_limits::max without constexpr.  This workaround should only
// affect a narrow range of compiler/library pairings.
// 
// In both cases, we find max() by computing ~(unsigned)0 right-shifted
// by is_signed.
template <typename T>
R123_CONSTEXPR R123_STATIC_INLINE R123_CUDA_DEVICE T maxTvalue(){
    typedef typename make_unsigned<T>::type uT;
    return (~uT(0)) >> std::numeric_limits<T>::is_signed;
 }
#else
template <typename T>
R123_CONSTEXPR R123_STATIC_INLINE T maxTvalue(){
    return std::numeric_limits<T>::max();
}
#endif
/** @endcond
    @}
 */

//! Return a uniform real value in (0, 1]
/**
    @ingroup uniform
     Input is a W-bit integer (signed or unsigned).  It is cast to
     a W-bit unsigned integer, multiplied by Ftype(2^-W) and added to
     Ftype(2^(-W-1)).  A good compiler should optimize it down to an
     int-to-float conversion followed by a multiply and an add, which
     might be fused, depending on the architecture.
   
    If the input is a uniformly distributed integer, and if Ftype
    arithmetic follows IEEE754 round-to-nearest rules, then the
    result is a uniformly distributed floating point number in (0, 1].

-    The result is never exactly 0.0.  
-    The smallest value returned is 2^-(W-1).
-    Let M be the number of mantissa bits in Ftype (typically 24 or 53).
  -    If W>M  then the largest value retured is 1.0.
  -    If W<=M then the largest value returned is Ftype(1.0 - 2^(-W-1)).
*/
template <typename Ftype, typename Itype>
R123_CUDA_DEVICE R123_STATIC_INLINE Ftype u01(Itype in){
    typedef typename make_unsigned<Itype>::type Utype;
    R123_CONSTEXPR Ftype factor = Ftype(1.)/(Ftype(maxTvalue<Utype>()) + Ftype(1.));
    R123_CONSTEXPR Ftype halffactor = Ftype(0.5)*factor;
#if R123_UNIFORM_FLOAT_STORE
    volatile Ftype x = Utype(in)*factor; return x+halffactor;
#else
    return Utype(in)*factor + halffactor;
#endif
}

//! Return a signed value in [-1,1]
/**
    @ingroup uniform
   The argument is converted to a W-bit signed integer, multiplied by Ftype(2^-(W-1)) and
   then added to Ftype(2^-W).  A good compiler should optimize
   it down to an int-to-float conversion followed by a multiply and
   an add, which might be fused, depending on the architecture.

 If the input is a uniformly distributed integer, and if Ftype
 arithmetic follows IEEE754 round-to-nearest rules, then the
 output is a uniformly distributed floating point number in [-1, 1].

- The result is never exactly 0.0.
- The smallest absolute value returned is 2^-W
- Let M be the number of mantissa bits in Ftype.
  - If W>M  then the largest value retured is 1.0 and the smallest is -1.0.
  - If W<=M then the largest value returned is the Ftype(1.0 - 2^-W)
    and the smallest value returned is -Ftype(1.0 - 2^-W).
*/
template <typename Ftype, typename Itype>
R123_CUDA_DEVICE R123_STATIC_INLINE Ftype uneg11(Itype in){
    typedef typename make_signed<Itype>::type Stype;
    R123_CONSTEXPR Ftype factor = Ftype(1.)/(Ftype(maxTvalue<Stype>()) + Ftype(1.));
    R123_CONSTEXPR Ftype halffactor = Ftype(0.5)*factor;
#if R123_UNIFORM_FLOAT_STORE
    volatile Ftype x = Stype(in)*factor; return x+halffactor;
#else
    return Stype(in)*factor + halffactor;
#endif
}

//! Return a value in (0,1) chosen from a set of equally spaced fixed-point values
/**
    @ingroup uniform
   Let:
     - W = width of Itype, e.g., 32 or 64, regardless of signedness.
     - M = mantissa bits of Ftype, e.g., 24, 53 or 64
     - B = min(M, W)

   Then the 2^(B-1) possible output values are: 2^-B*{1, 3, 5, ..., 2^B - 1}

   The smallest output is: 2^-B

   The largest output is:  1 - 2^-B

   The output is never exactly 0.0, nor 0.5, nor 1.0.

   The 2^(B-1) possible outputs:
     - are equally likely,
     - are uniformly spaced by 2^-(B-1),
     - are balanced around 0.5
*/
template <typename Ftype, typename Itype>
R123_CUDA_DEVICE R123_STATIC_INLINE Ftype u01fixedpt(Itype in){
    typedef typename make_unsigned<Itype>::type Utype;
    R123_CONSTEXPR int excess = std::numeric_limits<Utype>::digits - std::numeric_limits<Ftype>::digits;
    if(excess>=0){
        R123_CONSTEXPR int ex_nowarn = (excess>=0) ? excess : 0;
        R123_CONSTEXPR Ftype factor = Ftype(1.)/(Ftype(1.) + Ftype((maxTvalue<Utype>()>>ex_nowarn)));
        return (1 | (Utype(in)>>ex_nowarn)) * factor;
    }else
        return u01<Ftype>(in);
}

#if R123_USE_CXX11_STD_ARRAY

//! Apply u01 to every item in an r123array, returning a std::array
/** @ingroup uniform
 * Only in C++11 and newer.
 * The argument type may be any integer collection with a constexpr static_size member,
 * e.g., an r123array or a std::array of an integer type.
 */
template <typename Ftype, typename CollType>
static inline
std::array<Ftype, CollType::static_size> u01all(CollType in)
{
    std::array<Ftype, CollType::static_size> ret;
    auto p = ret.begin();
    for(auto e : in){
        *p++ = u01<Ftype>(e);
    }
    return ret;
}

//! Apply uneg11 to every item in an r123array, returning a std::array
/** @ingroup uniform
 * Only in C++11 and newer.
 * The argument type may be any integer collection with a constexpr static_size member,
 * e.g., an r123array or a std::array of an integer type.
 */
template <typename Ftype, typename CollType>
static inline
std::array<Ftype, CollType::static_size> uneg11all(CollType in)
{
    std::array<Ftype, CollType::static_size> ret;
    auto p = ret.begin();
    for(auto e : in){
        *p++ = uneg11<Ftype>(e);
    }
    return ret;
}

//! Apply u01fixedpt to every item in an r123array, returning a std::array
/** @ingroup uniform 
 * Only in C++11 and newer.
 * The argument type may be any integer collection with a constexpr static_size member,
 * e.g., an r123array or a std::array of an integer type.
*/
template <typename Ftype, typename CollType>
static inline
std::array<Ftype, CollType::static_size> u01fixedptall(CollType in)
{
    std::array<Ftype, CollType::static_size> ret;
    auto p = ret.begin();
    for(auto e : in){
        *p++ = u01fixedpt<Ftype>(e);
    }
    return ret;
}
#endif // __cplusplus >= 201103L

} // namespace r123

#endif

