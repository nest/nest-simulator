/*
 *  ginzburg_neuron.h
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

#ifndef GINZBURG_H
#define GINZBURG_H

#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "universal_data_logger.h"
#include "recordables_map.h"
#include "exp_randomdev.h"
#include <cmath>

namespace nest{
  
  class Network;

  /* BeginDocumentation
     Name: ginzburg_neuron - Binary stochastic neuron with sigmoidal activation function.

     Description:

     The neuron model ginzburg is an implementation of a binary neuron that
     is irregularly updated as Poisson time points. At each update point the
     total synaptic input h into the neuron is summed up, passed through a
     gain function g whose output is interpreted as the probability of
     the neuron to be in the active (1) state.
     The gain function g used here is g(h) = c1*h + c2 * 0.5*(1 + tanh(c3*(h-theta)))
     (output clipped to [0,1]). This allows to obtain affin-linear (c1!=0, c2!=0,
     c3=0) or sigmoidal (c1=0, c2=1, c3!=0) shaped gain functions.
     The latter choice corresponds to the definition in [1], giving the name
     to this neuron model. The time constant tau_m is defined as the mean
     inter-update-interval that is drawn from an exponential distribution with
     this parameter. Using this neuron to reprodce simulations with asynchronous
     update [1], the time constant needs to be chosen as tau_m = dt*N,
     where dt is the simulation time step and N the number of neurons in the original
     simulation with asynchronous update. This ensures that a neuron is updated on average
     every tau_m ms. Since in the original paper [1] neurons are coupled with
     zero delay, this implementation follows this definition. It uses the update
     scheme described in [2] to maintain causality: The incoming events in
     time step t_i are taken into account at the beginning of the time step
     to calculate the gain function and to decide upon a transition.
     In order to obtain delayed coupling with delay d , the user has to specify
     the delay d+h upon connection, where h is the simulation time step.
     
     
     Remarks:
     This neuron has a special use for spike events to convey the
     binary state of the neuron to the target. The neuron model
     only sends a spike if a transition of its state occurs. If the
     state makes an up-transition it sends a spike with multiplicity 2,
     if a down transition occurs, it sends a spike with multiplicity 1.
     
     Parameters: 

     The following parameters can be set in the status dictionary.

     tau_m      double - Membrane time constant (mean inter-update-interval) in ms.
     theta      double - threshold for sigmoidal activation function mV
     c1         double - linear gain factor (probability/mV)
     c2         double - prefactor of sigmoidal gain (probability)
     c3         double - slope factor of sigmoidal gain (1/mV)

 
     References:
     [1] Iris Ginzburg, Haim Sompolinsky. Theory of correlations in stochastic neural networks (1994). PRE 50(4) p. 3171
     [2] Abigail Morrison, Markus Diesmann. Maintaining Causality in Discrete Time Neuronal Simulations.
     In: Lectures in Supercomputational Neuroscience, p. 267. Peter beim Graben, Changsong Zhou, Marco Thiel, Juergen Kurths (Eds.), Springer 2008.

     Sends: SpikeEvent

     Receives: SpikeEvent, PotentialRequest

     Author:  February 2010, Helias
     SeeAlso: pp_psc_delta
  */



  /**
   * Binary stochastic neuron with linear or sigmoidal gain function.
   */
  class ginzburg:
  public Archiving_Node
  {
    
  public:        
    
    typedef Node base;
    
    ginzburg();
    ginzburg(const ginzburg&);

    /**
     * Import sets of overloaded virtual functions.
     * We need to explicitly include sets of overloaded
     * virtual functions into the current scope.
     * According to the SUN C++ FAQ, this is the correct
     * way of doing things, although all other compilers
     * happily live without.
     */

    using Node::connect_sender;
    using Node::handle;

    port check_connection(Connection&, port);
    
    void handle(SpikeEvent &);
    void handle(DataLoggingRequest &);
    
    port connect_sender(SpikeEvent &, port);
    port connect_sender(DataLoggingRequest &, port);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);

  private:

    void init_state_(const Node& proto);
    void init_buffers_();
    void calibrate();
    
    double_t gain_(double_t h);

    void update(Time const &, const long_t, const long_t);

    // The next two classes need to be friends to access the State_ class/member
    friend class RecordablesMap<ginzburg>;
    friend class UniversalDataLogger<ginzburg>;

 
    
    // ---------------------------------------------------------------- 

    /** 
     * Independent parameters of the model. 
     */
    struct Parameters_ {
      /** mean inter-update interval in ms (acts like a membrane time constant). */
      double_t tau_m_;

      /** threshold of sigmoidal activation function */
      double_t theta_;

      /** linear gain factor of gain function */
      double_t c1_;

      /** prefactor of sigmoidal gain function */
      double_t c2_;

      /** gain factor of sigmoidal gain function */
      double_t c3_;

      Parameters_();  //!< Sets default parameter values

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary
      void set(const DictionaryDatum&);  //!< Set values from dicitonary
    };
    
    // ---------------------------------------------------------------- 

    /**
     * State variables of the model.
     */
    struct State_ {
      bool y_;   //!< output of neuron in [0,1]
      double h_;   //!< total input current to neuron
      double last_in_gid_;   //!< gid of the last spike being received
      Time t_next_; //!< time point of next update
      Time t_last_in_spike_; //!< time point of last input spike seen

      State_();    //!< Default initialization
      
      void get(DictionaryDatum&, const Parameters_&) const;
      void set(const DictionaryDatum&, const Parameters_&);
    };    

    // ---------------------------------------------------------------- 

    /**
     * Buffers of the model.
     */
    struct Buffers_ {
      Buffers_(ginzburg&);
      Buffers_(const Buffers_&, ginzburg&);

      /** buffers and sums up incoming spikes/currents */
      RingBuffer spikes_;

      //! Logger for all analog data
      UniversalDataLogger<ginzburg> logger_;
     };
    
    // ---------------------------------------------------------------- 

    /**
     * Internal variables of the model.
     */
    struct Variables_ {
      librandom::RngPtr rng_; // random number generator of my own thread
      librandom::ExpRandomDev exp_dev_;  // random deviate generator
    };

    // Access functions for UniversalDataLogger -------------------------------
    
    //! Read out the binary state of the neuron
    double_t get_output_state__() const { return S_.y_; }

    //! Read out the summed input of the neuron (= membrane potential)
    double_t get_input__() const { return S_.h_; }

    // ---------------------------------------------------------------- 

    /**
     * @defgroup iaf_psc_alpha_data
     * Instances of private data structures for the different types
     * of data pertaining to the model.
     * @note The order of definitions is important for speed.
     * @{
     */   
    Parameters_ P_;
    State_      S_;
    Variables_  V_;
    Buffers_    B_;
    /** @} */
    
    //! Mapping of recordables names to access functions
    static RecordablesMap<ginzburg> recordablesMap_;

  };

    
  inline
    port ginzburg::check_connection(Connection& c, port receptor_type)
    {
      SpikeEvent e;
      e.set_sender(*this);
      c.check_event(e);
      return c.get_target()->connect_sender(e, receptor_type);
    }

  inline
    port ginzburg::connect_sender(SpikeEvent&, port receptor_type)
    {
      if (receptor_type != 0)
	throw UnknownReceptorType(receptor_type, get_name());
      return 0;
    }
 
   inline
    port ginzburg::connect_sender(DataLoggingRequest& dlr, 
				       port receptor_type)
    {
      if (receptor_type != 0)
	throw UnknownReceptorType(receptor_type, get_name());
      return B_.logger_.connect_logging_device(dlr, recordablesMap_);
    }

inline
void ginzburg::get_status(DictionaryDatum &d) const
{
  P_.get(d);
  S_.get(d, P_);
  Archiving_Node::get_status(d);
  (*d)[names::recordables] = recordablesMap_.get_list();
}

inline
void ginzburg::set_status(const DictionaryDatum &d)
{
  Parameters_ ptmp = P_;  // temporary copy in case of errors
  ptmp.set(d);                       // throws if BadProperty
  State_      stmp = S_;  // temporary copy in case of errors
  stmp.set(d, ptmp);                 // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not 
  // write them back to (P_, S_) before we are also sure that 
  // the properties to be set in the parent class are internally 
  // consistent.
  Archiving_Node::set_status(d);

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif /* #ifndef GINZBURG_H */
