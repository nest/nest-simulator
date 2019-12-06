/*
 *  gamma_sup_generator.h
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

#ifndef GAMMA_SUP_GENERATOR_H
#define GAMMA_SUP_GENERATOR_H

// C++ includes:
#include <vector>

// Includes from librandom:
#include "binomial_randomdev.h"
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

Name: gamma_sup_generator - simulate the superimposed spike train of a
                            population of gamma process.
Description:

The gamma_sup_generator generator simulates the pooled spike train of a
population of neurons firing independently with gamma process statistics.

Parameters:

The following parameters appear in the element's status dictionary:
\verbatim embed:rst
============  ======== =========================================================
 rate         spikes/s Mean firing rate of the component processes,
                       default: 0 spikes/s
 gamma_shape  integer  Shape paramter of component gamma processes, default: 1
 n_proc       integer  Number of superimposed independent component processes,
                       default: 1
============  ======== =========================================================
\endverbatim

References:

\verbatim embed:rst
.. [1] Deger, Helias, Boucsein, Rotter (2011). Statistical properties of
       superimposed stationary spike trains. Journal of Computational
       Neuroscience. DOI: https://doi.org/10.1007/s10827-011-0362-8
\endverbatim

Author:
   Jan 2011, Moritz Deger

SeeAlso: ppd_sup_generator, poisson_generator_ps, spike_generator, Device,
StimulatingDevice
*/
class gamma_sup_generator : public DeviceNode
{

public:
  gamma_sup_generator();
  gamma_sup_generator( const gamma_sup_generator& );

  bool
  has_proxies() const
  {
    return false;
  }

  bool
  is_off_grid() const
  {
    return false;
  }

  Name
  get_element_type() const
  {
    return names::stimulator;
  }

  using Node::event_hook;

  port send_test_event( Node&, rport, synindex, bool );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& );
  void init_buffers_();
  void calibrate();

  /**
   * Update state.
   * Update cannot send spikes directly, since we need to identify each
   * target to know the age distribution of the component processes.
   * Since target information is in the Connectors, we send a DSSpikeEvent
   * to all targets, which is reflected to this->event_hook() with target
   * information.
   * @see event_hook, DSSpikeEvent
   */
  void update( Time const&, const long, const long );

  /**
   * Send out spikes.
   * Called once per target to dispatch actual output spikes.
   * @param contains target information.
   */
  void event_hook( DSSpikeEvent& );

  // ------------------------------------------------------------

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    double rate_;               //!< rate of component gamma process [Hz]
    unsigned long gamma_shape_; //!< gamma shape parameter [1]
    unsigned long n_proc_;      //!< number of component processes

    /**
     * Number of targets.
     * This is a hidden parameter; must be placed in parameters,
     * even though it is an implementation detail, since it
     * concerns the connections and must not be affected by resets.
     */
    size_t num_targets_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary
  };

  // ------------------------------------------------------------

  class Internal_states_
  {

    librandom::BinomialRandomDev bino_dev_;   //!< random deviate generator
    librandom::PoissonRandomDev poisson_dev_; //!< random deviate generator
    std::vector< unsigned long > occ_;        //!< occupation numbers of internal states

  public:
    Internal_states_( size_t num_bins,
      unsigned long ini_occ_ref,
      unsigned long ini_occ_act );                                         //!< initialize occupation numbers
    unsigned long update( double transition_prob, librandom::RngPtr rng ); //!< update age dist and generate spikes
  };


  struct Buffers_
  {
    /**
     * Occupation numbers of the internal states of the generator
     */

    std::vector< Internal_states_ > internal_states_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    double transition_prob_; //!< transition probabililty to go to next
                             //!< internal state

    /**
     * @name update-hook communication.
     * The following variables are used for direct communication from
     * update() to event_hook(). They rely on the fact that event_hook()
     * is called instantaneuously from update().
     * Spikes are sent at times t that fulfill
     *
     *   t_min_active_ < t <= t_max_active_
     */
    //@{
    double t_min_active_; //!< start of generator activity in slice
    double t_max_active_; //!< end of generator activity in slice
    //@}
  };

  // ------------------------------------------------------------

  StimulatingDevice< CurrentEvent > device_;
  Parameters_ P_;
  Variables_ V_;
  Buffers_ B_;
};

inline port
gamma_sup_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
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
    const port p = target.handles_test_event( e, receptor_type );
    if ( p != invalid_port_ )
    {
      // count number of targets
      ++P_.num_targets_;
    }
    return p;
  }
}

inline void
gamma_sup_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  device_.get_status( d );
}

inline void
gamma_sup_generator::set_status( const DictionaryDatum& d )
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

} // namespace

#endif
