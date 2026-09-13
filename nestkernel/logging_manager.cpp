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


namespace nest
{
LoggingManager::LoggingManager()
  : client_callbacks_()
  , logging_level_( VerbosityLevel::INFO )
  , dict_miss_is_error_( true )
{
}

LoggingManager::~LoggingManager()
{
}

void
LoggingManager::initialize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( not adjust_number_of_threads_or_rng_only )
  {
    dict_miss_is_error_ = true;
  }
}

void
LoggingManager::finalize( const bool )
{
}

void
LoggingManager::set_status( const Dictionary& dict )
{
  dict.update_value( names::dict_miss_is_error, dict_miss_is_error_ );
  dict.update_value( names::verbosity, logging_level_ );  // safe, because entry must be VerbosityLevel
}

void
LoggingManager::get_status( Dictionary& dict )
{
  dict[ names::dict_miss_is_error ] = dict_miss_is_error_;
  dict[ names::verbosity ] = logging_level_;
}


void
LoggingManager::register_logging_client( const deliver_logging_event_ptr callback )
{
  assert( callback );

  client_callbacks_.push_back( callback );
}

void
LoggingManager::deliver_logging_event_( const LoggingEvent& event ) const
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
LoggingManager::default_logging_callback_( const LoggingEvent& event ) const
{
  std::ostream* out;

  if ( event.severity < VerbosityLevel::WARNING )
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
LoggingManager::publish_log( const VerbosityLevel s,
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

}  // namespace nest
