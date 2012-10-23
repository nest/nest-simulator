/*
 *  binomial.cpp
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

#include "binomial.h"

namespace nest
{

  Binomial::Binomial():
    grouped_exponentiation_list_(),
    cumulative_probabilities_(),
    length_(0),
    set_(false), 
    set_exception_(0)
  {}

  void Binomial::set_fixed(index n=0)
  {
    set_ = true;
    set_exception_ = n;
  }

  void Binomial::initialise(NodeWrapper& driver,
			    lockPTR<std::vector<NodeWrapper> >& pool,
			    Parameters& probability)
  {
    // Get list of probabilities
    std::vector<double_t> probability_list(pool->size(), 0.0);

    for(uint_t i = 0; i < pool->size(); ++i)
      {
	probability_list.at(i) = 
	  probability.get_value(driver.get_position(),
				pool->at(i).get_position());
      }
      
    // Convert probabilities to relative probabilities.
    double_t sum = 0.0;
      
    for(uint_t i = 0; i < probability_list.size(); ++i)
      {
	sum += probability_list.at(i);
      }

    for(uint_t i = 0; i < probability_list.size(); ++i)
      {
	probability_list.at(i) /= sum;
      }

    length_ = probability_list.size();

    // Express probability list in terms of exponentiations 
    // with 2 as the base.
    std::vector<Exponentiation> exponentiation_list(pool->size(), 
						    Exponentiation());
      
    for(uint_t i = 0; i < pool->size(); ++i)
      {
	exponentiation_list.at(i) = 
	  Exponentiation(probability_list.at(i), i);
      }

    // Sort new probability list with probability with lowest 
    // exponent value first.
    std::sort(exponentiation_list.begin(), exponentiation_list.end());
      
    // Group new probability list in groups with same exponent
    // value.

    grouped_exponentiation_list_.clear();

    // Exponents start at 0 and decrease
    int_t last = std::numeric_limits<int_t>::max();

    for(int_t i = pool->size()-1; i > -1; --i)
      {
	if(exponentiation_list.at(i).exponent != last)
	  {
	    // Start on new group.
	    grouped_exponentiation_list_.
	      push_back(GroupedExponentiation());

	    last = exponentiation_list.at(i).exponent;
	  }

	// Add element to current group.
	grouped_exponentiation_list_.back().
	  push_back(exponentiation_list.at(i));
      }

    // Convert the grouped exponentiation probabilities to  
    // cumulutative probabilities.

    assert(grouped_exponentiation_list_.size() != 0);
    cumulative_probabilities_ = 
      std::vector<double_t>(grouped_exponentiation_list_.size(), 0.0);

    cumulative_probabilities_.at(0) = 
      grouped_exponentiation_list_.at(0).total_probability;

    for(uint_t i = 1; i < grouped_exponentiation_list_.size(); ++i)
      {
	cumulative_probabilities_.at(i) =  
	  grouped_exponentiation_list_.at(i).total_probability +
	  cumulative_probabilities_.at(i-1);
      }

    // Sum of cumulative_probabilites_ is 1.0 (100%)
    // Adjust for any uncertainties that may occur due to 
    // rounding of probability values.
    assert(cumulative_probabilities_.back() < 1.001 &&
	   cumulative_probabilities_.back() > 0.999);
    cumulative_probabilities_.back() = 1.0;
  }

  int_t Binomial::get_random_id(librandom::RngPtr& rng)
  {
    // Draw random number and pick exponentiation group

    // Pick correct exponentiation group
    const double_t r = rng->drand();
	
    int_t i = 0;
	
    while(r > cumulative_probabilities_.at(i))
      {
	++i;
      }

    int_t j = -1;

    do{
      // Pick node from within selected 
      // exponentiation group.
      j = 
	rng->ulrand(grouped_exponentiation_list_.at(i).size());

      // Draw an additional random number and continue picking 
      // nodes as long as the new random number is less than 
      // the probability identified by the first random number.
      // The second random number should be between 0 and the maximum
      // probability in the group of exponentiations selected.
      // random * 2^group_exponent < selected probability
      // --> random < selected mantissa
    }while(rng->drand() < 
	   grouped_exponentiation_list_.at(i).at(j).mantissa);
    
    return grouped_exponentiation_list_.at(i).at(j).lid;
  }

  bool Binomial::is_set(index pool_size) const
  {
    return set_ && pool_size == set_exception_ && static_cast<index>(length_) == pool_size;
  }
  
}
