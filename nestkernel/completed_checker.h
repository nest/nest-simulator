/*
 *  completed_checker.h
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

#ifndef COMPLETED_CHECKER_H
#define COMPLETED_CHECKER_H

// C++ includes:
#include <cassert>
#include <cstddef>

// Includes from nestkernel:
#include "nest_types.h"
#include "vp_manager.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * A thread-safe array to coordinate progress across threads during
 * gather operations. Uses an array of bools instead of an STL vector,
 * since omp atomic operations can not be applied to vector
 * assignments.
 */

class CompletedChecker
{
private:
  /**
   * Array holding status values for all threads. Must be of type
   * bool; 'bitwise and' is used below instead of 'logical and'.
   */
  bool* a_;

  /**
   * Size of a_, should always be identical to number of threads.
   */
  size_t size_;

public:
  CompletedChecker();

  ~CompletedChecker();

  /**
   *  Returns whether all elements are false. Waits for all threads.
   */
  bool all_false() const;

  /**
   * Returns whether all elements are true. Waits for all threads.
   */
  bool all_true() const;

  /**
   * Clears array and sets size to zero.
   */
  void clear();

  /**
   * Updates element for thread tid by computing its 'logical and'
   * with given value v.
   */
  void logical_and( const thread tid, const bool v );

  /**
   * Resizes array to given size.
   */
  void resize( const size_t new_size, const bool v );

  /**
   * Sets element for thread tid to given value v.
   */
  void set( const thread tid, const bool v );

  /**
   * Returns const reference to element at position tid.
   */
  bool operator[]( const thread tid ) const;
};

inline void
CompletedChecker::logical_and( const thread tid, const bool v )
{
// Use 'bitwise and', since 'logical and' is not supported by 'omp
// atomic update'; yields same result for bool.
#pragma omp atomic update
  a_[ tid ] &= v;
}

inline void
CompletedChecker::set( const thread tid, const bool v )
{
#pragma omp atomic write
  a_[ tid ] = v;
}

inline bool CompletedChecker::operator[]( const thread tid ) const
{
  return a_[ tid ];
}

} // namespace nest

#endif /* COMPLETED_CHECKER_H */
