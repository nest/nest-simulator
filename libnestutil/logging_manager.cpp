/*
 *  logging_manager.cpp
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

#include "logging_manager.h"
#include "logging_event.h"

#include <cassert>

nest::LoggingManager::LoggingManager()
  : client_callbacks_()
  , logging_level_( M_ALL )
{
}

void
nest::LoggingManager::init()
{
}

void
nest::LoggingManager::reset()
{
}

void
nest::LoggingManager::set_status( const DictionaryDatum& )
{
}

void
nest::LoggingManager::get_status( DictionaryDatum& )
{
}


void
nest::LoggingManager::register_logging_client( const deliver_logging_event_ptr callback )
{
  assert( callback != 0 );

  client_callbacks_.push_back( callback );
}

void
nest::LoggingManager::deliver_logging_event_( const LoggingEvent& event )
{
  std::vector< deliver_logging_event_ptr >::iterator it;
  for ( std::vector< deliver_logging_event_ptr >::const_iterator it = client_callbacks_.begin();
        it != client_callbacks_.end();
        ++it )
  {
    ( *it )( event );
  }
}

void
nest::LoggingManager::publish_log( const nest::severity_t s,
  const std::string& fctn,
  const std::string& msg,
  const char* file,
  const size_t line )
{
  if ( s >= logging_level_ )
  {
    LoggingEvent e( s, fctn, msg, file, line );
#pragma omp critical( logging )
    {
      deliver_logging_event_( e );
    }
  }
}

void
nest::LoggingManager::set_logging_level( const nest::severity_t level )
{
  assert( level >= M_ALL );
  assert( level <= M_QUIET );

  logging_level_ = level;
}

nest::severity_t
nest::LoggingManager::get_logging_level()
{
  return logging_level_;
}