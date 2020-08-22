/*
   This is a modified version of (see below for details):

   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.
   Copyright (C) 2005, Mutsuo Saito
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

/*
        Modifications to the originial mt19937.{h,c} code by Nishimura,
        Matsumoto and Saito:
        - Implemented as C++ class with local variables, to allow for multiple
          generators.
        - Inclusion guard added.

        Hans Ekkehard Plesser, 2008-01-03
*/

#include "mt19937.h"

const unsigned int librandom::MT19937::N = 624;
const unsigned int librandom::MT19937::M = 397;
const unsigned long librandom::MT19937::MATRIX_A = 0x9908b0dfUL;
const unsigned long librandom::MT19937::UPPER_MASK = 0x80000000UL;
const unsigned long librandom::MT19937::LOWER_MASK = 0x7fffffffUL;
const double librandom::MT19937::I2DFactor_ = 1.0 / 4294967296.0;

librandom::MT19937::MT19937( unsigned long s )
  : mt( N )
  , mti( N + 1 )
{
  init_genrand( s );
}

void
librandom::MT19937::seed_( unsigned long s )
{
  init_genrand( s );
}

void
librandom::MT19937::init_genrand( unsigned long s )
{
  mt[ 0 ] = s & 0xffffffffUL;
  for ( mti = 1; static_cast< unsigned int >( mti ) < N; mti++ )
  {
    mt[ mti ] = ( 1812433253UL * ( mt[ mti - 1 ] ^ ( mt[ mti - 1 ] >> 30 ) ) + mti );
    /* See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier. */
    /* In the previous versions, MSBs of the seed affect   */
    /* only MSBs of the array mt[].                        */
    /* 2002/01/09 modified by Makoto Matsumoto             */
    mt[ mti ] &= 0xffffffffUL;
    /* for >32 bit machines */
  }
}

unsigned long
librandom::MT19937::genrand_int32()
{
  unsigned long y;
  static unsigned long mag01[ 2 ] = { 0x0UL, MATRIX_A };
  /* mag01[x] = x * MATRIX_A  for x=0,1 */

  if ( static_cast< unsigned int >( mti ) >= N )
  { /* generate N words at one time */
    int kk;

    if ( mti == N + 1 ) /* if init_genrand() has not been called, */
    {
      init_genrand( 5489UL ); /* a default initial seed is used */
    }

    for ( kk = 0; static_cast< unsigned int >( kk ) < N - M; kk++ )
    {
      y = ( mt[ kk ] & UPPER_MASK ) | ( mt[ kk + 1 ] & LOWER_MASK );
      mt[ kk ] = mt[ kk + M ] ^ ( y >> 1 ) ^ mag01[ y & 0x1UL ];
    }
    for ( ; static_cast< unsigned int >( kk ) < N - 1; kk++ )
    {
      y = ( mt[ kk ] & UPPER_MASK ) | ( mt[ kk + 1 ] & LOWER_MASK );
      mt[ kk ] = mt[ kk + ( M - N ) ] ^ ( y >> 1 ) ^ mag01[ y & 0x1UL ];
    }
    y = ( mt[ N - 1 ] & UPPER_MASK ) | ( mt[ 0 ] & LOWER_MASK );
    mt[ N - 1 ] = mt[ M - 1 ] ^ ( y >> 1 ) ^ mag01[ y & 0x1UL ];

    mti = 0;
  }

  y = mt[ mti++ ];

  /* Tempering */
  y ^= ( y >> 11 );
  y ^= ( y << 7 ) & 0x9d2c5680UL;
  y ^= ( y << 15 ) & 0xefc60000UL;
  y ^= ( y >> 18 );

  return y;
}
