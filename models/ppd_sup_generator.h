/*
 *  ppd_sup_generator.h
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

#ifndef ppd_sup_generator_H
#define ppd_sup_generator_H

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

Simulate the superimposed spike train of a population of Poisson
processes with dead time

Description
+++++++++++

The ppd_sup_generator generator simulates the pooled spike train of a
population of neurons firing independently with Poisson process with dead
time statistics.
The rate parameter can also be sine-modulated. The generator does not
initialize to equilibrium in this case, initial transients might occur.

.. include:: ../models/stimulation_device.rst

rate
    Mean firing rate of the component processes, default: 0 spikes/s

dead_time
    Minimal time between two spikes of the component processes, default: 0 ms

n_proc
    Number of superimposed independent component processes, default: 1

frequency
    Rate modulation frequency, default: 0 Hz

relative_amplitude
    Relative rate modulation amplitude, default: 0

Set parameters from a stimulation backend
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The parameters in this stimulation device can be updated with input
coming from a stimulation backend. The data structure used for the
update holds one value for each of the parameters mentioned above.
The indexing is as follows:

 0. dead_time
 1. rate
 2. n_proc
 3. frequency
 4. relative_amplitude

References
++++++++++

.. [1]  Deger M, Helias M, Boucsein C, Rotter S (2011). Statistical properties
        of superimposed stationary spike trains. Journal of Computational
        Neuroscience. DOI: https://doi.org/10.1007/s10827-011-0362-8

See also
++++++++

gamma_sup_generator, poisson_generator_ps, spike_generator

EndUserDocs */

class ppd_sup_generator : public StimulationDevice
{

public:
  ppd_sup_generator();
  ppd_sup_generator( const ppd_sup_generator& );

  bool is_off_grid() const override;

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
    double rate_;          //!< process rate [Hz]
    double dead_time_;     //!< dead time [ms]
    unsigned long n_proc_; //!< number of component processes
    double frequency_;     //!< rate modulation frequency [Hz]
    double amplitude_;     //!< rate modulation amplitude [Hz]

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


  class Age_distribution_
  {
    binomial_distribution bino_dist_;             //!< binomial distribution
    poisson_distribution poisson_dist_;           //!< poisson distribution
    std::vector< unsigned long > occ_refractory_; //!< occupation numbers of ages below dead time
    unsigned long occ_active_;                    //!< summed occupation number of ages above dead time
    size_t activate_;                             //!< rotating pointer

  public:
    //! initialize age dist
    Age_distribution_( size_t num_age_bins, unsigned long ini_occ_ref, unsigned long ini_occ_act );

    //! update age dist and generate spikes
    unsigned long update( double hazard_rate, RngPtr rng );
  };


  struct Buffers_
  {
    /**
     * Age distribution of component Poisson processes with dead time of the
     * superposition.
     */

    std::vector< Age_distribution_ > age_distributions_;
  };

  // ------------------------------------------------------------

  struct Variables_
  {
    double hazard_step_;   //!< base hazard rate in units of time step
    double hazard_step_t_; //!< hazard rate at time t in units of time step
    double omega_;         //!< angular velocity of rate modulation [rad/ms]

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
ppd_sup_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
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
    if ( p != invalid_port_ and not is_model_prototype() )
    {
      ++P_.num_targets_; // count number of targets
    }
    return p;
  }
}

inline void
ppd_sup_generator::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  StimulationDevice::get_status( d );
}

inline void
ppd_sup_generator::set_status( const DictionaryDatum& d )
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
ppd_sup_generator::is_off_grid() const
{
  return false;
}

inline StimulationDevice::Type
ppd_sup_generator::get_type() const
{
  return StimulationDevice::Type::SPIKE_GENERATOR;
}

} // namespace

#endif // PPD_SUP_GENERATOR_H
