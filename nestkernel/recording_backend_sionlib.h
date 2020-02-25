/*
 *  recording_backend_sionlib.h
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

#ifndef RECORDING_BACKEND_SIONLIB_H
#define RECORDING_BACKEND_SIONLIB_H

// C includes:
#include <mpi.h>
#include <sion.h>

#include "recording_backend.h"

/* BeginDocumentation

.. _sionlib_backend:

Store data to an efficient binary format
########################################

The `sionlib` recording backend writes collected data persistently to
a binary container file (or to a rather small set of such files). This
is especially useful for large-scale simulations running in a
distributed way on many MPI processes/OpenMP threads. In this usage
scenario, writing to plain text files (see :ref:`ASCII backend
<ascii_backend>`) would cause a large overhead because of the huge
number of generated files and be very inefficient. For the
implementation of writing to binary container files, NEST relies on
the SIONlib library (http://www.fz-juelich.de/jsc/sionlib). Depending
on the I/O architecture of the computing cluster (supercomputer) and
the global settings of the `sionlib` recording backend (see below),
either a single container file or a set of these files is created. In
case of a single file, it is named according to the following pattern:

::

   data_path/data_prefixfilename

In case of multiple files, this name is extended for each file by a
dot followed by a consecutive number. The properties ``data_path`` and
``data_prefix`` are global kernel properties. They can for example be
set during repetitive simulation protocols to separate the data
originating from individual runs.

The life of a set of associated container files starts with the call
to ``Prepare`` and ends with the call to ``Cleanup``. Data that is
produced during successive calls to ``Run`` in between a pair of
``Prepare`` and ``Cleanup`` calls will be written to the same file
set. When creating a new recording, if the filename already exists,
the ``Prepare`` call will fail with a corresponding error message. To
instead overwrite the old file set, the kernel property
``overwrite_files`` can be set to *true* using ``SetKernelStatus``. An
alternative way for avoiding name clashes is to re-set the kernel
properties ``data_path`` or ``data_prefix``, so that another full
filename is composed.

Data format
+++++++++++

The container files contain the data in a custom binary format. The
recorder property ``label`` and all other recorder-specific metadata
is stored with each recording device. To retrieve the data after the
simulation, the `nestio reader API` in Python has to be used. SIONlib
container files are sparse files: They only use up the space on disk
which is actually filled with data.

Recorder-specific parameters
++++++++++++++++++++++++++++

.. glossary::

 label
   A recorder-specific string (default: *""*) that serves as alias
   name for the recording device, and which is stored in the metadata
   section of the container files.

Global parameters (to be set via ``SetKernelStatus``)
+++++++++++++++++++++++++++++++++++++++++++++++++++++

.. glossary::

 filename
   The filename (default: *"output.sion"*) part of the pattern
   according to which the full filename (incl. path) is generated (see
   above).

 sion_n_files
   The number of container files (default: *1*) used for storing the
   results of a single call to ``Simulate`` (or of a single
   ``Prepare``-``Run``-``Cleanup`` cycle). The default is one
   file. Using multiple files may have a performance advantage on
   large computing clusters, depending on how the (parallel) file
   system is accessed from the compute nodes.

 sion_chunksize
   In SIONlib nomenclature, a single OpenMP thread running on a single
   MPI process is called a task. For each task, a specific number of
   bytes is allocated in the container file(s) from the
   beginning. This number is set by the parameter ``sion_chunksize``
   (default: *262144*). If the number of bytes written by each task
   during the simulation is known in advance, it is advantageous to
   set the chunk size to this value. In this way, the size of the
   container files has not to be adjusted by SIONlib during the
   simulation. This yields a slight performance advantage.  Choosing a
   value for ``sion_chunksize`` which is too large does not hurt that
   much because SIONlib container files are sparse files (if supported
   by the underlying file system) which only use up the disk space
   which is actually required by the stored data.

buffer_size
   The size of task-specific buffers (default: *1024*) within the
   `sionlib` recording backend in bytes.  These buffers are used to
   temporarily store data generated by the recording devices on each
   task. As soon as a buffer is full, its contents are written to the
   respective container file. To achieve optimum performance, the size
   of these buffers should at least amount to the size of the file
   system blocks.

sion_collective
   Flag (default: *false*) to enable the collective mode of
   SIONlib. In collective mode, recorded data is buffered completely
   during ``Run`` and only written at the very end of ``Run`` to the
   container files, all tasks acting synchronously. Furthermore,
   within SIONlib so-called collectors aggregate data from a specific
   number of tasks, and actually only these collectors directly access
   the container files, in this way minimizing load on the file
   system. The number of tasks per collector is determined
   automatically by SIONlib. However, collector size can also be set
   explicitly by the user via the environment variable SION_COLLSIZE
   before the start of NEST. On large simulations which also generate
   a large amount of data, collective mode can offer a performance
   advantage.

EndDocumentation */

namespace nest
{

class RecordingBackendSIONlib : public RecordingBackend
{
public:
  const static unsigned int SIONLIB_REC_BACKEND_VERSION;
  const static unsigned int DEV_NAME_BUFFERSIZE;
  const static unsigned int DEV_LABEL_BUFFERSIZE;
  const static unsigned int VALUE_NAME_BUFFERSIZE;
  const static unsigned int NEST_VERSION_BUFFERSIZE;

  RecordingBackendSIONlib();

  ~RecordingBackendSIONlib() throw();

  void initialize() override;
  void finalize() override;

  void enroll( const RecordingDevice& device, const DictionaryDatum& params ) override;

  void disenroll( const RecordingDevice& device ) override;

  void set_value_names( const RecordingDevice& device,
    const std::vector< Name >& double_value_names,
    const std::vector< Name >& long_value_names ) override;

  void prepare() override;

  void cleanup() override;

  void write( const RecordingDevice& device,
    const Event& event,
    const std::vector< double >& double_values,
    const std::vector< long >& long_values ) override;

  void set_status( const DictionaryDatum& ) override;

  void get_status( DictionaryDatum& ) const override;

  void pre_run_hook() override;

  void post_run_hook() override;

  void post_step_hook() override;

  void check_device_status( const DictionaryDatum& ) const override;
  void get_device_defaults( DictionaryDatum& ) const override;
  void get_device_status( const RecordingDevice& device, DictionaryDatum& params_dictionary ) const override;

private:
  void open_files_();
  void close_files_();
  const std::string build_filename_() const;

  bool files_opened_;
  size_t num_enrolled_devices_;

  class SIONBuffer
  {
  private:
    char* buffer_;
    size_t ptr_;
    size_t max_size_;

  public:
    SIONBuffer();
    SIONBuffer( size_t size );
    ~SIONBuffer();

    void reserve( size_t size );
    void ensure_space( size_t size );
    void write( const char* v, size_t n );

    size_t
    get_capacity()
    {
      return max_size_;
    };

    size_t
    get_size()
    {
      return ptr_;
    };

    size_t
    get_free()
    {
      return max_size_ - ptr_;
    };

    void
    clear()
    {
      ptr_ = 0;
    };

    char*
    read()
    {
      return buffer_;
    };

    template < typename T >
    SIONBuffer& operator<<( const T data );
  };

  struct DeviceInfo
  {
    DeviceInfo()
      : n_rec( 0 )
    {
    }

    index node_id;
    unsigned int type;
    std::string name;
    std::string label;

    long origin, t_start, t_stop;

    unsigned long int n_rec;
    std::vector< std::string > double_value_names;
    std::vector< std::string > long_value_names;
  };

  struct DeviceEntry
  {
    DeviceEntry( const RecordingDevice& device )
      : device( device )
      , info()
    {
    }

    const RecordingDevice& device;
    DeviceInfo info;
  };

  struct FileEntry
  {
    int sid;
    SIONBuffer buffer;
  };

  typedef std::vector< std::map< index, DeviceEntry > > device_map;
  device_map devices_;

  typedef std::map< thread, FileEntry > file_map;
  file_map files_;

  std::string filename_;
  MPI_Comm local_comm_; // single copy of local MPI communicator
                        // for all threads using the sionlib
                        // recording backend in parallel (for broadcasting
                        // the results of MPIX..(..) in open_files_(..))

  double t_start_; // simulation start time for storing

  struct Parameters_
  {
    std::string filename_; //!< the file name extension to use, without .
    bool sion_collective_; //!< use SIONlib's collective mode.
    long sion_chunksize_;  //!< the size of SIONlib's buffer.
    int sion_n_files_;     //!< the number of SIONLIB container files used.
    long buffer_size_;     //!< the size of the internal buffer.

    Parameters_();

    void get( const RecordingBackendSIONlib&, DictionaryDatum& ) const;
    void set( const RecordingBackendSIONlib&, const DictionaryDatum& );
  };

  Parameters_ P_;
};

} // namespace

#endif // RECORDING_BACKEND_SIONLIB_H
