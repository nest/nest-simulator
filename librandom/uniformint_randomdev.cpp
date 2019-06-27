/*
 *  uniformint_randomdev.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "uniformint_randomdev.h"

// C++ includes:
#include <cmath>
#include <limits>

// Includes from libnestutil:
#include "compose.hpp"

// Includes from sli:
#include "dictutils.h"
#include "sliexceptions.h"

// by default, init as exponential density with mean 1
librandom::UniformIntRandomDev::UniformIntRandomDev( RngPtr r_source )
  : RandomDev( r_source )
  , nmin_( 0 )
  , nmax_( 0 )
  , range_( nmax_ - nmin_ + 1 )
{
}

librandom::UniformIntRandomDev::UniformIntRandomDev()
  : RandomDev()
  , nmin_( 0 )
  , nmax_( 0 )
  , range_( nmax_ - nmin_ + 1 )
{
}

void
librandom::UniformIntRandomDev::set_status( const DictionaryDatum& d )
{
  long new_nmin = nmin_;
  long new_nmax = nmax_;

  updateValue< long >( d, names::low, new_nmin );
  updateValue< long >( d, names::high, new_nmax );

  if ( new_nmax < new_nmin )
  {
    throw BadParameterValue( "Uniformint RDV: low <= high required." );
  }

  // The following test is based on
  // https://www.securecoding.cert.org/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow

  // Rollover cases (integer bounds min, max):
  // new_nmax = n+, new_nmin = n-
  //
  //   a) n+ >= n-:
  //       1) n- >= 0: 0 <= n+ - n- <= max
  //       2) n- <  0: 0 <= n+ - n-
  //         must confirm that n+ - n- <= max
  //
  //   b) n+ < n-:
  //       1) n- <= 0: 0 > n+ - n- >= min
  //       2) n- >  0: 0 > n+ - n-
  //         must confirm that n+ - n- >= min
  //
  //  Case b) is eliminated by the first test above.
  //  Case a) is checked by confirming that
  //    n+ <= max + n-
  //    which can not rollover when n- < 0
  //
  //  Additionally, we set range = 1 + (n+ - n-)
  //  a1: n+ - n- < max
  //    n+ < max + n-
  //  a2: n+ - n- < max
  //
  //  so we need to confirm that n+ - n- != max.
  //  See pull request #61

  const long max = std::numeric_limits< long >::max();
  if ( ( new_nmin < 0 && new_nmax >= max + new_nmin ) || ( new_nmax - new_nmin == max ) )
  {
    throw BadParameterValue(
      String::compose( "Uniformint RDV: high - low < %1 required.", static_cast< double >( max ) ) );
  }

  nmin_ = new_nmin;
  nmax_ = new_nmax;
  range_ = nmax_ - nmin_ + 1;
}

void
librandom::UniformIntRandomDev::get_status( DictionaryDatum& d ) const
{
  RandomDev::get_status( d );

  def< long >( d, names::low, nmin_ );
  def< long >( d, names::high, nmax_ );
}
