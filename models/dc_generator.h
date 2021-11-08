/*
 *  dc_generator.h
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


#ifndef DC_GENERATOR_H
#define DC_GENERATOR_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "stimulation_device.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Provide a direct current (DC) input

Description
+++++++++++

The dc_generator provides a constant DC input to the connected
node. The unit of the current is pA.

The dc_generator is rather inefficient, since it needs to send the
same current information on each time step. If you only need a
constant bias current into a neuron, you could instead directly set
the property *I_e*, which is available in many neuron models.

.. include:: ../models/stimulation_device.rst

amplitude
    Amplitude of current (pA)

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. amplitude

Sends
+++++

CurrentEvent

See also
++++++++

ac_generator, noise_generator, step_current_generator

EndUserDocs */

class dc_generator : public StimulationDevice
{

public:
  dc_generator();
  dc_generator( const dc_generator& );

  //! Allow multimeter to connect to local instances
  bool local_receiver() const override;

  port send_test_event( Node&, rport, synindex, bool ) override;

  using Node::handle;
  using Node::handles_test_event;

  void handle( DataLoggingRequest& ) override;

  port handles_test_event( DataLoggingRequest&, rport ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  StimulationDevice::Type get_type() const override;

  void set_data_from_stimulation_backend( std::vector< double >& input_param ) override;

private:
  void init_state_() override;
  void init_buffers_() override;
  void calibrate() override;

  void update( Time const&, const long, const long ) override;

  // ------------------------------------------------------------

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    double amp_; //!< stimulation amplitude, in pA

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& );
    Parameters_& operator=( const Parameters_& p );

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

  // ------------------------------------------------------------

  struct State_
  {
    double I_; //!< Instantaneous current value; used for recording current
               //!< Required to handle current values when device is inactive

    State_(); //!< Sets default parameter values
  };

  // ------------------------------------------------------------

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< dc_generator >;
  friend class UniversalDataLogger< dc_generator >;

  // ------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    explicit Buffers_( dc_generator& );
    Buffers_( const Buffers_&, dc_generator& );
    UniversalDataLogger< dc_generator > logger_;
  };

  // ------------------------------------------------------------

  double
  get_I_() const
  {
    return S_.I_;
  }

  // ------------------------------------------------------------

  static RecordablesMap< dc_generator > recordablesMap_;
  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
};

inline port
dc_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool )
{
  StimulationDevice::enforce_single_syn_type( syn_id );

  CurrentEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
dc_generator::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
dc_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  StimulationDevice::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
dc_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, this );   // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulationDevice::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

inline bool
dc_generator::local_receiver() const
{
  return true;
}

inline StimulationDevice::Type
dc_generator::get_type() const
{
  return StimulationDevice::Type::CURRENT_GENERATOR;
}

} // namespace

#endif /* #ifndef DC_GENERATOR_H */
