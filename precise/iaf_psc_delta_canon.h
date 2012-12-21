/*
 *  iaf_psc_delta_canon.h
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

#ifndef IAF_PSC_DELTA_CANON_H
#define IAF_PSC_DELTA_CANON_H

#include "config.h"

#include "nest.h"
#include "event.h"
#include "node.h"
#include "slice_ring_buffer.h"
#include "ring_buffer.h"
#include "connection.h"

#include "universal_data_logger.h"

namespace nest{
  
  class Network;

  /* BeginDocumentation
     Name: iaf_psc_delta_canon - Leaky integrate-and-fire neuron model.

     Description:
     iaf_psc_delta_canon is an implementation of a leaky integrate-and-fire model
     where the potential jumps on each spike arrival. 

     The threshold crossing is followed by an absolute refractory period
     during which the membrane potential is clamped to the resting
     potential.  

     Spikes arriving while the neuron is refractory, are discarded by
     default. If the property "refractory_input" is set to true, such
     spikes are added to the membrane potential at the end of the
     refractory period, dampened according to the interval between
     arrival and end of refractoriness.

     The linear subthresold dynamics is integrated by the Exact
     Integration scheme [1]. The neuron dynamics are solved exactly in
     time. Incoming and outgoing spike times are handled precisely [3].

     An additional state variable and the corresponding differential
     equation represents a piecewise constant external current.

     Spikes can occur either on receipt of an excitatory input spike, or
     be caused by a depolarizing input current.  Spikes evoked by
     incoming spikes, will occur precisely at the time of spike arrival,
     since incoming spikes are modeled as instantaneous potential
     jumps. Times of spikes caused by current input are determined
     exactly by solving the membrane potential equation. Note that, in
     contrast to the neuron models discussed in [3,4], this model has so
     simple dynamics that no interpolation or iterative spike location
     technique is required at all.

     The general framework for the consistent formulation of systems with
     neuron like dynamics interacting by point events is described in
     [1].  A flow chart can be found in [2].

     Critical tests for the formulation of the neuron model are the
     comparisons of simulation results for different computation step
     sizes. sli/testsuite/nest contains a number of such tests.
  
     The iaf_psc_delta_canon is the standard model used to check the consistency
     of the nest simulation kernel because it is at the same time complex
     enough to exhibit non-trivial dynamics and simple enough compute
     relevant measures analytically.

     Remarks:

     The iaf_psc_delta_canon neuron accepts CurrentEvent connections.
     However, the present method for transmitting CurrentEvents in 
     NEST (sending the current to be applied) is not compatible with off-grid
     currents, if more than one CurrentEvent-connection exists. Once CurrentEvents
     are changed to transmit change-of-current-strength, this problem will 
     disappear and the canonical neuron will also be able to handle CurrentEvents.

     The present implementation uses individual variables for the
     components of the state vector and the non-zero matrix elements of
     the propagator.  Because the propagator is a lower triangular matrix
     no full matrix multiplication needs to be carried out and the
     computation can be done "in place" i.e. no temporary state vector
     object is required.

     The template support of recent C++ compilers enables a more succinct
     formulation without loss of runtime performance already at minimal
     optimization levels. A future version of iaf_psc_delta_canon will probably
     address the problem of efficient usage of appropriate vector and
     matrix objects.
     
     Please note that this node is capable of sending precise spike times
     to target nodes (on-grid spike time plus offset). If this node is
     connected to a spike_detector, the property "precise_times" of the
     spike_detector has to be set to true in order to record the offsets
     in addition to the on-grid spike times.

     Parameters: 
     The following parameters can be set in the status dictionary.

     V_m        double - Membrane potential in mV 
     E_L        double - Resting membrane potential in mV. 
     C_m        double - Specific capacitance of the membrane in pF/mum^2 
     tau_m      double - Membrane time constant in ms. 
     t_ref      double - Duration of refractory period in ms. 
     V_th       double - Spike threshold in mV. 
     V_reset    double - Reset potential of the membrane in mV.
     I_e        double - Constant input current in pA. 
     V_min      double - Absolute lower value for the membrane potential.

     refractory_input bool - If true, do not discard input during
     refractory period. Default: false.
 
     References:
     [1] Rotter S & Diesmann M (1999) Exact simulation of time-invariant linear
     systems with applications to neuronal modeling. Biologial Cybernetics
     81:381-402.
     [2] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001) State space 
     analysis of synchronous spiking in cortical neural networks. 
     Neurocomputing 38-40:565-571.
     [3] Morrison A, Straube S, Plesser H E, & Diesmann M (2006) Exact Subthreshold 
     Integration with Continuous Spike Times in Discrete Time Neural Network 
     Simulations. To appear in Neural Computation.
     [4] Hanuschkin A, Kunkel S, Helias M, Morrison A & Diesmann M (2010) 
     A general and efficient method for incorporating exact spike times in 
     globally time-driven simulations Front Neuroinformatics, 4:113

     Sends: SpikeEvent

     Receives: SpikeEvent, CurrentEvent, DataLoggingRequest
     
     Author:  May 2006, Plesser; based on work by Diesmann, Gewaltig, Morrison, Straube, Eppler
     SeeAlso: iaf_psc_delta, iaf_psc_exp_ps
  */

  class iaf_psc_delta_canon:
  public Node
  {
  
  public:        
    
    typedef Node base;
    
    /** Basic constructor.
	This constructor should only be used by GenericModel to create 
	model prototype instances.
    */
    iaf_psc_delta_canon();

    /** Copy constructor.
	GenericModel::allocate_() uses the copy constructor to clone
	actual model instances from the prototype instance. 
      
	@note The copy constructor MUST NOT be used to create nodes based
	on nodes that have been placed in the network.
    */ 
    iaf_psc_delta_canon(const iaf_psc_delta_canon&);

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

    bool is_off_grid() const {return true;}  // uses off_grid events    
    port connect_sender(SpikeEvent &, port);
    port connect_sender(CurrentEvent &, port);
    port connect_sender(DataLoggingRequest &, port);

    void get_status(DictionaryDatum &) const;
    void set_status(const DictionaryDatum &) ;

  private:

    /** @name Interface functions
     * @note These functions are private, so that they can be accessed
     * only through a Node*. 
     */
    //@{
    void init_state_(const Node& proto);
    void init_buffers_();

    void calibrate();
    void update(Time const &, const long_t, const long_t);
    
    void set_spiketime(Time const &);
    Time get_spiketime() const;

    /**
     * Emit a single spike caused by DC current in absence of spike input.
     * Emits a single spike and reset neuron given that the membrane
     * potential was below threshold at the beginning of a mini-timestep
     * and above afterwards.
     *
     * @param origin    Time stamp at beginning of slice
     * @param lag       Time step within slice
     * @param offset_U  Time offset for U value, ie for time when threhold 
     *                  crossing was detected
     */
    void emit_spike_(Time const& origin, const long_t lag, 
		     const double_t offset_U);

    /**
     * Emit a single spike caused instantly by an input spike.
     *
     * @param origin    Time stamp at beginning of slice
     * @param lag       Time step within slice
     * @param spike_offset  Time offset for spike
     */
    void emit_instant_spike_(Time const& origin, const long_t lag, 
			     const double_t spike_offset);

    /**
     * Propagate neuron state.
     * Propagate the neuron's state by dt.
     * @param dt Interval over which to propagate
     */
    void propagate_(const double_t dt);

    // ---------------------------------------------------------------- 

    /** 
     * Independent parameters of the model. 
     */
    struct Parameters_ {

      /** Membrane time constant in ms. */
      double_t tau_m_; 

      /** Membrane capacitance in pF. */
      double_t c_m_;

      /** Refractory period in ms. */
      double_t t_ref_;

      /** Resting potential in mV. */
      double_t E_L_;

      /** External DC current [pA] */
      double_t I_e_;

      /** Threshold, RELATIVE TO RESTING POTENTAIL(!).
          I.e. the real threshold is U_th_ + E_L_. */
      double_t U_th_;

      /** Lower bound, RELATIVE TO RESTING POTENTAIL(!).
          I.e. the real lower bound is U_min_+E_L_. */
      double_t U_min_;

      /** Reset potential. 
	  At threshold crossing, the membrane potential is reset to this value. 
	  Relative to resting potential.
      */
      double_t U_reset_;

      Parameters_();  //!< Sets default parameter values

      void get(DictionaryDatum&) const;  //!< Store current values in dictionary

      /** Set values from dictionary.
       * @returns Change in reversal potential E_L, to be passed to State_::set()
       */
      double set(const DictionaryDatum&);
    };
    

    // The next two classes need to be friends to access the State_ class/member
    friend class RecordablesMap<iaf_psc_delta_canon>;
    friend class UniversalDataLogger<iaf_psc_delta_canon>;

    // ---------------------------------------------------------------- 

    /**
     * State variables of the model.
     */
    struct State_ {
      double_t U_;  //!< This is the membrane potential RELATIVE TO RESTING POTENTIAL.
      double_t I_;  //!< This is the current to be applied during this time step
      
      long_t   last_spike_step_;   //!< step of last spike, for reporting in status dict
      double_t last_spike_offset_; //!< offset of last spike, for reporting in status dict
      
      bool     is_refractory_;     //!< flag for refractoriness
      bool     with_refr_input_;   //!< spikes arriving during refractory period are counted
      
      State_();  //!< Default initialization
      
      void get(DictionaryDatum&, const Parameters_&) const;

      /** Set values from dictionary.
       * @param dictionary to take data from
       * @param current parameters
       * @param Change in reversal potential E_L specified by this dict
       */
      void set(const DictionaryDatum&, const Parameters_&, double);
    };
    
    // ---------------------------------------------------------------- 

    /**
     * Buffers of the model.
     */
    struct Buffers_ {
      Buffers_(iaf_psc_delta_canon&);
      Buffers_(const Buffers_&, iaf_psc_delta_canon&);

      /**
       * Queue for incoming events.
       * @note Return from refractoriness is stored as events "spikes"
       *       with weight == numerics::NaN
       */
      SliceRingBuffer events_;

      /**
       * Queue for incoming current events.
       */
      RingBuffer currents_;
  
      //! Logger for all analog data
      UniversalDataLogger<iaf_psc_delta_canon> logger_;
    };

    // ---------------------------------------------------------------- 

    /**
     * Internal variables of the model.
     */
    struct Variables_ { 
      double_t exp_t_;     //!< @$ e^{-t/\tau_m} @$
      double_t expm1_t_;   //!< @$ e^{-t/\tau_m} - 1 @$
      double_t v_inf_;     //!< @$ \frac{I_e\tau_m}{c_m} @$
      double_t I_contrib_; //!< @$ \frac{I_e\tau_m}{c_m} (1-e^{-t/\tau_m})@$
      
      double_t h_ms_;  //!< duration of time step [ms]
      
      long_t   refractory_steps_;  //!< refractory time in steps
      
      /** Accumulate spikes arriving during refractory period, discounted for
	  decay until end of refractory period.
      */
      double_t    refr_spikes_buffer_;
    };
    
    // Access functions for UniversalDataLogger -------------------------------

    //! Read out the real membrane potential
    double_t get_V_m_() const { return S_.U_ + P_.E_L_; }

    // ---------------------------------------------------------------- 

    /**
     * @defgroup iaf_psc_delta_data
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
   static RecordablesMap<iaf_psc_delta_canon> recordablesMap_;

  };
  
  inline
    port iaf_psc_delta_canon::check_connection(Connection& c, port receptor_type)
    {
      SpikeEvent e;
      e.set_sender(*this);
      c.check_event(e);
      return c.get_target()->connect_sender(e, receptor_type);
    }

  inline
    port iaf_psc_delta_canon::connect_sender(SpikeEvent&, port receptor_type)
    {
      if (receptor_type != 0)
	throw UnknownReceptorType(receptor_type, get_name());
      return 0;
    }

  inline
    port iaf_psc_delta_canon::connect_sender(CurrentEvent&, port receptor_type)
    {
      if (receptor_type != 0)
	throw UnknownReceptorType(receptor_type, get_name());
      return 0;
    }

  inline
    port iaf_psc_delta_canon::connect_sender(DataLoggingRequest& dlr, 
					     port receptor_type)
    {
      if (receptor_type != 0)
	throw UnknownReceptorType(receptor_type, get_name());
      return B_.logger_.connect_logging_device(dlr, recordablesMap_);
    }
  
  inline 
    Time iaf_psc_delta_canon::get_spiketime() const
    {
      return Time::step(S_.last_spike_step_);
    }

  inline
    void iaf_psc_delta_canon::get_status(DictionaryDatum &d) const
  {
    P_.get(d);
    S_.get(d, P_);
    (*d)[names::recordables] = recordablesMap_.get_list();
  }

  inline
    void iaf_psc_delta_canon::set_status(const DictionaryDatum &d)
  {
    Parameters_ ptmp = P_;  // temporary copy in case of errors
    const double delta_EL = ptmp.set(d);                       // throws if BadProperty
    State_      stmp = S_;  // temporary copy in case of errors
    stmp.set(d, ptmp, delta_EL);                 // throws if BadProperty

    // if we get here, temporaries contain consistent set of properties
    P_ = ptmp;
    S_ = stmp;
  }

} // namespace

#endif //IAF_PSC_DELTA_CANON_H
