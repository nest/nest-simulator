/*
 *  completed_checker.cpp
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

#include "completed_checker.h"

namespace nest
{

CompletedChecker::CompletedChecker()
  : a_( NULL )
  , size_( 0 )
{
}

CompletedChecker::~CompletedChecker()
{
  clear();
}

bool
CompletedChecker::all_false() const
{
#pragma omp barrier
  for ( size_t i = 0; i < size_; ++i )
  {
    if ( a_[ i ] )
    {
      return false;
    }
  }
  return true;
}

bool
CompletedChecker::all_true() const
{
#pragma omp barrier
  for ( size_t i = 0; i < size_; ++i )
  {
    if ( not a_[ i ] )
    {
      return false;
    }
  }
  return true;
}

void
CompletedChecker::clear()
{
  VPManager::assert_single_threaded();
  if ( a_ != NULL )
  {
    delete a_;
    a_ = NULL;
    size_ = 0;
  }
}

void
CompletedChecker::resize( const size_t new_size, const bool v )
{
  VPManager::assert_single_threaded();
  clear();
  a_ = new bool[ new_size ];
  for ( size_t i = 0; i < new_size; ++i )
  {
    a_[ i ] = v;
  }
  size_ = new_size;
}

} /* namespace nest */
