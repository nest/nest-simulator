/*
 *  iaf_psc_exp.h
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

#ifndef IAF_PSC_EXP_H
#define IAF_PSC_EXP_H

#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "universal_data_logger.h"
#include "recordables_map.h"

namespace nest
{
  class Network;

  /* BeginDocumentation
     Name: iaf_psc_exp - Leaky integrate-and-fire neuron model with exponential PSCs.

     Description:
     iaf_psc_expp is an implementation of a leaky integrate-and-fire model
     with exponential shaped postsynaptic currents (PSCs) according to [1].
     Thus, postsynaptic currents have an infinitely short rise time. 

     The threshold crossing is followed by an absolute refractory period (t_ref)
     during which the membrane potential is clamped to the resting potential
     and spiking is prohibited.
     
     The linear subthresold dynamics is integrated by the Exact
     Integration scheme [2]. The neuron dynamics is solved on the time
     grid given by the computation step size. Incoming as well as emitted
     spikes are forced to that grid.  

     An additional state variable and the corresponding differential
     equation represents a piecewise constant external current.

     The general framework for the consistent formulation of systems with
     neuron like dynamics interacting by point events is described in
     [2]. A flow chart can be found in [3].

     Remarks:
     The present implementation uses individual variables for the
     components of the state vector and the non-zero matrix elements of
     the propagator.  Because the propagator is a lower triangular matrix
     no full matrix multiplication needs to be carried out and the
     computation can be done "in place" i.e. no temporary state vector
     object is required.

     The template support of recent C++ compilers enables a more succinct
     formulation without loss of runtime performance already at minimal
     optimization levels. A future version of iaf_psc_exp will probably
     address the problem of efficient usage of appropriate vector and
     matrix objects.

     Parameters: 
     The following parameters can be set in the status dictionary.

     E_L          double - Resting membrane potential in mV. 
     C_m          double - Capacity of the membrane in pF
     tau_m        double - Membrane time constant in ms.
     tau_syn_ex   double - Time constant of postsynaptic excitatory currents in ms
     tau_syn_in   double - Time constant of postsynaptic inhibitory currents in ms
     t_ref        double - Duration of refractory period (V_m = V_reset) in ms.
     V_m          double - Membrane potential in mV
     V_th         double - Spike threshold in mV.
     V_reset      double - Reset membrane potential after a spike in mV.
     I_e          double - Constant input current in pA.
     t_spike      double - Point in time of last spike in ms.
 
     Note:
     tau_m != tau_syn_{ex,in} is required by the current implementation to avoid a
     degenerate case of the ODE describing the model [1]. For very similar values,
     numerics will be unstable.

     References:
     [1] Misha Tsodyks, Asher Uziel, and Henry Markram (2000) Synchrony Generation in Recurrent
     Networks with Frequency-Dependent Synapses, The Journal of Neuroscience, 2000, Vol. 20 RC50 p. 1-5
     [2] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
     systems with applications to neuronal modeling. Biologial Cybernetics
     81:381-402.
     [3] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001) State space 
     analysis of synchronous spiking in cortical neural networks. 
     Neurocomputing 38-40:565-571.

     Sends: SpikeEvent

     Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

     SeeAlso: iaf_psc_exp_ps

     FirstVersion: March 2006
     Author: Moritz Helias
  */

  /**
   * Leaky integrate-and-fire neuron with exponential PSCs.
   */
  class iaf_psc_exp: 
  public Archiving_Node 
  {
    
  public:        
    
    typedef Node base;
    
    iaf_psc_exp();
    iaf_psc_exp(const iaf_psc_exp&);

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
    void handle(CurrentEvent &);
    void handle(DataLoggingRequest &);
    
    port connect_sender(SpikeEvent &, port);
    port connect_sender(CurrentEvent &, port);
    port connect_sender(DataLoggingRequest &, port);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &);

  private:

    void init_state_(const Node& proto);
    void init_buffers_();
    void calibrate();

    void update(const Time &, const long_t, const long_t);

    // The next two classes need to be friends to access the State_ class/member
    friend class RecordablesMap<iaf_psc_exp>;
    friend class UniversalDataLogger<iaf_psc_exp>;

    // ---------------------------------------------------------------- 

    /** 
     * Independent parameters of the model. 
     */
    struct Parameters_
    {
  
      /** Membrane time constant in ms. */
      double_t Tau_; 

      /** Membrane capacitance in pF. */
      double_t C_;
    
      /** Refractory period in ms. */
      double_t t_ref_;

      /** Resting potential in mV. */
      double_t U0_;

      /** External current in pA */
      double_t I_e_;

      /** Threshold, RELATIVE TO RESTING POTENTAIL(!).
          I.e. the real threshold is (U0_+Theta_). */
      double_t Theta_;

      /** reset value of the membrane potential */
      double_t V_reset_;

      /** Time constant of excitatory synaptic current in ms. */
      double_t tau_ex_;

      /** Time constant of inhibitory synaptic current in ms. */
      double_t tau_in_;
      
      Parameters_();  //!< Sets default parameter values

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary

      /** Set values from dictionary.
       * @returns Change in reversal potential E_L, to be passed to State_::set()
       */
      double set(const DictionaryDatum&);
    };
    
    // ---------------------------------------------------------------- 

    /**
     * State variables of the model.
     */
    struct State_
    {
      // state variables
      double_t i_0_;       // synaptic dc input current, variable 0
      double_t i_syn_ex_;  // postsynaptic current for exc. inputs, variable 1
      double_t i_syn_in_;  // postsynaptic current for inh. inputs, variable 1
      double_t V_m_;       // membrane potential, variable 2

      int_t r_ref_;  // absolute refractory counter (no membrane potential propagation)
      
      State_();  //!< Default initialization
      
      void get(DictionaryDatum&, const Parameters_ &) const;

      /** Set values from dictionary.
       * @param dictionary to take data from
       * @param current parameters
       * @param Change in reversal potential E_L specified by this dict
       */
      void set(const DictionaryDatum&, const Parameters_ &, const double);
    };    

    // ---------------------------------------------------------------- 

    /**
     * Buffers of the model.
     */
    struct Buffers_
    {
      Buffers_(iaf_psc_exp &);
      Buffers_(const Buffers_ &, iaf_psc_exp &);

      /** buffers and sums up incoming spikes/currents */
      RingBuffer spikes_ex_;
      RingBuffer spikes_in_;
      RingBuffer currents_;  

      //! Logger for all analog data
      UniversalDataLogger<iaf_psc_exp> logger_;
    };
    
    // ---------------------------------------------------------------- 

    /**
     * Internal variables of the model.
     */
    struct Variables_
    { 
      /** Amplitude of the synaptic current.
	  This value is chosen such that a post-synaptic potential with
	  weight one has an amplitude of 1 mV.
	  @note mog - I assume this, not checked. 
      */
      //    double_t PSCInitialValue_;
    
      // time evolution operator
      double_t P20_;
      double_t P11ex_;
      double_t P11in_;
      double_t P21ex_;
      double_t P21in_;
      double_t P22_;

      int_t RefractoryCounts_;
    };

    // Access functions for UniversalDataLogger -------------------------------

    //! Read out the real membrane potential
    double_t get_V_m_() const { return S_.V_m_ + P_.U0_; }

    // ---------------------------------------------------------------- 

    /**
     * @defgroup iaf_psc_exp_data
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
    static RecordablesMap<iaf_psc_exp> recordablesMap_;
  };

  inline
  port iaf_psc_exp::check_connection(Connection &c, port receptor_type)
  {
    SpikeEvent e;
    e.set_sender(*this);
    c.check_event(e);
    return c.get_target()->connect_sender(e, receptor_type);
  }

  inline
  port iaf_psc_exp::connect_sender(SpikeEvent&, port receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return 0;
  }
 
  inline
  port iaf_psc_exp::connect_sender(CurrentEvent &, port receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return 0;
  }

  inline
  port iaf_psc_exp::connect_sender(DataLoggingRequest &dlr, 
				   port receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return B_.logger_.connect_logging_device(dlr, recordablesMap_); 
  }

  inline
  void iaf_psc_exp::get_status(DictionaryDatum &d) const
  {
    P_.get(d);
    S_.get(d, P_);
    Archiving_Node::get_status(d);

    (*d)[names::recordables] = recordablesMap_.get_list();
  }

  inline
  void iaf_psc_exp::set_status(const DictionaryDatum &d)
  {
    Parameters_ ptmp = P_;  // temporary copy in case of errors
    const double delta_EL = ptmp.set(d);                       // throws if BadProperty
    State_      stmp = S_;  // temporary copy in case of errors
    stmp.set(d, ptmp, delta_EL);                 // throws if BadProperty

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

#endif // IAF_PSC_EXP_H
