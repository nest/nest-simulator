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

// C++ includes:
#include <string>
#include <vector>

// Includes from libnestutil:
#include "logging.h"
#include "manager_interface.h"

// Includes from sli:
#include "dictdatum.h"

// Inclused from nestkernel:
#include "nest_names.h"

class Dictionary;

namespace nest
{

class LoggingEvent;

class LoggingManager : public ManagerInterface
{
public:
  LoggingManager();

  virtual void initialize();
  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  /**
   * Register a logging client.
   *
   * Register a callback function that will receive all subsequent
   * LoggingEvents. For the method signature see logging.h .
   */
  void register_logging_client( const deliver_logging_event_ptr callback );

  /**
   * Set the logging level.
   *
   * All logging messages with a lower severity will not be
   * forwarded to the logging clients.
   */
  void set_logging_level( const severity_t level );

  /**
   * Get the current logging level.
   */
  severity_t get_logging_level() const;

  /**
   * Create a LoggingEvent.
   *
   * This function creates a LoggingEvent that will be delivered to all
   * registered logging clients, if the severity is above the set logging
   * level. Do not use this function to do actuall logging in the source code,
   * insted use the LOG() function provided by the logging.h header.
   *
   */
  void publish_log( const severity_t, const std::string&, const std::string&, const std::string&, const size_t ) const;

  /**
   * Implements standard behaviour for dictionary entry misses. Use with define
   * ALL_ENTRIES_ACCESSED.
   */
  void all_entries_accessed( const Dictionary&,
    const std::string&,
    const std::string&,
    const std::string&,
    const size_t ) const;

  void all_entries_accessed( const Dictionary&,
    const std::string&,
    const std::string&,
    const std::string&,
    const std::string&,
    const size_t ) const;

private:
  /**
   * Delivers a LoggingEvent to all registered clients.
   *
   * It iterates all callback from the client_callbacks_ and calls it with the
   * LoggingEvent as argument.
   */
  void deliver_logging_event_( const LoggingEvent& event ) const;

  void default_logging_callback_( const LoggingEvent& event ) const;

private:
  std::vector< deliver_logging_event_ptr > client_callbacks_;
  nest::severity_t logging_level_;
  bool dict_miss_is_error_; //!< whether to throw exception on missed dictionary
                            //!< entries
};
} // namespace nest

#endif // ifndef LOGGING_MANAGER_H
