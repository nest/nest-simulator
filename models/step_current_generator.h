/*
 *  step_current_generator.h
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


#ifndef STEP_CURRENT_GENERATOR_H
#define STEP_CURRENT_GENERATOR_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "stimulation_device.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "stimulation_device.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Provide a piecewise constant DC input current

Description
+++++++++++

The dc_generator provides a piecewise constant DC input to the
connected node(s).  The amplitude of the current is changed at the
specified times. The unit of the current is pA.

If *allow_offgrid_spikes* is set false, times will be rounded to the
nearest step if they are less than tic/2 from the step, otherwise NEST
reports an error. If true, times are rounded to the nearest step if
within tic/2 from the step, otherwise they are rounded up to the *end*
of the step.

Times of amplitude changes must be strictly increasing after conversion
to simulation time steps. The option allow_offgrid_times may be
useful, e.g., if you are using randomized times for current changes
which typically would not fall onto simulation time steps.

.. include:: ../models/stimulation_device.rst

amplitude_times
    Times at which current changes (list of times in ms)

amplitude_values
    Amplitudes of step current current (list of currents in pA)

allow_offgrid_times
    Boolean indicating if offgrid times should be used (default: False)

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds pairs of values in the form

 [ (amplitude_times, amplitude_values), (amplitude_times, amplitude_values), ... ].

Thus, the size of the data for the step_rate_generator needs to be even.

Sends
+++++

CurrentEvent

See also
++++++++

ac_generator, dc_generator, noise_generator

EndUserDocs */

class step_current_generator : public StimulationDevice
{

public:
  step_current_generator();
  step_current_generator( const step_current_generator& );

  //! Allow multimeter to connect to local instances
  bool local_receiver() const override;

  port send_test_event( Node&, rport, synindex, bool ) override;

  using Node::handle;
  using Node::handles_test_event;

  void handle( DataLoggingRequest& ) override;

  port handles_test_event( DataLoggingRequest&, rport ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  void set_data_from_stimulation_backend( std::vector< double >& input_spikes ) override;

  StimulationDevice::Type get_type() const override;

private:
  void init_state_() override;
  void init_buffers_() override;
  void calibrate() override;

  void update( Time const&, long, long ) override;

  struct Buffers_;

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    //! Times of amplitude changes
    std::vector< Time > amp_time_stamps_;

    //! Amplitude values activated at given times
    std::vector< double > amp_values_;

    //! Allow and round up amplitude times not on steps
    bool allow_offgrid_amp_times_;

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_&, Buffers_& );
    Parameters_( const Parameters_& );
    Parameters_& operator=( const Parameters_& p );

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    //! Set values from dictionary
    void set( const DictionaryDatum&, Buffers_&, Node* );

    /**
     * Return time as Time object if valid, otherwise throw BadProperty
     *
     * @param amplitude time, ms
     * @param previous time stamp
     */
    Time validate_time_( double, const Time& );
  };

  // ------------------------------------------------------------

  struct State_
  {
    double I_; //!< Instantaneous current value; used for recording current

    State_(); //!< Sets default parameter values
  };

  // ------------------------------------------------------------

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< step_current_generator >;
  friend class UniversalDataLogger< step_current_generator >;

  // ------------------------------------------------------------

  struct Buffers_
  {
    size_t idx_; //!< index of current amplitude
    double amp_; //!< current amplitude

    explicit Buffers_( step_current_generator& );
    Buffers_( const Buffers_&, step_current_generator& );
    UniversalDataLogger< step_current_generator > logger_;
  };

  // ------------------------------------------------------------

  double
  get_I_() const
  {
    return S_.I_;
  }

  // ------------------------------------------------------------

  static RecordablesMap< step_current_generator > recordablesMap_;
  Parameters_ P_;
  State_ S_;
  Buffers_ B_;
};

inline port
step_current_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool )
{
  StimulationDevice::enforce_single_syn_type( syn_id );

  CurrentEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
step_current_generator::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
step_current_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  StimulationDevice::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
step_current_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;   // temporary copy in case of errors
  ptmp.set( d, B_, this ); // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulationDevice::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

//! Allow multimeter to connect to local instances
inline bool
step_current_generator::local_receiver() const
{
  return true;
}

inline StimulationDevice::Type
step_current_generator::get_type() const
{
  return StimulationDevice::Type::CURRENT_GENERATOR;
}
} // namespace

#endif /* #ifndef STEP_CURRENT_GENERATOR_H */
