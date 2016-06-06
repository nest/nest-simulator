#ifndef MT19937_H
#define MT19937_H

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

// C++ includes:
#include <vector>

// Includes from librandom:
#include "randomgen.h"

namespace librandom
{
/**
 * Mersenne Twister MT19937.
 * This class implements the 32-bit MT19937 Mersenne Twister
 * RNG by Matsumoto and Nishimura. The implementation wraps a C++ class
 * around the originial code.
 */
class MT19937 : public RandomGen
{
public:
  //! Create generator with given seed
  explicit MT19937( unsigned long );

  ~MT19937(){};

  RngPtr
  clone( unsigned long s )
  {
    return RngPtr( new MT19937( s ) );
  }

private:
  //! implements seeding for RandomGen
  void seed_( unsigned long );

  //! implements drawing a single [0,1) number for RandomGen
  double drand_();

private:
  // functions inherited from C-version of mt19937

  /* initializes mt[N] with a seed */
  void init_genrand( unsigned long );

  /* generates a random number on [0,0xffffffff]-interval */
  unsigned long genrand_int32();

  /* generates a random number on [0,1)-real-interval */
  double genrand_real2();

  /* Period parameters */
  static const unsigned int N;
  static const unsigned int M;
  static const unsigned long MATRIX_A;   /* constant vector a */
  static const unsigned long UPPER_MASK; /* most significant w-r bits */
  static const unsigned long LOWER_MASK; /* least significant r bits */
  static const double I2DFactor_;        //!< int to double factor

  std::vector< unsigned long > mt; /* the array for the state vector  */
  int mti;                         /* mti==N+1 means mt[N] is not initialized */
};
}

inline double
librandom::MT19937::drand_()
{
  return genrand_real2();
}

inline double
librandom::MT19937::genrand_real2()
{
  return I2DFactor_ * genrand_int32();
  /* divided by 2^32 */
}


#endif
