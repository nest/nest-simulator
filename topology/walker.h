#ifndef WALKER_H
#define WALKER_H

/*
 *  walker.h
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

#include "nodewrapper.h"
#include "parameters.h"

#include "randomgen.h"

#include <algorithm>

namespace nest
{

  /**
   * This class implements Walker's Alias algorithm.
   * The class is used to simplify the drawing of random 
   * nodes done in the TopologyConnector class.
   */
  class Walker//: public NodeSelector
  {

    /**
     * Structure that stores a decimal and an integer number. Could
     * be replaced by std::pair<double_t, int_t>() if 
     * the std::pair < operator was overloaded. The custom made
     * Pair structure seems to be faster however.
     */
    struct Pair
    {
      double_t probability;
      int_t id;

    Pair():
      probability(0.0),
	id(0)
      {}

    Pair(double_t p, int_t i):
      probability(p),
	id(i)
      {}

    Pair(const Pair& p):
      probability(p.probability),
	id(p.id)
      {}

      bool operator<(const Pair& a) const
      {
	return probability < a.probability;
      }

      bool operator>(const Pair& a) const
      {
	return probability > a.probability;
      }


    };

  public:
    /**
     * Create uninitialised Walker object.
     */
    Walker();

    /**
     * Set the variable set_ to true. Used to indicate that
     * it might not be necessary to re-initialise the 
     * Walker object. Walker object always needs to be
     * re-initialised if input pool size to is_set()
     * is equal to exception setting in input parameter 
     * to set_fixed(). This is the case when the walker 
     * object is used on a fixed grid layers without edge
     * wrap and we're vertically shifting a centered mask from 
     * one edge to the next.
     * @param n   exception to set_fixed() rule
     */
    void set_fixed(index n);

    /**
     * Initialise the Walker object (the P_ and Y_ vectors).
     * @param driver node
     * @param pool list of pool nodes with probabilities
     * @param probability 
     */
    void initialise(NodeWrapper& driver,
		    lockPTR<std::vector<NodeWrapper> >& pool,
		    Parameters& probability);

    /**
     * Draw a random ID based upon the P_ and Y_ vector.
     * @param rng Random number generator
     */
    int_t get_random_id(librandom::RngPtr& rng);

    /**
     * If fixed grid layers are used the Walker object only
     * needs to be re-initialised when the number of nodes
     * in the pool region changes. This function indicates 
     * if that is the case.
     * @param pool_size number of nodes in selected pool.
     * @returns true if the Walker object already is properly
     *          initialised.
     */
    bool is_set(index pool_size) const;

  private:
    std::vector<double_t> P_;
    std::vector<int_t> Y_;

    // Status parameter
    bool set_;
    index set_exception_;

    /**
     * Insert a single data pair into a sorted vector. The last element
     * in the vector will be lost in this insertion. For this reason 
     * the vector should always be expanded so that it have space
     * for one extra data pair just prior to calling this function.
     * @param begin 
     * @param end
     * @param pair Probability/ID pair
     *
     */
    void insert_right(const std::vector<Walker::Pair>::iterator& begin,
		      const std::vector<Walker::Pair>::iterator& end_it,
		      const Walker::Pair& pair);

    /**
     * Function is similar to insert_right(), just that the vector
     * should be expanded at the beginning rather than the end. The
     * element referred to by the begin iterator will be lost by calling
     * this function.
     * @param begin 
     * @param end
     * @param pair Probability/ID pair
     *
     */
    void insert_left(const std::vector<Walker::Pair>::iterator& begin,
		     const std::vector<Walker::Pair>::iterator& end_it,
		     const Walker::Pair& pair);

    /**
     * Move a set of nodes to a new position. Similar to std::copy(b,e,n)
     * @param begin iterator pointing to the first of the nodes that
     *              should be moved.
     * @param end   iterator pointing to one position past the last
     *              of the nodes that should be moved.
     * @param new_begin new start position of the node list. 
     */
    void shift_pairs(std::vector<Walker::Pair>::iterator& begin,
		     std::vector<Walker::Pair>::iterator& end,
		     std::vector<Walker::Pair>::iterator new_begin);


    /**
     * Development function. Asserts that all the P_ and Y_ elements have
     * been initialised with reasonable values.
     */
    bool test_validity();

  };

  inline
    void Walker::insert_right(const std::vector<Walker::Pair>::iterator& begin,
			      const std::vector<Walker::Pair>::iterator& end,
			      const Walker::Pair& pair)
  {
    // This function is inspired by an algorithm found in a forum thread
    // on insertion sort from www.cplusplus.com

    // The element pointed to by end-1 should be free. Variables stored at 
    // this position will be lost.
    std::vector<Walker::Pair>::iterator key = end-1;
    
    while(key != begin && *(key-1) > pair)
      {
	std::swap(*key, *(key-1));
	--key;
      }
    *key = pair;
  }

  inline
    void Walker::insert_left(const std::vector<Walker::Pair>::iterator& begin,
			     const std::vector<Walker::Pair>::iterator& end,
			     const Walker::Pair& pair)
  {
    // The element pointed to by begin should be free. Variables stored at 
    // this position will be lost.

    std::vector<Walker::Pair>::iterator key = begin;
    
    while(key != (end-1) && *(key+1) < pair)
      {
	std::swap(*key, *(key+1));
	++key;
      }
    *key = pair;
  }

  inline 
    void Walker::shift_pairs(std::vector<Walker::Pair>::iterator& begin,
			     std::vector<Walker::Pair>::iterator& end,
			     std::vector<Walker::Pair>::iterator new_begin)
  {
    std::vector<Walker::Pair>::iterator key = begin;
    begin = new_begin;

    while(key != end)
      {
	*new_begin = *key;
	++new_begin;
	++key;
      }
    end = new_begin;
  }

}

#endif
