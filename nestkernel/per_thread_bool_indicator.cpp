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

BoolIndicatorUInt64& PerThreadBoolIndicator::operator[]( const thread tid )
{
  return per_thread_status_[ tid ];
}

void
PerThreadBoolIndicator::initialize( const thread num_threads, const bool status )
{
  VPManager::assert_single_threaded();
  per_thread_status_.clear();
  per_thread_status_.resize( num_threads, BoolIndicatorUInt64( status ) );
}

bool
PerThreadBoolIndicator::all_false() const
{
#pragma omp barrier
  for ( auto it = per_thread_status_.begin(); it < per_thread_status_.end(); ++it )
  {
    if ( it->is_true() )
    {
      return false;
    }
  }
  return true;
}

bool
PerThreadBoolIndicator::all_true() const
{
#pragma omp barrier
  for ( auto it = per_thread_status_.begin(); it < per_thread_status_.end(); ++it )
  {
    if ( it->is_false() )
    {
      return false;
    }
  }
  return true;
}

bool
PerThreadBoolIndicator::any_false() const
{
#pragma omp barrier
  for ( auto it = per_thread_status_.begin(); it < per_thread_status_.end(); ++it )
  {
    if ( it->is_false() )
    {
      return true;
    }
  }
  return false;
}

bool
PerThreadBoolIndicator::any_true() const
{
#pragma omp barrier
  for ( auto it = per_thread_status_.begin(); it < per_thread_status_.end(); ++it )
  {
    if ( it->is_true() )
    {
      return true;
    }
  }
  return false;
}

} /* namespace nest */
