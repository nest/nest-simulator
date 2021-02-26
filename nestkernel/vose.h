/*
 *  vose.h
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

#ifndef VOSE_H
#define VOSE_H

// C++ includes:
#include <vector>

// Includes from librandom:
#include "randomgen.h"

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

/**
 * Vose's alias method for selecting a random number using a discrete
 * probability distribution. See Michael D. Vose (1991), A linear
 * algorithm for generating random numbers with a given distribution,
 * IEEE trans. softw. eng. 17(9):972.
 * See also http://www.keithschwarz.com/darts-dice-coins/
 */
class Vose
{
  /**
   * An object containing two possible outcomes and a probability to
   * choose between the two.
   */
  struct BiasedCoin
  {
    index heads, tails;
    double probability; ///< Probability for heads
    BiasedCoin()
      : heads( 0 )
      , tails( 0 )
      , probability( 0 ){};
    BiasedCoin( index h, index t, double p )
      : heads( h )
      , tails( t )
      , probability( p ){};
  };

public:
  /**
   * Constructor taking a probability distribution.
   * @param dist - probability distribution.
   */
  Vose( std::vector< double > dist );

  /**
   * @returns a randomly selected index with the given distribution
   */
  index get_random_id( librandom::RngPtr rng ) const;

private:
  std::vector< BiasedCoin > dist_;
};


} // namespace nest

#endif
