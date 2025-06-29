/*
 *  logging.h
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

#ifndef LOGGING_H
#define LOGGING_H

/**
 *
 */
#define LOG( s, fctn, msg ) nest::kernel().logging_manager.publish_log( ( s ), ( fctn ), ( msg ), __FILE__, __LINE__ )

/**
 *
 */
#define ALL_ENTRIES_ACCESSED( d, fctn, msg ) \
  nest::kernel().logging_manager.all_entries_accessed( ( d ), ( fctn ), ( msg ), __FILE__, __LINE__ )

/**
 *
 */
#define ALL_ENTRIES_ACCESSED2( d, fctn, msg1, msg2 ) \
  nest::kernel().logging_manager.all_entries_accessed( ( d ), ( fctn ), ( msg1 ), ( msg2 ), __FILE__, __LINE__ )

namespace nest
{

class LoggingEvent;
class LoggingDeliverer;

//! Report only messages at levels higher than chosen level to user or logs. Default INFO.
enum class VerbosityLevel
{
  ALL = 0,
  DEBUG = 5,
  STATUS = 7,
  INFO = 10,
  PROGRESS = 15,
  DEPRECATED = 18,
  WARNING = 20,
  ERROR = 30,
  FATAL = 40,
  QUIET = 100
};

typedef void ( *deliver_logging_event_ptr )( const LoggingEvent& e );
}

#endif
