/*
 *  logging_event.cpp
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

#include "logging_event.h"

#include <ctime>
#include <cassert>

nest::LoggingEvent::LoggingEvent( const nest::severity_t s,
  const std::string& fctn,
  const std::string& msg,
  const char* file,
  const size_t line )
  : message( msg )
  , function( fctn )
  , severity( s )
  , time_stamp( 0 )
  , file_name( file )
  , line_number( line )
{
  assert( severity > M_ALL );
  assert( severity < M_QUIET );
  time( const_cast< time_t* >( &time_stamp ) );
}

namespace nest
{

std::ostream& operator<<( std::ostream& out, const LoggingEvent& e )
{
  struct tm* ptm = localtime( &e.time_stamp );
  out << "LoggingEvent(Severity = " << e.severity << ", " << e.function << ", '" << e.message
      << "', " << ptm->tm_year + 1900 << "." << ptm->tm_mon + 1 << "." << ptm->tm_mday << " "
      << ptm->tm_hour << ":" << ptm->tm_min << ":" << ptm->tm_sec << ", " << e.file_name << ":"
      << e.line_number << ")";
  return out;
}
}
