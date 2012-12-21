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

#include <vector>
#include "nest.h"
#include "event.h"
#include "node.h"
#include "stimulating_device.h"
#include "scheduler.h"
#include "binomial_randomdev.h"
#include "poisson_randomdev.h"
#include "connection.h"

/*BeginDocumentation
Name: ppd_sup_generator - simulate the superimposed spike train of a population of Poisson processes with dead time.
Description:

  The ppd_sup_generator generator simulates the pooled spike train of a 
  population of neurons firing independently with Poisson process with dead 
  time statistics. 
  The rate parameter can also be sine-modulated. The generator does not 
  initialize to equilibrium in this case, initial transients might occur.

Parameters:
   The following parameters appear in the element's status dictionary:

   rate - mean firing rate of the component processes. (double, var)
   dead_time - minimal time between two spikes of the component processes. (double, var)
   n_proc - number of superimposed independent component processes. (long, var)
   frequency - rate modulation frequency. (double, var)
   amplitude - relative rate modulation amplitude. (double, var)

Note:
   The generator has been published in Deger, Helias, Boucsein, Rotter (2011) 
   Statistical properties of superimposed stationary spike trains, 
   Journal of Computational Neuroscience.
   URL: http://www.springerlink.com/content/u75211r381p08301/
   DOI: 10.1007/s10827-011-0362-8

Authors:
   June 2009, Moritz Deger, Moritz Helias

SeeAlso: gamma_sup_generator, poisson_generator_ps, spike_generator, Device, StimulatingDevice
*/


namespace nest{

  /** 
   * Generator of the spike output of a population of Poisson processes with dead time.
   * 
   * This Poisson process with dead time superposition generator sends different spike 
   * trains to all its targets. 
   *
   * @ingroup Devices
   */
  class ppd_sup_generator: public Node
  {
    
  public:        
    
    typedef Node base;
    
    ppd_sup_generator();
    ppd_sup_generator(const ppd_sup_generator&);
    
    bool has_proxies() const {return false;}
    bool is_off_grid() const {return false;}  // does not use off_grid events

    using Node::event_hook;

    port check_connection(Connection&, port);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);
    

  private:
    void init_state_(const Node&);
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
    void update(Time const &, const long_t, const long_t);
    
    /**
     * Send out spikes.
     * Called once per target to dispatch actual output spikes.
     * @param contains target information.
     */
    void event_hook(DSSpikeEvent&);

    // ------------------------------------------------------------

    /**
     * Store independent parameters of the model.
     */
    struct Parameters_ {
      double_t                rate_;        //!< process rate [Hz]
      double_t                dead_time_;   //!< dead time [ms]
      ulong_t                 n_proc_;      //!< number of component processes
      double_t                frequency_;   //!< rate modulation frequency [Hz]
      double_t                amplitude_;   //!< rate modulation amplitude [Hz]

      /**
       * Number of targets.
       * This is a hidden parameter; must be placed in parameters,
       * even though it is an implementation detail, since it 
       * concerns the connections and must not be affected by resets.
       */
      size_t num_targets_;

      Parameters_();  //!< Sets default parameter values

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary
      void set(const DictionaryDatum&);  //!< Set values from dicitonary
    };

    // ------------------------------------------------------------


    class Age_distribution_ {
    
      librandom::BinomialRandomDev bino_dev_;       //!< random deviate generator
      librandom::PoissonRandomDev poisson_dev_;     //!< random deviate generator
      std::vector<ulong_t> occ_refractory_;         //!< occupation numbers of ages below dead time
      ulong_t  occ_active_;                         //!< summed occupation number of ages above dead time
      size_t activate_;                             //!< rotating pointer
      
      public:
      Age_distribution_(size_t num_age_bins, ulong_t ini_occ_ref, ulong_t ini_occ_act);  //!< initialize age dist
      ulong_t update(double_t hazard_rate, librandom::RngPtr rng);    //!< update age dist and generate spikes
    
    };
    
    

    struct Buffers_ {
      /**
       * Age distribution of component Poisson processes with dead time of the superposition.
       */

      std::vector<Age_distribution_> age_distributions_;
      
    };

    // ------------------------------------------------------------

    struct Variables_ {
      double_t                hazard_step_;    //!< base hazard rate in units of time step
      double_t                hazard_step_t_;  //!< hazard rate at time t in units of time step
      double_t                omega_;          //!< angular velocity of rate modulation [rad/ms]

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
      double_t t_min_active_;  //!< start of generator activity in slice 
      double_t t_max_active_;  //!< end of generator activity in slice 
      //@}
    };

    // ------------------------------------------------------------

    StimulatingDevice<CurrentEvent> device_;
    Parameters_ P_;
    Variables_  V_;
    Buffers_    B_;
  };

inline  
port ppd_sup_generator::check_connection(Connection& c, port receptor_type)
{
  DSSpikeEvent e;
  e.set_sender(*this);
  c.check_event(e);
  port receptor = c.get_target()->connect_sender(e, receptor_type);
  ++P_.num_targets_;     // count number of targets
  return receptor;
}

inline
void ppd_sup_generator::get_status(DictionaryDatum &d) const
{
  P_.get(d);
  device_.get_status(d);
}

inline
void ppd_sup_generator::set_status(const DictionaryDatum &d)
{
  Parameters_ ptmp = P_;  // temporary copy in case of errors
  ptmp.set(d);               // throws if BadProperty

  // We now know that ptmp is consistent. We do not write it back
  // to P_ before we are also sure that the properties to be set
  // in the parent class are internally consistent.
  device_.set_status(d);

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
}

} // namespace

#endif //PPD_SUP_GENERATOR_H
