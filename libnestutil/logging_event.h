/*
 *  logging_event.h
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


#ifndef LOGGING_EVENT_H
#define LOGGING_EVENT_H

// C++ includes:
#include <ostream>
#include <string>

// Includes from libnestutil:
#include "logging.h"

namespace nest
{

class LoggingEvent
{
public:
  LoggingEvent( const severity_t s,
    const std::string& fctn,
    const std::string& msg,
    const std::string& file = "none",
    const size_t line = 0 );

  friend std::ostream& operator<<( std::ostream&, const LoggingEvent& );

public:
  const std::string& message;
  const std::string& function;
  const severity_t severity;
  const time_t time_stamp;
  const std::string& file_name;
  const size_t line_number;
};

} // namespace nest

#endif // ifndef LOGGING_EVENT_H
