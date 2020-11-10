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

/*
 * Written by Tom Schoonjans <Tom.Schoonjans@me.com>
 */

#ifndef __metalfeatures_dot_hpp
#define __metalfeatures_dot_hpp

#ifndef R123_STATIC_INLINE
#define R123_STATIC_INLINE inline
#endif

#ifndef R123_FORCE_INLINE
#define R123_FORCE_INLINE(decl) decl __attribute__((always_inline))
#endif

#ifndef R123_CUDA_DEVICE
#define R123_CUDA_DEVICE
#endif

#ifndef R123_METAL_THREAD_ADDRESS_SPACE
#define R123_METAL_THREAD_ADDRESS_SPACE thread
#endif

#ifndef R123_METAL_CONSTANT_ADDRESS_SPACE
#define R123_METAL_CONSTANT_ADDRESS_SPACE constant
#endif

#ifndef R123_ASSERT
#define R123_ASSERT(x)
#endif

#ifndef R123_BUILTIN_EXPECT
#define R123_BUILTIN_EXPECT(expr,likely) expr
#endif

#ifndef R123_USE_GNU_UINT128
#define R123_USE_GNU_UINT128 0
#endif

#ifndef R123_USE_MULHILO64_ASM
#define R123_USE_MULHILO64_ASM 0
#endif

#ifndef R123_USE_MULHILO64_MSVC_INTRIN
#define R123_USE_MULHILO64_MSVC_INTRIN 0
#endif

#ifndef R123_USE_MULHILO64_CUDA_INTRIN
#define R123_USE_MULHILO64_CUDA_INTRIN 0
#endif

#ifndef R123_USE_MULHILO64_OPENCL_INTRIN
#define R123_USE_MULHILO64_OPENCL_INTRIN 0
#endif

#ifndef R123_USE_MULHILO32_MULHI_INTRIN
#define R123_USE_MULHILO32_MULHI_INTRIN 1
#endif

#if R123_USE_MULHILO32_MULHI_INTRIN
#include <metal_integer>
#define R123_MULHILO32_MULHI_INTRIN metal::mulhi
#endif

#ifndef R123_USE_AES_NI
#define R123_USE_AES_NI 0
#endif

#ifndef R123_USE_64BIT
#define R123_USE_64BIT 0 /* Metal currently (Feb 2019, Specification-2) does not support 64-bit variable types */
#endif

#ifndef R123_ULONG_LONG
/* the longest integer type in Metal (Feb 2019, Specification-2) is a
 * 32-bit unsigned int.  Let's hope for the best... */
#define R123_ULONG_LONG unsigned int 
#endif

#endif
