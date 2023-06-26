/*
 *  io_manager.h
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

#ifndef IO_MANAGER_H
#define IO_MANAGER_H

// C++ includes:
#include <string>

// Includes from libnestutil:
#include "manager_interface.h"

#include "recording_backend.h"
#include "stimulation_backend.h"

namespace nest
{

/*
  IOManager: Handles data storage files from spike recorders and
  multimeters to file system(s)/memory/output. Distinct from logging
  for error streams.
*/
class IOManager : public ManagerInterface
{
public:
  IOManager();
  ~IOManager() override;

  void initialize() override;
  void finalize() override;
  void change_number_of_threads() override;
  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) override;

  void set_recording_backend_status( std::string, const DictionaryDatum& );
  DictionaryDatum get_recording_backend_status( std::string );

  /**
   * The prefix for files written by devices.
   * The prefix must not contain any part of a path.
   * @see get_data_dir(), overwrite_files()
   */
  const std::string& get_data_prefix() const;

  /**
   * The path for files written by devices.
   * It may be the empty string (use current directory).
   * @see get_data_prefix(), overwrite_files()
   */
  const std::string& get_data_path() const;

  /**
   * Indicate if existing data files should be overwritten.
   * @return true if existing data files should be overwritten by devices.
   * Default: false.
   */
  bool overwrite_files() const;

  /**
   * Clean up in all registered recording backends after a single call to run by
   * calling the backends' post_run_hook() functions
   */
  void post_run_hook();
  void pre_run_hook();

  /**
   * Clean up in all registered recording backends after a single simulation
   * step by calling the backends' post_step_hook() functions
   */
  void post_step_hook();

  /**
   * Finalize all registered recording backends after a call to
   * SimulationManager::simulate() or SimulationManager::cleanup() by
   * calling the backends' finalize() functions
   */
  void cleanup() override;
  void prepare() override;

  template < class RecordingBackendT >
  void register_recording_backend( Name );

  template < class StimulationBackendT >
  void register_stimulation_backend( const Name );

  bool is_valid_recording_backend( const Name ) const;
  bool is_valid_stimulation_backend( const Name ) const;

  /**
   * Send device data to a given recording backend.
   *
   * This function is called from a RecordingDevice `device` when it
   * wants to write data to a given recording backend, identified by
   * its `backend_name`. The function takes an Event `event` from
   * which some fundamental data is taken and additionally vectors of
   * `double_values` and `long_values` that have to be written. The
   * data vectors may be empty, if no additional data has to be
   * written.
   *
   * \param backend_name the name of the RecordingBackend to write to
   * \param device a reference to the RecordingDevice that wants to write
   * \param event the Event to be written
   * \param double_values a vector of doubles to be written
   * \param long_values a vector of longs to be written
   */
  void write( const Name backend_name,
    const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& double_values,
    const std::vector< long >& long_values );

  void enroll_recorder( const Name, const RecordingDevice&, const DictionaryDatum& );
  void enroll_stimulator( const Name, StimulationDevice&, const DictionaryDatum& );

  void set_recording_value_names( const Name backend_name,
    const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names );

  void check_recording_backend_device_status( const Name, const DictionaryDatum& );
  void get_recording_backend_device_defaults( const Name, DictionaryDatum& );
  void get_recording_backend_device_status( const Name, const RecordingDevice&, DictionaryDatum& );

private:
  void set_data_path_prefix_( const DictionaryDatum& );

  std::string data_path_;   //!< Path for all files written by devices
  std::string data_prefix_; //!< Prefix for all files written by devices
  bool overwrite_files_;    //!< If true, overwrite existing data files.

  /**
   * A mapping from names to registered recording backends.
   */
  std::map< Name, RecordingBackend* > recording_backends_;

  /**
   * A mapping from names to registered stimulation backends
   */
  std::map< Name, StimulationBackend* > stimulation_backends_;
};

} // namespace nest

inline const std::string&
nest::IOManager::get_data_path() const
{
  return data_path_;
}

inline const std::string&
nest::IOManager::get_data_prefix() const
{
  return data_prefix_;
}

inline bool
nest::IOManager::overwrite_files() const
{
  return overwrite_files_;
}

#endif /* #ifndef IO_MANAGER_H */
