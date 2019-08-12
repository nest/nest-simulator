/*
 *  recording_backend_ascii.h
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

#ifndef RECORDING_BACKEND_ASCII_H
#define RECORDING_BACKEND_ASCII_H

// C++ includes:
#include <fstream>

#include "recording_backend.h"

namespace nest
{

/**
 * ASCII specialization of the RecordingBackend interface.
 *
 * RecordingBackendASCII maintains a data structure mapping one file
 * stream to every recording device instance on every thread. Files
 * are opened and inserted into the map during the enroll() call
 * (issued by the recorder's calibrate() function) and closed in
 * cleanup(), which is called on all registered recording backends by
 * IOManager::cleanup().
 */
class RecordingBackendASCII : public RecordingBackend
{
public:
  RecordingBackendASCII();

  ~RecordingBackendASCII() throw();

  void initialize() override;

  void finalize() override;

  void enroll( const RecordingDevice& device ) override;

  void disenroll( const RecordingDevice& device ) override;

  void set_value_names( const RecordingDevice& device,
    const std::vector< Name >& double_value_names, const std::vector< Name >& long_value_names ) override;

  void prepare() override;

  void cleanup() override;

  void pre_run_hook() override;

  /**
   * Flush files after a single call to Run
   */
  void post_run_hook() override;

  void write( const RecordingDevice&, const Event&,
	      const std::vector< double >&, const std::vector< long >& ) override;

  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) const override;

  void set_device_status( const RecordingDevice& device, const DictionaryDatum& d ) override;
  void get_device_status( const RecordingDevice& device, DictionaryDatum& ) const override;

private:
  /**
   * Build device file basename as being the device's label (or model
   * name if no label is given), the device's GID, and the virtual
   * process ID, all separated by dashes, followed by a dot and the
   * filename extension.
   */
  const std::string build_basename_( const RecordingDevice& device ) const;

  struct DeviceData
  {
    DeviceData() = delete;
    DeviceData( std::string );
    void set_value_names( const std::vector< Name >&, const std::vector< Name >&);
    void open_file();
    void write( const Event&, const std::vector< double >&, const std::vector< long >& );
    void flush_file();
    void close_file();
    void get_status( DictionaryDatum& ) const;
    void set_status( const DictionaryDatum& );
  private:
    long precision_;                          //!< Number of decimal places used when writing decimal values
    bool time_in_steps_;                      //!< Should time be recorded in steps (ms if false)
    std::string file_basename_;               //!< File name up to but not including the "."
    std::string file_extension_;              //!< File name extension without leading "."
    std::string filename_;                    //!< Full filename as determined and used by open_file()
    std::ofstream file_;                      //!< File stream to use for the device
    std::vector< Name > double_value_names_;  //!< names for values of type double
    std::vector< Name > long_value_names_;    //!< names for values of type long
  };

  typedef std::vector< std::map< size_t, DeviceData > > data_map;
  data_map device_data_;
};

} // namespace

#endif // RECORDING_BACKEND_ASCII_H
