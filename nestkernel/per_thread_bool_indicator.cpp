/*
 *  per_thread_bool_indicator.cpp
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

#include "per_thread_bool_indicator.h"

// Includes from nestkernel
#include "kernel_manager.h"

#include "simulation_manager.h"

namespace nest
{

BoolIndicatorUInt64::BoolIndicatorUInt64()
  : status_( false_uint64 )
{
}

BoolIndicatorUInt64::BoolIndicatorUInt64( const bool status )
  : status_( status )
{
}

BoolIndicatorUInt64&
PerThreadBoolIndicator::operator[]( const size_t tid )
{
  return per_thread_status_[ tid ];
}

void
PerThreadBoolIndicator::initialize( const size_t num_threads, const bool status )
{
  kernel::manager< VPManager >.assert_single_threaded();
  per_thread_status_.clear();
  per_thread_status_.resize( num_threads, BoolIndicatorUInt64( status ) );
  size_ = num_threads;
  if ( status )
  {
    are_true_ = num_threads;
  }
  else
  {
    are_true_ = 0;
  }
}

bool
PerThreadBoolIndicator::all_false() const
{
  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
// We need two barriers here to ensure that no thread can continue and change the result
// before all threads have determined the result.
#pragma omp barrier
  // We need two barriers here to ensure that no thread can continue and change the result
  // before all threads have determined the result.
  bool ret = ( are_true_ == 0 );
#pragma omp barrier

  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();
  return ret;
}

bool
PerThreadBoolIndicator::all_true() const
{
  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
  bool ret = ( are_true_ == size_ );
#pragma omp barrier
  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();
  return ret;
}

bool
PerThreadBoolIndicator::any_false() const
{
  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
  bool ret = ( are_true_ < size_ );
#pragma omp barrier

  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();
  return ret;
}

bool
PerThreadBoolIndicator::any_true() const
{
  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().start();
#pragma omp barrier
  bool ret = ( are_true_ > 0 );
#pragma omp barrier

  kernel::manager< SimulationManager >.get_omp_synchronization_construction_stopwatch().stop();
  return ret;
}

bool
BoolIndicatorUInt64::is_true() const
{
  return ( status_ == true_uint64 );
}

bool
BoolIndicatorUInt64::is_false() const
{
  return ( status_ == false_uint64 );
}

void
BoolIndicatorUInt64::set_true()
{
  status_ = true_uint64;
}

void
BoolIndicatorUInt64::set_false()
{
  status_ = false_uint64;
}

void
BoolIndicatorUInt64::logical_and( const bool status )
{
  status_ = ( static_cast< bool >( status_ ) and status );
}

void
PerThreadBoolIndicator::set_true( const size_t tid )
{
  if ( per_thread_status_[ tid ].is_false() )
  {
    are_true_++;
    per_thread_status_[ tid ].set_true();
  }
}

void
PerThreadBoolIndicator::set_false( const size_t tid )
{
  if ( per_thread_status_[ tid ].is_true() )
  {
    are_true_--;
    per_thread_status_[ tid ].set_false();
  }
}

void
PerThreadBoolIndicator::logical_and( const size_t tid, const bool status )
{
  if ( per_thread_status_[ tid ].is_true() and not status )
  {
    are_true_--;
    per_thread_status_[ tid ].set_false();
  }
}
} // namespace nest
