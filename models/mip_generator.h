/*
 *  mip_generator.h
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

#ifndef MIP_GENERATOR_H
#define MIP_GENERATOR_H

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

create spike trains as described by the MIP model

Description
+++++++++++

The mip_generator generates correlated spike trains using an Multiple
Interaction Process (MIP) as described in [1]_. Underlying principle is a
Poisson mother process with rate r, the spikes of which are copied into the
child processes with a certain probability p. Every node the mip_generator is
connected to receives a distinct child process as input, whose rate is p*r.
The value of the pairwise correlation coefficient of two child processes
created by a MIP process equals p.

Remarks:

The MIP generator may emit more than one spike through a child process
during a single time step, especially at high rates.  If this happens,
the generator does not actually send out n spikes.  Instead, it emits
a single spike with n-fold synaptic weight for the sake of efficiency.
Furthermore, note that as with the Poisson generator, different threads
have their own copy of a MIP generator. By using the same mother_seed
it is ensured that the mother process is identical for each of the
generators.

IMPORTANT: The mother_seed of mpi_generator must be different from any
           seeds used for the global or thread-specific RNGs set in
           the kernel.

TODO: Better handling of private random number generator, see #143.
      Most important: If RNG is changed in prototype by SetDefaults,
      then this is

Parameters
++++++++++

The following parameters appear in the element's status dictionary:

============  ======== ================================================
 rate         spikes/s Mean firing rate of the mother process
 p_copy       real     Copy probability
 mother_rng   rng      Random number generator of mother process
 mother_seed  integer  Seed of RNG of mother process
============  ======== ================================================

Sends
+++++

SpikeEvent

References
++++++++++

.. [1] Kuhn A, Aertsen A, Rotter S (2003). Higher-order statistics of input
       ensembles and the response of simple model neurons. Neural Computation
       15:67-101.
       DOI: https://doi.org/10.1162/089976603321043702

EndUserDocs */

/*! Class mip_generator generates spike trains as described
    in the MIP model.
*/
class mip_generator : public DeviceNode
{

public:
  /**
   * The generator is threaded, so the RNG to use is determined
   * at run-time, depending on thread. An additional RNG is used
   * for the mother process.
   */
  mip_generator();

  /**
   * Copy constructor. Called, when a new instance is created.
   * Needs to be overrwritten to initialize the random generator
   * for the mother process.
   */
  mip_generator( const mip_generator& rhs );

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

  /**
   * @todo Should use binomial distribution
   */
  void event_hook( DSSpikeEvent& );

  // ------------------------------------------------------------

  /**
   * Store independent parameters of the model.
   * Mother RNG is a parameter since it can be changed. Not entirely in
   * keeping with persistence rules, since it changes state during
   * updates. Should go once we have proper global RNG scheme.
   */
  struct Parameters_
  {
    double rate_;               //!< process rate in Hz
    double p_copy_;             //!< copy probability for each spike in the mother process
    unsigned long mother_seed_; //!< seed of the mother process
    librandom::RngPtr rng_;     //!< random number generator for mother process

    Parameters_(); //!< Sets default parameter values
    Parameters_( const Parameters_& );

    Parameters_& operator=( const Parameters_& );

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    librandom::PoissonRandomDev poisson_dev_; //!< random deviate generator
  };

  // ------------------------------------------------------------

  StimulatingDevice< SpikeEvent > device_;
  Parameters_ P_;
  Variables_ V_;
};

inline port
mip_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
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
mip_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  device_.get_status( d );
}

inline void
mip_generator::set_status( const DictionaryDatum& d )
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
