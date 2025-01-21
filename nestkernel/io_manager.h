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

/**
 * Manager to handle everything related to input and output.
 *
 * IOManager handles the data path and prefix variables of the NEST kernel and
 * manages the recording and stimulation backends and the routing of data from
 * and to devices to and from the backends.
 *
 * This manager is not responsible for logging and messaging to the user.
 * See LoggingManager if you are looging for that.
 */
class IOManager : public ManagerInterface
{
public:
  IOManager();
  ~IOManager() override;

  void initialize( const bool ) override;
  void finalize( const bool ) override;
  void set_status( const dictionary& ) override;
  void get_status( dictionary& ) override;

  void change_number_of_threads() override;

  void set_status( const dictionary& ) override;
  void get_status( dictionary& ) override;

  void set_recording_backend_status( std::string, const dictionary& );
  dictionary get_recording_backend_status( std::string );

  /**
   * The prefix for files written by devices.
   *
   * The prefix must not contain any part of a path.
   * @see get_data_dir(), overwrite_files()
   */
  const std::string& get_data_prefix() const;

  /**
   * The path for files written by devices.
   *
   * It may be the empty string (use current directory).
   * @see get_data_prefix(), overwrite_files()
   */
  const std::string& get_data_path() const;

  /**
   * Indicate if existing data files should be overwritten.
   *
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
  void register_recording_backend( const std::string& );
  template < class StimulationBackendT >
  void register_stimulation_backend( const std::string& );

  bool is_valid_recording_backend( const std::string& ) const;
  bool is_valid_stimulation_backend( const std::string& ) const;

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
  void write( const std::string&,
    const RecordingDevice&,
    const Event&,
    const std::vector< double >&,
    const std::vector< long >& );

  void enroll_recorder( const std::string&, const RecordingDevice&, const dictionary& );
  void enroll_stimulator( const std::string&, StimulationDevice&, const dictionary& );

  void set_recording_value_names( const std::string& backend_name,
    const RecordingDevice& device,
    const std::vector< std::string >& double_value_names,
    const std::vector< std::string >& long_value_names );

  void check_recording_backend_device_status( const std::string&, const dictionary& );
  void get_recording_backend_device_defaults( const std::string&, dictionary& );
  void get_recording_backend_device_status( const std::string&, const RecordingDevice&, dictionary& );

private:
  void set_data_path_prefix_( const dictionary& );

  std::string data_path_;   //!< Path for all files written by devices
  std::string data_prefix_; //!< Prefix for all files written by devices
  bool overwrite_files_;    //!< If true, overwrite existing data files.

  /**
   * A mapping from names to registered recording backends.
   */
  std::map< std::string, RecordingBackend* > recording_backends_;

  /**
   * A mapping from names to registered stimulation backends
   */
  std::map< std::string, StimulationBackend* > stimulation_backends_;
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
