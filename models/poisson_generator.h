/*
 *  poisson_generator.h
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

#ifndef POISSON_GENERATOR_H
#define POISSON_GENERATOR_H

// Includes from librandom:
#include "poisson_randomdev.h"

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_types.h"
#include "stimulating_device.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Simulated neuron firing with Poisson process statistics

Description
+++++++++++

The poisson_generator simulates a neuron that is firing with Poisson
statistics, i.e. exponentially distributed interspike intervals. It will
generate a _unique_ spike train for each of it's targets. If you do not want
this behavior and need the same spike train for all targets, you have to use a
parrot neuron between the poisson generator and the targets.

Parameters
++++++++++

The following parameters appear in the element's status dictionary:

=======   ======== =========================================================
 rate     spikes/s Mean firing rate
 origin   ms       Time origin for device timer
 start    ms       Begin of device application with resp. to origin
 stop     ms       End of device application with resp. to origin
=======   ======== =========================================================

Sends
+++++

SpikeEvent

See also
++++++++

poisson_generator_ps, Device, parrot_neuron

EndUserDocs */

class poisson_generator : public StimulatingDevice
{

public:
  /**
   * The generator is threaded, so the RNG to use is determined
   * at run-time, depending on thread.
   */
  poisson_generator();
  poisson_generator( poisson_generator const& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::event_hook;

  port send_test_event( Node&, rport, synindex, bool ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

  StimulatingDevice::Type get_type() const override;
  void set_data_from_stimulating_backend( std::vector< double > input_param ) override;

private:
  void init_state_( const Node& ) override;
  void init_buffers_() override;
  void calibrate() override;

  void update( Time const&, const long, const long ) override;
  void event_hook( DSSpikeEvent& ) override;

  // ------------------------------------------------------------

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    double rate_; //!< process rate in Hz

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    librandom::PoissonRandomDev poisson_dev_; //!< Random deviate generator
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  Variables_ V_;
};

inline port
poisson_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  StimulatingDevice::enforce_single_syn_type( syn_id );

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
poisson_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  StimulatingDevice::get_status( d );
}

inline void
poisson_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, this );   // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  StimulatingDevice::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

inline StimulatingDevice::Type
poisson_generator::get_type() const
{
  return StimulatingDevice::Type::SPIKE_GENERATOR;
}

} // namespace nest

#endif
