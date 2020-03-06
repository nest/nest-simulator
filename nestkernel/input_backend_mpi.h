/*
 *  recording_backend_screen.h
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

#ifndef INPUT_BACKEND_MPI_H
#define INPUT_BACKEND_MPI_H

#include "input_backend.h"
#include "nest_types.h"
#include "nest_time.h"
#include <set>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mpi.h>


namespace nest
{

/**
 * A simple input backend MPI implementation
 */
class InputBackendMPI : public InputBackend {
public:
    /**
     * InputBackend constructor
     * The actual initialization is happening in RecordingBackend::initialize()
     */
    InputBackendMPI() = default;

    /**
     * InputBackend destructor
     * The actual finalization is happening in RecordingBackend::finalize()
     */
    ~InputBackendMPI() noexcept = default;

    void initialize() override;

    void finalize() override;

    void enroll(InputDevice &device, const DictionaryDatum &params) override;

    void disenroll(InputDevice &device) override;

    void cleanup() override;

    void prepare() override;

    void set_status(const DictionaryDatum &) override;

    void get_status(DictionaryDatum &) const override;

    void pre_run_hook() override;

    void post_run_hook() override;

    void post_step_hook() override;

    void check_device_status(const DictionaryDatum &) const override;

    void set_value_names(const InputDevice &device,
                         const std::vector<Name> &double_value_names,
                         const std::vector<Name> &long_value_names) override;

    void get_device_defaults(DictionaryDatum &) const override;

    void get_device_status(const InputDevice &device, DictionaryDatum &params_dictionary) const override;

private:

  /**
   * A map for the enrolled devices. We have a vector with one map per local
   * thread. The map associates the gid of a device on a given thread
   * with its MPI connection and device.
  */
  typedef std::vector< std::map< index, std::pair<const MPI_Comm*, InputDevice* > > > device_map;
  device_map devices_;
  /**
   * A map of MPI communicator by thread.
   * This map contains also the number of the device by MPI communicator.
   */
  typedef std::vector< std::map< std::string, std::pair< MPI_Comm*, int> > > comm_map;
  comm_map commMap_;

  static void get_port(InputDevice *device, std::string* port_name);
  static void get_port(const index index_node, const std::string& label,std::string* port_name);
  static void receive_spike_train(const MPI_Comm& comm, InputDevice& device);
};

} // namespace

#endif // INPUT_BACKEND_MPI_H
