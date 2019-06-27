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

enum severity_t
{
  M_ALL = 0,
  M_DEBUG = 5,
  M_STATUS = 7,
  M_INFO = 10,
  M_PROGRESS = 15,
  M_DEPRECATED = 18,
  M_WARNING = 20,
  M_ERROR = 30,
  M_FATAL = 40,
  M_QUIET = 100
};

typedef void ( *deliver_logging_event_ptr )( const LoggingEvent& e );
}

#endif
