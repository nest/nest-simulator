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
/****************************************/
/* class poisson_generator              */
/*                  Vers. 1.0       hep */
/*                  Implementation: hep */
/****************************************/

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

/** @BeginDocumentation
@ingroup Devices
@ingroup generator

Name: poisson_generator - simulate neuron firing with Poisson processes
                          statistics.
Description:

The poisson_generator simulates a neuron that is firing with Poisson
statistics, i.e. exponentially distributed interspike intervals. It will
generate a _unique_ spike train for each of it's targets. If you do not want
this behavior and need the same spike train for all targets, you have to use a
parrot neuron inbetween the poisson generator and the targets.

Parameters:

The following parameters appear in the element's status dictionary:
\verbatim embed:rst
=======   ======== =========================================================
 rate     spikes/s Mean firing rate
 origin   ms       Time origin for device timer
 start    ms       Begin of device application with resp. to origin
 stop     ms       End of device application with resp. to origin
=======   ======== =========================================================
\endverbatim

Sends: SpikeEvent

Remarks:

A Poisson generator may, especially at high rates, emit more than one
spike during a single time step. If this happens, the generator does
not actually send out n spikes. Instead, it emits a single spike with
n-fold synaptic weight for the sake of efficiency.

The design decision to implement the Poisson generator as a device
which sends spikes to all connected nodes on every time step and then
discards the spikes that should not have happened generating random
numbers at the recipient side via an event hook is twofold.

On one hand, it leads to the saturation of the messaging network with
an enormous amount of spikes, most of which will never get delivered
and should not have been generated in the first place.

On the other hand, a proper implementation of the Poisson generator
needs to provide two basic features: (a) generated spike trains
should be IID processes w.r.t. target neurons to which the generator
is connected and (b) as long as virtual_num_proc is constant, each
neuron should receive an identical Poisson spike train in order to
guarantee reproducibility of the simulations across varying machine
numbers.

Therefore, first, as Network::get_network().send sends spikes to all the
recipients, differentiation has to happen in the hook, second, the
hook can use the RNG from the thread where the recipient neuron sits,
which explains the current design of the generator. For details,
refer to:

http://ken.brainworks.uni-freiburg.de/cgi-bin/mailman/private/nest_developer/2011-January/002977.html

SeeAlso: poisson_generator_ps, Device, parrot_neuron
*/
class poisson_generator : public DeviceNode
{

public:
  /**
   * The generator is threaded, so the RNG to use is determined
   * at run-time, depending on thread.
   */
  poisson_generator();
  poisson_generator( poisson_generator const& );

  bool
  has_proxies() const
  {
    return false;
  }

  Name
  get_element_type() const
  {
    return names::stimulator;
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

  StimulatingDevice< SpikeEvent > device_;
  Parameters_ P_;
  Variables_ V_;
};

inline port
poisson_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  device_.enforce_single_syn_type( syn_id );

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
  device_.get_status( d );
}

inline void
poisson_generator::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, this );   // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  device_.set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

} // namespace nest

#endif
