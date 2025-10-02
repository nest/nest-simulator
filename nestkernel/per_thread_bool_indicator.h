/*
 *  per_thread_bool_indicator.h
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

#ifndef PER_THREAD_BOOL_INDICATOR_H
#define PER_THREAD_BOOL_INDICATOR_H

// C++ includes:
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>

// Includes from nestkernel:
#include "nest_types.h"
#include "vp_manager.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * A wrapper class for an integer that is only allowed to take the
 * values 0 and 1. Used by PerThreadBoolIndicator to create a
 * thread-safe vector indicating per-thread status. See issue #1394.
 */
class BoolIndicatorUInt64
{
public:
  BoolIndicatorUInt64();
  BoolIndicatorUInt64( const bool status );

  bool is_true() const;
  bool is_false() const;


protected:
  void set_true();
  void set_false();
  void logical_and( const bool status );

private:
  static constexpr std::uint_fast64_t true_uint64 = true;
  static constexpr std::uint_fast64_t false_uint64 = false;
  std::uint_fast64_t status_;
  friend class PerThreadBoolIndicator;
};


/**
 * A thread-safe vector to keep track of the status across threads,
 * for example during gather operations. Uses a vector of integers
 * instead of a vector of bools to guarantee thread safety.
 * See issue #1394.
 */
class PerThreadBoolIndicator
{
public:
  PerThreadBoolIndicator() {};

  BoolIndicatorUInt64& operator[]( const size_t tid );

  void set_true( const size_t tid );

  void set_false( const size_t tid );

  void logical_and( const size_t tid, const bool status );

  /**
   * Resize to the given number of threads and set all elements to false.
   */
  void initialize( const size_t num_threads, const bool status );

  /**
   * Waits for all threads and returns whether all elements are false.
   */
  bool all_false() const;

  /**
   * Waits for all threads and returns whether all elements are true.
   */
  bool all_true() const;

  /**
   * Waits for all threads and returns whether any elements are false.
   */
  bool any_false() const;

  /**
   * Waits for all threads and returns whether any elements are true.
   */
  bool any_true() const;

private:
  std::vector< BoolIndicatorUInt64 > per_thread_status_;
  int size_ { 0 };

  /** Number of per-thread indicators currently true
   *
   * are_true_ == 0 -> all are false
   * are_true_ == size_  -> all are true
   * 0 < are_true_ < size_  -> some true, some false
   */
  std::atomic< int > are_true_ { 0 };
};

} // namespace nest

#endif /* PER_THREAD_BOOL_INDICATOR_H */
