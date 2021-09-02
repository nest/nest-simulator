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

Simulate the superimposed spike train of a population of gamma processes

Description
+++++++++++

The gamma_sup_generator generator simulates the pooled spike train of a
population of neurons firing independently with gamma process statistics.

.. include:: ../models/stimulation_device.rst

rate
    Mean firing rate of the component processes, default: 0 spikes/s

gamma_shape
    Shape parameter of component gamma processes, default: 1

n_proc
    Number of superimposed independent component processes, default: 1

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. rate
 1. gamma_shape
 2. n_proc

References
++++++++++

.. [1] Deger, Helias, Boucsein, Rotter (2011). Statistical properties of
       superimposed stationary spike trains. Journal of Computational
       Neuroscience. DOI: https://doi.org/10.1007/s10827-011-0362-8

See also
++++++++

ppd_sup_generator, poisson_generator_ps, spike_generator

EndUserDocs */

class gamma_sup_generator : public StimulationDevice
{

public:
  gamma_sup_generator();
  gamma_sup_generator( const gamma_sup_generator& );

  bool is_off_grid() const override;

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

  /**
   * Update state.
   * Update cannot send spikes directly, since we need to identify each
   * target to know the age distribution of the component processes.
   * Since target information is in the Connectors, we send a DSSpikeEvent
   * to all targets, which is reflected to this->event_hook() with target
   * information.
   * @see event_hook, DSSpikeEvent
   */
  void update( Time const&, const long, const long ) override;

  /**
   * Send out spikes.
   * Called once per target to dispatch actual output spikes.
   * @param contains target information.
   */
  void event_hook( DSSpikeEvent& ) override;

  // ------------------------------------------------------------

  /**
   * Store independent parameters of the model.
   */
  struct Parameters_
  {
    double rate_;               //!< rate of component gamma process [Hz]
    unsigned long gamma_shape_; //!< gamma shape parameter [1]_
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
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

  // ------------------------------------------------------------

  class Internal_states_
  {
    binomial_distribution bino_dist_;   //!< binomial distribution
    poisson_distribution poisson_dist_; //!< poisson distribution
    std::vector< unsigned long > occ_;  //!< occupation numbers of internal states

  public:
    Internal_states_( size_t num_bins,
      unsigned long ini_occ_ref,
      unsigned long ini_occ_act );                              //!< initialize occupation numbers
    unsigned long update( double transition_prob, RngPtr rng ); //!< update age dist and generate spikes
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

  Parameters_ P_;
  Variables_ V_;
  Buffers_ B_;
};

inline port
gamma_sup_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
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
  StimulationDevice::get_status( d );
}

inline void
gamma_sup_generator::set_status( const DictionaryDatum& d )
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
gamma_sup_generator::is_off_grid() const
{
  return false;
}

inline StimulationDevice::Type
gamma_sup_generator::get_type() const
{
  return StimulationDevice::Type::SPIKE_GENERATOR;
}

} // namespace

#endif
