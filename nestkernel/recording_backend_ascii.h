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

/* BeginUserDocs: NOINDEX

Recording backend `ascii` - Write data to plain text files
##########################################################

Description
+++++++++++

The `ascii` recording backend writes collected data persistently to a
plain text ASCII file. It can be used for small to medium sized
simulations, where the ease of a simple data format outweighs the
benefits of high-performance output operations.

This backend will open one file per recording device per thread on
each MPI process. This can cause a high load on the file system in
large simulations. This backend can become prohibitively inefficient,
particularly on machines with distributed filesystems. In case you
experience such scaling problems, the :doc:`recording backend for
SIONlib <recording_backend_sionlib>` may be a possible alternative.

Filenames of data files are determined according to the following
pattern:

::

   data_path/data_prefix(label|model_name)-node_id-vp.file_extension

The properties ``data_path`` and ``data_prefix`` are global kernel
properties. They can, for example, be set during repetitive simulation
protocols to separate the data originating from individual runs. The
``label`` replaces the model name component if it is set to a non-empty
string. ``node_id`` and ``vp`` denote the zero-padded global ID and virtual
process of the recorder writing the file. The filename ends in a dot
and the ``file_extension``.

The life of a file starts with the call to ``Prepare`` and ends with
the call to ``Cleanup``. Data that is produced during successive calls
to ``Run`` in between a pair of ``Prepare`` and ``Cleanup`` calls will
be written to the same file, while the call to ``Run`` will flush all
data to the file, so it is available for immediate inspection.

When creating a new recording, if the file name already
exists, the ``Prepare`` call will fail with a corresponding error
message. To instead overwrite the old file, the kernel property
``overwrite_files`` can be set to *true* using ``SetKernelStatus``. An
alternative way for avoiding name clashes is to re-set the kernel
properties ``data_path`` or ``data_prefix``, so that another filename is
chosen.

Data format
+++++++++++

Any file written by the `ascii` recording backend starts with an
informational header. The first header line contains the NEST version,
with which the file was created, followed by the version of the
recording backend in the second. The third line describes the data by
means of the field names for the different columns. All lines of the
header start with a `#` character.

The first field of each record written is the node ID of the neuron
the event originated from, i.e., the *source* of the event. This is
followed by the time of the measurement, the recorded floating point
values and the recorded integer values.

The format of the time field depends on the value of the property
``time_in_steps``. If set to *false* (which is the default), time is
written as a single floating point number representing the simulation
time in ms. If ``time_in_steps`` is *true*, the time of the event is
written as a pair of values consisting of the integer simulation time
step in units of the simulation resolution and the negative floating
point offset in ms from the next integer grid point.

.. note::

   The number of decimal places for all decimal numbers written can be
   controlled using the recorder property ``precision``.

Parameter summary
+++++++++++++++++

file_extension
    A string (default: *"dat"*) that specifies the file name extension,
    without leading dot. The generic default was chosen, because the
    exact type of data cannot be known a priori.

filenames
    A list of the filenames where data is recorded to. This list has one
    entry per local thread and is a read-only property.

label
    A string (default: *""*) that replaces the model name component in
    the filename if it is set.

precision
    An integer (default: *3*) that controls the number of decimal places
    used to write decimal numbers to the output file.

time_in_steps
    A Boolean (default: *false*) specifying whether to write time in
    steps, i.e., in integer multiples of the simulation resolution plus
    a floating point number for the negative offset from the next grid
    point in ms, or just the simulation time in ms. This property
    cannot be set after Simulate has been called.

EndUserDocs */

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
  const static unsigned int ASCII_REC_BACKEND_VERSION;

  RecordingBackendASCII();

  ~RecordingBackendASCII() throw();

  void initialize() override;

  void finalize() override;

  void enroll( const RecordingDevice& device, const DictionaryDatum& params ) override;

  void disenroll( const RecordingDevice& device ) override;

  void set_value_names( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names ) override;

  void prepare() override;

  void cleanup() override;

  void pre_run_hook() override;

  /**
   * Flush files after a single call to Run
   */
  void post_run_hook() override;

  void post_step_hook() override;

  void write( const RecordingDevice&, const Event&, const std::vector< double >&, const std::vector< long >& ) override;

  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) const override;

  void check_device_status( const DictionaryDatum& ) const override;
  void get_device_defaults( DictionaryDatum& ) const override;
  void get_device_status( const RecordingDevice& device, DictionaryDatum& ) const override;

private:
  const std::string compute_vp_node_id_string_( const RecordingDevice& device ) const;

  struct DeviceData
  {
    DeviceData() = delete;
    DeviceData( std::string, std::string );
    void set_value_names( const std::vector< Name >&, const std::vector< Name >& );
    void open_file();
    void write( const Event&, const std::vector< double >&, const std::vector< long >& );
    void flush_file();
    void close_file();
    void get_status( DictionaryDatum& ) const;
    void set_status( const DictionaryDatum& );

  private:
    long precision_;                         //!< Number of decimal places used when writing decimal values
    bool time_in_steps_;                     //!< Should time be recorded in steps (ms if false)
    std::string modelname_;                  //!< File name up to but not including the "."
    std::string vp_node_id_string_;          //!< The vp and node ID component of the filename
    std::string file_extension_;             //!< File name extension without leading "."
    std::string label_;                      //!< The label of the device.
    std::ofstream file_;                     //!< File stream to use for the device
    std::vector< Name > double_value_names_; //!< names for values of type double
    std::vector< Name > long_value_names_;   //!< names for values of type long

    std::string compute_filename_() const; //!< Compose and return the filename
  };

  typedef std::vector< std::map< size_t, DeviceData > > data_map;
  data_map device_data_;
};

} // namespace

#endif // RECORDING_BACKEND_ASCII_H
