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

// This file implements the Box-Muller method for generating gaussian
// random variables (GRVs).  Box-Muller has the advantage of
// deterministically requiring exactly two uniform random variables as
// input and producing exactly two GRVs as output, which makes it
// especially well-suited to the counter-based generators in
// Random123.  Other methods (e.g., Ziggurat, polar) require an
// indeterminate number of inputs for each output and so require a
// 'MicroURNG' to be used with Random123.  The down side of Box-Muller
// is that it calls sincos, log and sqrt, which may be slow.  However,
// on GPUs, these functions are remarkably fast, which makes
// Box-Muller the fastest GRV generator we know of on GPUs.
//
// This file exports two structs and one overloaded function,
// all in the r123 namespace:
//   struct r123::float2{ float x,y; }
//   struct r123::double2{ double x,y; }
//
//   r123::float2  r123::boxmuller(uint32_t u0, uint32_t u1);
//   r123::double2 r123::boxmuller(uint64_t u0, uint64_t u1);
//  
// float2 and double2 are identical to their synonymous global-
// namespace structures in CUDA.
//
// This file may not be as portable, and has not been tested as
// rigorously as other files in the library, e.g., the generators.
// Nevertheless, we hope it is useful and we encourage developers to
// copy it and modify it for their own use.  We invite comments and
// improvements.

#ifndef _r123_BOXMULLER_HPP__
#define _r123_BOXMULLER_HPP__

#include <Random123/features/compilerfeatures.h>
#include <Random123/uniform.hpp>
#include <math.h>

namespace r123{

#if !defined(__CUDACC__)
typedef struct { float x, y; } float2;
typedef struct { double x, y; } double2;
#else
typedef ::float2 float2;
typedef ::double2 double2;
#endif

#if !defined(R123_NO_SINCOS) && defined(__APPLE__)
/* MacOS X 10.10.5 (2015) doesn't have sincosf */
#define R123_NO_SINCOS 1
#endif

#if R123_NO_SINCOS /* enable this if sincos and sincosf are not in the math library */
R123_CUDA_DEVICE R123_STATIC_INLINE void sincosf(float x, float *s, float *c) {
    *s = sinf(x);
    *c = cosf(x);
}

R123_CUDA_DEVICE R123_STATIC_INLINE void sincos(double x, double *s, double *c) {
    *s = sin(x);
    *c = cos(x);
}
#endif /* sincos is not in the math library */

#if !defined(CUDART_VERSION) || CUDART_VERSION < 5000 /* enabled if sincospi and sincospif are not in math lib */

R123_CUDA_DEVICE R123_STATIC_INLINE void sincospif(float x, float *s, float *c){
    const float PIf = 3.1415926535897932f;
    sincosf(PIf*x, s, c);
}

R123_CUDA_DEVICE R123_STATIC_INLINE void sincospi(double x, double *s, double *c) {
    const double PI = 3.1415926535897932;
    sincos(PI*x, s, c);
}
#endif /* sincospi is not in math lib */

/*
 * take two 32bit unsigned random values and return a float2 with
 * two random floats in a normal distribution via a Box-Muller transform
 */
R123_CUDA_DEVICE R123_STATIC_INLINE float2 boxmuller(uint32_t u0, uint32_t u1) {
    float r;
    float2 f;
    sincospif(uneg11<float>(u0), &f.x, &f.y);
    r = sqrtf(-2.f * logf(u01<float>(u1))); // u01 is guaranteed to avoid 0.
    f.x *= r;
    f.y *= r;
    return f;
}

/*
 * take two 64bit unsigned random values and return a double2 with
 * two random doubles in a normal distribution via a Box-Muller transform
 */
R123_CUDA_DEVICE R123_STATIC_INLINE double2 boxmuller(uint64_t u0, uint64_t u1) {
    double r;
    double2 f;

    sincospi(uneg11<double>(u0), &f.x, &f.y);
    r = sqrt(-2. * log(u01<double>(u1))); // u01 is guaranteed to avoid 0.
    f.x *= r;
    f.y *= r;
    return f;
}
} // namespace r123

#endif /* BOXMULLER_H__ */
