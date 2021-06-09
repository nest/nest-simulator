/*
 *  inhomogeneous_poisson_generator.h
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

#ifndef INHOMOGENEOUS_POISSON_GENERATOR_H
#define INHOMOGENEOUS_POISSON_GENERATOR_H

// C++ includes:
#include <vector>

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest.h"
#include "random_generators.h"
#include "ring_buffer.h"
#include "stimulation_device.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Provides Poisson spike trains at a piecewise constant rate

Description
+++++++++++

The inhomogeneous Poisson generator provides Poisson spike trains at a
piecewise constant rate to the connected node(s). The rate of the process
is changed at the specified times. The unit of the instantaneous rate
is spikes/s. By default, each target of the generator will receive
a different spike train.

.. include:: ../models/stimulation_device.rst

rate_times
    Times at which rate changes (list of ms)

rate_values
    Rate of Poisson spike train (list of spikes/s)

allow_offgrid_times
    If false, spike times will be rounded to the nearest step if they
    are less than tic/2 from the step, otherwise NEST reports an
    error.  If true, spike times are rounded to the nearest step if
    within tic/2 from the step, otherwise they are rounded up to the
    *end* of the step. Default: false

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. rate_times
 1. rate_values

Receives
++++++++

DataLoggingRequest

Sends
+++++

SpikeEvent

See also
++++++++

sinusoidal_poisson_generator, step_current_generator

EndUserDocs */

class inhomogeneous_poisson_generator : public StimulationDevice
{

public:
  inhomogeneous_poisson_generator();
  inhomogeneous_poisson_generator( const inhomogeneous_poisson_generator& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::event_hook;

  port send_test_event( Node&, rport, synindex, bool ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  StimulationDevice::Type get_type() const override;
  void set_data_from_stimulation_backend( std::vector< double >& input_param ) override;


private:
  void init_state_() override;
  void init_buffers_() override;
  void calibrate() override;

  void update( Time const&, const long, const long ) override;
  void event_hook( DSSpikeEvent& ) override;

  struct Buffers_;

  /*
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    std::vector< Time > rate_times_;
    std::vector< double > rate_values_;

    //! Allow and round up rate times not on steps;
    bool allow_offgrid_times_;

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_&, Buffers_& );

    //!< Store current values in dictionary
    void get( DictionaryDatum& ) const;
    //!< Set values from dictionary
    void set( const DictionaryDatum&, Buffers_&, Node* );
    //!< Align rate time to grid if necessary and insert it into rate_times_
    void assert_valid_rate_time_and_insert( const double t );
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    size_t idx_;  //!< index of current amplitude
    double rate_; //!< current amplitude
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    poisson_distribution poisson_dist_; //!< poisson distribution
    double h_;                          //! time resolution (ms)
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  Buffers_ B_;
  Variables_ V_;
};

inline port
inhomogeneous_poisson_generator::send_test_event( Node& target,
  rport receptor_type,
  synindex syn_id,
  bool dummy_target )
{
  StimulationDevice::enforce_single_syn_type( syn_id );

  // to ensure correct overloading resolution, we need explicit event types
  // therefore, we need to duplicate the code here
  if ( dummy_target )
  {
    DSSpikeEvent e;
    e.set_sender( *this );
    return target.handles_test_event( e, receptor_type );
  }
  else
  {
    SpikeEvent e;
    e.set_sender( *this );
    return target.handles_test_event( e, receptor_type );
  }
}


inline void
inhomogeneous_poisson_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  StimulationDevice::get_status( d );
}

inline void
inhomogeneous_poisson_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  ptmp.set( d, B_, this ); // throws if BadProperty
  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulationDevice::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

inline StimulationDevice::Type
inhomogeneous_poisson_generator::get_type() const
{
  return StimulationDevice::Type::SPIKE_GENERATOR;
}

} // namespace

#endif // INHOMOGENEOUS_POISSON_GENERATOR_H
