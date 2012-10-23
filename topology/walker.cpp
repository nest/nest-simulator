/*
 *  walker.cpp
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
#include "walker.h"

#include "randomgen.h"

#include <list>

#include "stdlib.h"
#include <algorithm>

namespace nest
{

  Walker::Walker():
    P_(),
    Y_(),
    set_(false),
    set_exception_(0)
  {}

  void Walker::set_fixed(index n = 0)
  {
    set_ = true;
    set_exception_ = n;
  }

  void Walker::initialise(NodeWrapper& driver,
			  lockPTR<std::vector<NodeWrapper> >& pool,
			  Parameters& probability)
  {
    // Resize P_ and Y_
    P_ = std::vector<double_t>(pool->size(), -1.0);
    Y_ = std::vector<int_t>(pool->size(), -1);
    
    // Creates a list of the local id of pool nodes and their probability.
    // In Knuth's description of this algorithm the local ids start at 1.
    // In our implementation we'll set the first local id to 0. Knuth does
    // this because he's working with a pool of samples between x_1 and x_k.
    // We're working with local id's between 0 and k-1. The rest of this
    // implementation of walker's alias takes this change into account.

    std::vector<Walker::Pair> probability_list;
    probability_list.reserve(pool->size());
    
    for(uint_t i = 0; i<pool->size(); ++i)
      {
	probability_list.
	  push_back(Walker::Pair(probability.get_value(driver.get_position(),
						       pool->at(i).get_position()), 
				 i));
      } 

    // Convert probabilities to relative probabilities.
    double_t sum = 0.0;
    
    for(std::vector<Walker::Pair>::iterator it = probability_list.begin();
	it != probability_list.end(); ++it)
      {
	sum += (*it).probability;
      }
    
    for(std::vector<Walker::Pair>::iterator it = probability_list.begin();
	it != probability_list.end(); ++it)
      {
	(*it).probability /= sum;
      }

    // Sort probability list (the list might already be in some form of 
    // presorted format).
    std::sort(probability_list.begin(), probability_list.end());

    assert(probability_list.size() != 0);

    // The size of the probability list might be altered so the 
    // starting length is stored in the variable k.    
    const int_t k = probability_list.size();

    // Instead of removing the first and last element of the probability 
    // vector for each run of the loop in Walker's algorithm we create two
    // iterators which always points to the current start and end element 
    // in the vector. Alternatively a std::list container could be used
    // (with both start and end access), but after testing of both the 
    // std::vector proved faster for this case.
    //
    // Three pairs of begin/end iterators are kept. One for the probability
    // list, one for a temporary insertion list (where newly generated
    // data pairs are temporarily stored), and one pointing to the 
    // current smallest and largest element in the probability or the 
    // insertion list.

    // Iterators pointing to the smallest and largest data pairs.
    std::vector<Walker::Pair>::iterator begin = probability_list.begin();
    std::vector<Walker::Pair>::iterator end = probability_list.end();

    // The size of the insertion list should be set so that there is
    // a balance between the cost of merging the insertion list and 
    // the probability list and inserting new elements in the insertion 
    // list. 
    std::vector<Walker::Pair> insertion_list(2000);

    // The beginning and end of the probability vector have already 
    // been retrieved by the common begin and end iterators. The 
    // begin_p and end_p are for this reason shifted one step. 
    std::vector<Walker::Pair>::iterator begin_p = probability_list.begin()+1;
    std::vector<Walker::Pair>::iterator end_p = probability_list.end()-1;
    
    // The insertion list is empty so both iterators point to the
    // beginning of the list.
    std::vector<Walker::Pair>::iterator begin_i = insertion_list.begin()+1000;
    std::vector<Walker::Pair>::iterator end_i = insertion_list.begin()+1000;

    // Add elements to the P_ and Y_ vectors as long as the probability
    // vector isn't empty. This algorithm follows a description given
    // by Knuth.
    //
    // At this point the insertion list is empty and the insertion
    // list iterators are both pointing at the first element of the
    // list. The probability list contain at least one element.
    // and the probability list iterators point to the element one
    // step past the beginning and one step before the end of the list. 
    // If the probability list only had one element the end_p iterator
    // will have passed the begin_p iterator (i.e. end_p-begin_p < 0) and
    // the while loop will be aborted.
    //
    // Execute loop as long as there exists more than 1 elements in the 
    // probability and insertion list. The removal of the last element from
    // the lists directs the common end and begin iterators to the last 
    // element and causes the end and begin iterators of the list where the
    // last element was found to pass one another, and the loop will be aborted.
    // The last element is inserted in the P_ and Y_ vector below the while
    // loop.
    while((end_p - begin_p) >= 0 && (end_i - begin_i) >= 0)
      {
	// P_ and Y_ elements are initialised at random, dependent upon
	// the probability list. 
	P_.at(begin->id) = 
	  k*begin->probability;
	
	Y_.at(begin->id) = 
	  (end-1)->id;

	// Create new pair element to be inserted in the lists.
	double_t q = 
	  (end-1)->probability-
	  (1.0/k)+begin->probability;
	
	int_t a = (end-1)->id;
	
	// After the run of the insert_pair() command we are guaranteed
	// that the insertion list contain at least one remaining
	// element (i.e. end_i > begin_i).
	assert(begin_i != insertion_list.begin());
	assert(end_i != insertion_list.end());
	
	if(end_i - begin_i > 100 && 
	   Walker::Pair(q, a) < *(begin_i + (end_i-begin_i)/2))
	  {
	    insert_left(--begin_i, end_i, Walker::Pair(q, a));
	  }
	else
	  {
	    insert_right(begin_i, ++end_i, Walker::Pair(q, a));
	  }

	// Move the insertion_list elements to the 
	// probability_list if the end of the insertion_list
	// is reached.
	if(end_i == insertion_list.end() ||
	   begin_i == insertion_list.begin())
	  {
	    // In the case where the insertion list hasn't been 
	    // utilised completely it might be usefull to 
	    // re-order the insertion list instead of performing
	    // the expensive job of merging the probability and the
	    // insertion list. This is known to be the case when 
	    // all the probability values in the data list are 
	    // equal.
	    if(end_i - begin_i < 500)
	      {
		shift_pairs(begin_i, end_i, insertion_list.begin()+750);

		begin = (end_p != begin_p &&
			 begin_p->probability <= begin_i->probability) ? 
		  begin_p++ : begin_i++;
		
		end = (end_p != begin_p && 
		       (end_p-1)->probability >= (end_i-1)->probability) ?
		  end_p-- : end_i--;
	      }
	    else
	      {
		assert(begin_p - probability_list.begin() >=
		       end_i - begin_i);
	    
		// Merge the probability and insertion list.
		// The merge is done in place in the free
		// space in the start of the probability list.
		// For every element in the insertion list we
		// are guaranteed that one element has been 
		// removed from the probability list so this
		// merge is safe.
		end_p = std::merge(begin_p, end_p,
				   begin_i, end_i,
				   begin_p - (end_i-begin_i));

		begin_p = begin_p - (end_i-begin_i);

		// Re-initialise other iterators.
		begin = begin_p++;
		end = end_p--;
		
		begin_i = insertion_list.begin()+1000;
		end_i = begin_i;
		
		// At this point we reached the starting conditions 
		// of the while-loop again.
	      }
	  }
	else
	  {
	    // If end_p == begin_p and end_i - begin_i == 1, 
	    // the lines below will move the end_i 
	    // iterator to the left of the begin_i operator. 
	    // The begin and end iterators will be set to the 
	    // last remaining element and the loop exited.
	    //
	    // If end_p == begin_p elements are automatically 
	    // picked from the insertion_list.

	    begin = (end_p != begin_p &&
		     begin_p->probability <= begin_i->probability) ? 
	      begin_p++ : begin_i++;

	    end = (end_p != begin_p && 
		   (end_p-1)->probability >= (end_i-1)->probability) ?
	      end_p-- : end_i--;
	  }
      }

    // Insert the last element in the P_ and Y_ vector.
    P_.at(begin->id) = 
      k*begin->probability;
    
    Y_.at(begin->id) = 
      (end-1)->id;

    test_validity();
  }

  bool Walker::test_validity()
  {
    for(std::vector<double_t>::iterator it = P_.begin();
	it != P_.end(); ++it)
      {
	assert(*it >= 0);
      }
    
    for(std::vector<int_t>::iterator it = Y_.begin();
	it != Y_.end(); ++it)
      {
        assert(*it >= 0);
      }

    return true;
  }

  int_t Walker::get_random_id(librandom::RngPtr& rng)
  {
    // Use Walker's algorithm to find the correct id from the
    // Y_ vector.
    const double_t r = rng->drand()*P_.size();

    // Get the integer part of the random number.
    uint_t K = static_cast<uint_t>(r);
    // Get the decimal (fraction) part of the random number
    const double_t V = r - K;
    
    if(K == P_.size())
      {
	// What should be done about this case????
	K -= 1;
      }

    if(V < P_.at(K))
      {
	// In Knuth's description of Walker's alias algorithm 
	// X_(K+1) is returned at this point. We want values
	// starting from 0 rather than 1, and return for this 
	// reason simply K.
	return K;
      }
    else
      {
	return Y_.at(K);
      }
  }

  
  bool Walker::is_set(index pool_size) const
  {
    // The Walker object is re-calculated every time for freely
    // distributed layers. For fixed grid layers the object is
    // re-computed if the pool size is different from a set
    // exception size. The exception size should be set to the max
    // size of the mask. The object is also re-calculated every
    // time the size of the pool changes.
    return set_ && pool_size == set_exception_ && P_.size() == pool_size;
  }

}
