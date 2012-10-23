#ifndef BINOMIAL_H
#define BINOMIAL_H

/*
 *  binomial.h
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

/*
  This file is part of the NEST topology module.
  Author: Kittel Austvoll
*/

#include <vector>
#include <cmath>

#include "nest.h"
#include "randomgen.h"

#include "nodewrapper.h"
#include "parameters.h"


namespace nest
{
  class Binomial//: public NodeSelector
  {
    /**
     * Structure that stores information about the derived
     * exponent, mantissa and LID of a probability vector.
     */
    struct Exponentiation
    {
      double_t probability;
      int_t exponent;
      double_t mantissa;
      int_t lid;
      
      Exponentiation():
	probability(0.0),
	   exponent(0),
	   mantissa(0.0),
	   lid(0)
      {}

    Exponentiation(double_t p, int_t i):
      probability(p),
	exponent(),
	mantissa(),
	lid(i)
      {
	mantissa = std::frexp(p, &exponent);
      }

      bool operator<(const Exponentiation& a ) const
      {
	return exponent < a.exponent;
      }

    };

    struct GroupedExponentiation
    {
      std::vector<Exponentiation> exponentiations;
      double_t total_probability;
      int_t exponent;

    GroupedExponentiation():
      exponentiations(),
	total_probability(0.0),
	exponent(std::numeric_limits<int_t>::max())
      {}

      void push_back(const Exponentiation& e)
      {
	if(exponentiations.size() == 0)
	  {
	    // First element in new group.
	    exponent = e.exponent;
	  }

	// All elements in group must have the
	// same exponent.
	assert(e.exponent == exponent);
	exponentiations.push_back(e);

	// Update probability sum.
	total_probability += e.probability;
      }
      
      Exponentiation at(int_t i)
      {
	return exponentiations.at(i);
      }
      
      int_t size() const
      {
	return exponentiations.size();
      }

    };

  public:
    Binomial();

    void set_fixed(index n);

    void initialise(NodeWrapper& driver,
		    lockPTR<std::vector<NodeWrapper> >& pool,
		    Parameters& probability);
    
    int_t get_random_id(librandom::RngPtr& rng);

    bool is_set(index pool_size) const;

  private:
    std::vector<GroupedExponentiation> grouped_exponentiation_list_;
    std::vector<double_t> cumulative_probabilities_;
    int_t length_;

    // Status parameter
    bool set_;
    index set_exception_;
  };
}

#endif
