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

// C++ includes:
#include <cassert>
#include <iostream>

// Includes from libnestutil:
#include "logging_event.h"

// Includes from thirdparty:
#include "compose.hpp"


nest::LoggingManager::LoggingManager()
  : client_callbacks_()
  , logging_level_( M_ALL )
  , dict_miss_is_error_( true )
{
}

nest::LoggingManager::~LoggingManager()
{
}

void
nest::LoggingManager::initialize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( not adjust_number_of_threads_or_rng_only )
  {
    dict_miss_is_error_ = true;
  }
}

void
nest::LoggingManager::finalize( const bool )
{
}

void
nest::LoggingManager::set_status( const dictionary& dict )
{
  dict.update_value( names::dict_miss_is_error, dict_miss_is_error_ );

  severity_t level = logging_level_;
  if ( dict.update_value( names::verbosity, level ) )
  {
    if ( level < M_ALL or M_QUIET < level )
    {
      throw BadParameter(
        String::compose( "Verbosity level must be between M_ALL (%1) and M_QUIET (%2).", M_ALL, M_QUIET ) );
    }
    logging_level_ = level;
  }
}

void
nest::LoggingManager::get_status( dictionary& dict )
{
  dict[ names::dict_miss_is_error ] = dict_miss_is_error_;
  dict[ names::verbosity ] = logging_level_;
}


void
nest::LoggingManager::register_logging_client( const deliver_logging_event_ptr callback )
{
  assert( callback );

  client_callbacks_.push_back( callback );
}

void
nest::LoggingManager::deliver_logging_event_( const LoggingEvent& event ) const
{
  if ( client_callbacks_.empty() )
  {
    default_logging_callback_( event );
  }
  for ( const auto& client_callback : client_callbacks_ )
  {
    client_callback( event );
  }
}

void
nest::LoggingManager::default_logging_callback_( const LoggingEvent& event ) const
{
  std::ostream* out;

  if ( event.severity < M_WARNING )
  {
    out = &std::cout;
  }
  else
  {
    out = &std::cerr;
  }

  *out << event << std::endl;
}

void
nest::LoggingManager::publish_log( const nest::severity_t s,
  const std::string& fctn,
  const std::string& msg,
  const std::string& file,
  const size_t line ) const
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
