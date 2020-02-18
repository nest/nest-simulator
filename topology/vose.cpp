/*
 *  vose.cpp
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

#include "vose.h"

// C++ includes:
#include <algorithm>
#include <cassert>

namespace nest
{
Vose::Vose( std::vector< double > dist )
{
  assert( not dist.empty() );

  const index n = dist.size();

  dist_.resize( n );

  // We accept distributions that do not sum to 1.
  double sum = 0.0;
  for ( std::vector< double >::iterator it = dist.begin(); it != dist.end(); ++it )
  {
    sum += *it;
  }
  // Partition distribution into small (<=1/n) and large (>1/n) probabilities
  std::vector< BiasedCoin >::iterator small = dist_.begin();
  std::vector< BiasedCoin >::iterator large = dist_.end();

  index i = 0;

  for ( std::vector< double >::iterator it = dist.begin(); it != dist.end(); ++it )
  {
    if ( *it <= sum / n )
    {
      *small++ = BiasedCoin( i++, 0, ( *it ) * n / sum );
    }
    else
    {
      *--large = BiasedCoin( i++, 0, ( *it ) * n / sum );
    }
  }

  // Generate aliases
  for ( small = dist_.begin(); ( small != large ) && ( large != dist_.end() ); ++small )
  {

    small->tails = large->heads; // 'tails' is the alias

    // The probability to select the alias is 1.0 - small->probability,
    // so we subtract this from large->probability. The following
    // equivalent calculation is more numerically stable:
    large->probability = ( large->probability + small->probability ) - 1.0;

    if ( large->probability <= 1.0 )
    {
      ++large;
    }
  }

  // Since floating point calculation is not perfect, there may be
  // probabilities left over, which should be very close to 1.0.
  while ( small != large )
  {
    ( small++ )->probability = 1.0;
  }
  while ( large != dist_.end() )
  {
    ( large++ )->probability = 1.0;
  }
}

index
Vose::get_random_id( RngPtr rng ) const
{
  // Choose random number between 0 and n
  double r = rng->drand() * dist_.size();

  // Use integer part to select bin
  index i = static_cast< index >( r );

  // Remainder determines whether to return original value or alias
  r -= i;

  if ( r < dist_[ i ].probability )
  {
    return dist_[ i ].heads;
  }
  else
  {
    return dist_[ i ].tails;
  }
}

} // namespace nest
