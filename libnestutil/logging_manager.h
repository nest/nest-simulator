/*
 *  logging_manager.h
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

#ifndef LOGGING_MANAGER_H
#define LOGGING_MANAGER_H

#include "logging.h"
#include "manager_interface.h"
#include "dictdatum.h"

#include <vector>
#include <string>

namespace nest
{

class LoggingEvent;

class LoggingManager : ManagerInterface
{
public:
  LoggingManager();

  virtual void init();
  virtual void reset();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  void register_logging_client( const deliver_logging_event_ptr callback );

  void set_logging_level( const severity_t level );

  severity_t get_logging_level();

  void
  publish_log( const severity_t, const std::string&, const std::string&, const char*, const size_t );

private:
  void deliver_logging_event_( const LoggingEvent& event );

private:
  std::vector< deliver_logging_event_ptr > client_callbacks_;
  nest::severity_t logging_level_;
};
} // namespace nest

#endif // ifndef LOGGING_MANAGER_H
