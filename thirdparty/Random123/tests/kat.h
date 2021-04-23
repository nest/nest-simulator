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
#ifndef __katdoth__
#define __katdoth__

#include <Random123/philox.h>
#include <Random123/threefry.h>
#include <Random123/ars.h>
#include <Random123/aes.h>

enum method_e{
#define RNGNxW_TPL(base, N, W) base##N##x##W##_e,
#include "rngNxW.h"
#undef RNGNxW_TPL
    last
};

#define RNGNxW_TPL(base, N, W)                       \
    typedef struct {                                 \
        base##N##x##W##_ctr_t ctr;                   \
        base##N##x##W##_ukey_t ukey;                 \
        base##N##x##W##_ctr_t expected;              \
        base##N##x##W##_ctr_t computed;              \
    } base##N##x##W##_kat;
#include "rngNxW.h"
#undef RNGNxW_TPL

typedef struct{
    enum method_e method;
    unsigned nrounds;
    union{
#define RNGNxW_TPL(base, N, W) base##N##x##W##_kat base##N##x##W##_data;
#include "rngNxW.h"
#undef RNGNxW_TPL
	/* Sigh... For those platforms that lack uint64_t, carve
	   out 128 bytes for the counter, key, expected, and computed. */
	char justbytes[128]; 
    }u;
} kat_instance;

#endif
