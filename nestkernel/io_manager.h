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

// Includes from sli:
#include "dictdatum.h"

namespace nest {

/*
  IOManager: Handles data storage files from spike detectors and
  multimeters to file system(s)/memory/output. Distinct from logging
  for error streams.
*/
class IOManager : public ManagerInterface {
public:
  virtual void initialize(); // called from meta-manager to construct
  virtual void finalize();   // called from meta-manger to reinit

  virtual void set_status( const DictionaryDatum& ); // set parameters
  virtual void get_status( DictionaryDatum& );       // get parameters

  IOManager(); // Construct only by meta-manager

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

private:
  std::string data_path_;   //!< Path for all files written by devices
  std::string data_prefix_; //!< Prefix for all files written by devices
  bool overwrite_files_;    //!< If true, overwrite existing data files.
};
}


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
