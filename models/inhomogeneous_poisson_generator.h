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

#include "poisson_randomdev.h"

#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_types.h"
#include "stimulating_device.h"

#include <vector>
#include "nest.h"
#include "ring_buffer.h"


namespace nest
{

/** @BeginDocumentation
Name: inhomogeneous_poisson_generator - provides Poisson spike trains
    at a piecewise constant rate

Description:
The inhomogeneous Poisson generator provides Poisson spike trains at a
piecewise constant rate to the connected node(s). The rate of the process
is changed at the specified times. The unit of the instantaneous rate
is spikes/s. By default, each target of the generator will receive
a different spike train.

Parameters:
The following parameters can be set in the status dictionary:
rate_times   list of doubles - Times at which rate changes in ms
rate_values  list of doubles - Rate of Poisson spike train in spikes/s
allow_offgrid_times     bool - If false, spike times will be rounded to the
                              nearest step if they are less than tic/2 from
                              the step, otherwise NEST reports an error.
                              If true, spike times are rounded to the
                              nearest step if within tic/2 from the step,
                              otherwise they are rounded up to the *end*
                              of the step. Default: false

Examples:
The rate can be altered in the following way:
/inhomogeneous_poisson_generator Create /sc Set
sc << /rate_times [0.2 0.5] /rate_values [2.0 4.0] >> SetStatus

The average firing rate of each realization of the Poisson process will be
0.0 in the time interval [0, 0.2), 2.0 in the interval [0.2, 0.5)
and 4.0 from then on.

Receives: DataLoggingRequest

Sends: SpikeEvent

Authors: Renato Duarte, Barna Zajzon

SeeAlso: sinusoidal_poisson_generator, step_current_generator, Device,
       StimulatingDevice
*/
class inhomogeneous_poisson_generator : public DeviceNode
{

public:
  inhomogeneous_poisson_generator();
  inhomogeneous_poisson_generator( const inhomogeneous_poisson_generator& );

  bool
  has_proxies() const
  {
    return false;
  }

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::event_hook;

  port send_test_event( Node&, rport, synindex, bool );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );


private:
  void init_state_( const Node& );
  void init_buffers_();
  void calibrate();

  void update( Time const&, const long, const long );
  void event_hook( DSSpikeEvent& );

  struct Buffers_;

  /*
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    std::vector< Time > rate_times_;
    std::vector< double_t > rate_values_;

    //! Allow and round up rate times not on steps;
    bool allow_offgrid_times_;

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_&, Buffers_& );

    //!< Store current values in dictionary
    void get( DictionaryDatum& ) const;
    //!< Set values from dictionary
    void set( const DictionaryDatum&, Buffers_& );
    //!< Align rate time to grid if necessary and insert it into rate_times_
    void assert_valid_rate_time_and_insert( const double_t t );
  };

  // ------------------------------------------------------------

  struct Buffers_
  {
    size_t idx_;    //!< index of current amplitude
    double_t rate_; //!< current amplitude
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    librandom::PoissonRandomDev poisson_dev_; //!< random deviate generator
    double_t h_;                              //! time resolution (ms)
  };

  // ------------------------------------------------------------

  StimulatingDevice< SpikeEvent > device_;

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
  device_.enforce_single_syn_type( syn_id );

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
  device_.get_status( d );
}

inline void
inhomogeneous_poisson_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  ptmp.set( d, B_ ); // throws if BadProperty
  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  device_.set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

} // namespace

#endif // INHOMOGENEOUS_POISSON_GENERATOR_H
