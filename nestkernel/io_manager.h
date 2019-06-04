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

namespace nest
{

/*
  IOManager: Handles data storage files from spike detectors and
  multimeters to file system(s)/memory/output. Distinct from logging
  for error streams.
*/
class IOManager : public ManagerInterface
{
public:
  virtual void initialize(); // called from meta-manager to construct
  virtual void finalize();   // called from meta-manger to reinit
  virtual void change_num_threads( thread );

  virtual void set_status( const DictionaryDatum& ); // set parameters
  virtual void get_status( DictionaryDatum& );       // get parameters

  IOManager(); // Construct only by meta-manager
  ~IOManager();

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

  //! Helper function to set device data path and prefix.
  void set_data_path_prefix_( const DictionaryDatum& d );

  /**
   * Indicate if existing data files should be overwritten.
   * @return true if existing data files should be overwritten by devices.
   * Default: false.
   */
  bool overwrite_files() const;

  /**
   * Clean up in all registered recording backends after a single call to run by
   * calling the backends' post_run_cleanup() functions
   */
  void post_run_cleanup();

  /**
   * Finalize all registered recording backends after a call to
   * SimulationManager::simulate() or SimulationManager::cleanup() by
   * calling the backends' finalize() functions
   */
  void prepare();
  void cleanup();

  /**
   * Force a synchronization in all registered recording backends by
   * calling the backends' synchronize() functions
   */
  void synchronize();

  template < class RBT >
  void register_recording_backend( Name );

  bool is_valid_recording_backend( Name );

  void clear_recording_backends( const RecordingDevice& );

  void write( Name,
    const RecordingDevice&,
    const Event&,
    const std::vector< double >&,
    const std::vector< long >& );

  void enroll_recorder( Name,
    const RecordingDevice&,
    const std::vector< Name >&,
    const std::vector< Name >& );

  void
  get_recording_device_status( Name, const RecordingDevice&, DictionaryDatum& );

  void set_recording_device_status( Name,
    const RecordingDevice&,
    const DictionaryDatum& );

private:
  std::string data_path_;   //!< Path for all files written by devices
  std::string data_prefix_; //!< Prefix for all files written by devices
  bool overwrite_files_;    //!< If true, overwrite existing data files.

  /**
   * A mapping from names to registered recording backends.
   */
  std::map< Name, RecordingBackend* > recording_backends_;

  /**
   * Return the recording backend stored under the given name.  This
   * function assumes and asserts that the backend actually exists and
   * expects the caller to have called is_valid_recording_backend()
   * previously.
   * @return a pointer to the recording backend.
   */
  RecordingBackend* get_recording_backend_( Name );
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

#endif /* IO_MANAGER_H */
