/*
 *  input_device.h
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

#ifndef INPUT_DEVICE_H
#define INPUT_DEVICE_H

// C++ includes:
#include <fstream>
#include <vector>

// Includes from libnestutil:
#include "lockptr.h"

// Includes from nestkernel:
#include "node.h"
#include "device.h"
#include "device_node.h"
#include "input_backend.h"
#include "nest_types.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dictdatum.h"
#include "dictutils.h"

namespace nest
{
/**
 * Base class for all input devices with backend.
 *
 * Input devices stimulate neurons. The stimulation can be defined
 * by a external backend at the begging of the run step. There can be only one
 * input backend selected by setting the device property
 * `imput_from` to the name of the backend (internal or mpi).
 *
 * Class InputDevice is merely a shallow interface class from
 * which concrete input devices can inherit in order to use the
 * input backend infrastructure.
 *
 * @ingroup Devices
 *
 * @author Lionel Kusch and Sandra Diaz
 */
class InputDevice : public DeviceNode, public Device
{
public:
  InputDevice();
  InputDevice( const InputDevice& );

  using Device::calibrate;
  using Node::calibrate;
  void calibrate( const std::vector< Name >&, const std::vector< Name >& );

  bool is_active( Time const& T ) const override;

  /**
   * Device type.
   */
  enum Type
  {
    STEP_CURRENT_GENERATOR,
    SPIKE_GENERATOR
  };

  virtual Type get_type() const = 0;

  const std::string& get_label() const;

  void set_status( const DictionaryDatum& ) override;
  void get_status( DictionaryDatum& ) const override;
  virtual void update_from_backend( std::vector< double > input_spikes ) = 0;


protected:
  void set_initialized_() override;

  struct Parameters_
  {
    std::string label_;  //!< A user-defined label for symbolic device names.
    bool time_in_steps_; //!< Flag indicating if time is recorded in steps or ms.
    Name input_from_;    //!< Array of input backends to use.

    Parameters_();
    Parameters_( const Parameters_& );
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  } P_;

  struct State_
  {
    size_t n_events_;

    State_();
    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  } S_;

private:
  DictionaryDatum backend_params_;
};

} // namespace

#endif // INPUT_DEVICE_H
