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

// Includes from nestkernel:
#include "connection.h"
#include "device_node.h"
#include "event.h"
#include "nest_types.h"
#include "random_generators.h"
#include "stimulation_device.h"

namespace nest
{

/* BeginUserDocs: device, generator

Short description
+++++++++++++++++

Create spike trains as described by the MIP model

Description
+++++++++++

The mip_generator generates correlated spike trains using an Multiple
Interaction Process (MIP) as described in [1]_. Underlying principle is a
Poisson parent process with rate r, the spikes of which are copied into the
child processes with a certain probability p. Every node the mip_generator is
connected to receives a distinct child process as input, whose rate is p*r.
The value of the pairwise correlation coefficient of two child processes
created by a MIP process equals p.

The MIP generator may emit more than one spike through a child process
during a single time step, especially at high rates.  If this happens,
the generator does not actually send out n spikes.  Instead, it emits
a single spike with n-fold synaptic weight for the sake of efficiency.
Furthermore, note that as with the Poisson generator, different threads
have their own copy of a MIP generator. By using the same mother_seed
it is ensured that the mother process is identical for each of the
generators.

.. include:: ../models/stimulation_device.rst

rate
    Mean firing rate of the parent process, spikes/s

p_copy
    Copy probability

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. rate
 1. p_copy

Sends
+++++

SpikeEvent

References
++++++++++

.. [1] Kuhn A, Aertsen A, Rotter S (2003). Higher-order statistics of input
       ensembles and the response of simple model neurons. Neural Computation
       15:67-101.
       DOI: https://doi.org/10.1162/089976603321043702

See also
++++++++

poisson_generator

EndUserDocs */

/*! Class mip_generator generates spike trains as described
    in the MIP model.
*/
class mip_generator : public StimulationDevice
{

public:
  mip_generator();
  mip_generator( const mip_generator& rhs );

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

  /**
   * @todo Should use binomial distribution
   */
  void event_hook( DSSpikeEvent& ) override;

  // ------------------------------------------------------------

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    double rate_;   //!< process rate in Hz
    double p_copy_; //!< copy probability for each spike in the parent process

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    poisson_distribution poisson_dist_; //!< poisson_distribution
  };

  // ------------------------------------------------------------

  Parameters_ P_;
  Variables_ V_;
};

inline port
mip_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  StimulationDevice::enforce_single_syn_type( syn_id );

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
  StimulationDevice::get_status( d );
}

inline void
mip_generator::set_status( const DictionaryDatum& d )
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

inline StimulationDevice::Type
mip_generator::get_type() const
{
  return StimulationDevice::Type::SPIKE_GENERATOR;
}

} // namespace nest

#endif
