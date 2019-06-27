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

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "sliexceptions.h"

nest::LoggingManager::LoggingManager()
  : client_callbacks_()
  , logging_level_( M_ALL )
  , dict_miss_is_error_( true )
{
}

void
nest::LoggingManager::initialize()
{
  dict_miss_is_error_ = true;
}

void
nest::LoggingManager::finalize()
{
}

void
nest::LoggingManager::set_status( const DictionaryDatum& dict )
{
  updateValue< bool >( dict, names::dict_miss_is_error, dict_miss_is_error_ );
}

void
nest::LoggingManager::get_status( DictionaryDatum& dict )
{
  ( *dict )[ names::dict_miss_is_error ] = dict_miss_is_error_;
}


void
nest::LoggingManager::register_logging_client( const deliver_logging_event_ptr callback )
{
  assert( callback != 0 );

  client_callbacks_.push_back( callback );
}

void
nest::LoggingManager::deliver_logging_event_( const LoggingEvent& event ) const
{
  if ( client_callbacks_.empty() )
  {
    default_logging_callback_( event );
  }
  std::vector< deliver_logging_event_ptr >::iterator it;
  for ( std::vector< deliver_logging_event_ptr >::const_iterator it = client_callbacks_.begin();
        it != client_callbacks_.end();
        ++it )
  {
    ( *it )( event );
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

void
nest::LoggingManager::all_entries_accessed( const Dictionary& d,
  const std::string& where,
  const std::string& msg,
  const std::string& file,
  const size_t line ) const
{
  std::string missed;
  if ( not d.all_accessed( missed ) )
  {
    if ( dict_miss_is_error_ )
    {
      throw UnaccessedDictionaryEntry( missed );
    }
    else
    {
      publish_log( M_WARNING, where, msg + missed, file, line );
    }
  }
}

void
nest::LoggingManager::all_entries_accessed( const Dictionary& d,
  const std::string& where,
  const std::string& msg1,
  const std::string& msg2,
  const std::string& file,
  const size_t line ) const
{
  std::string missed;
  if ( not d.all_accessed( missed ) )
  {
    if ( dict_miss_is_error_ )
    {
      throw UnaccessedDictionaryEntry( missed + "\n" + msg2 );
    }
    else
    {
      publish_log( M_WARNING, where, msg1 + missed + "\n" + msg2, file, line );
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
nest::LoggingManager::get_logging_level() const
{
  return logging_level_;
}
