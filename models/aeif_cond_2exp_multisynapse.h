/*
 *  aeif_cond_2exp_multisynapse.h
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

#ifndef AEIF_COND_2EXP_MULTISYNAPSE_H
#define AEIF_COND_2EXP_MULTISYNAPSE_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

/* BeginDocumentation
 Name: aeif_cond_2exp_multisynapse - Conductance based exponential
                                      integrate-and-fire neuron model according
                                      to Brette and Gerstner (2005) with
                                      multiple synaptic rise time and decay
                                      time constants, and synaptic conductance
                                      modeled by a double exponential.

 Description:

 aeif_cond_2exp_multisynapse is an extension of
 aeif_cond_alpha_multisynapse.  It allows an arbitrary number of synaptic
 rise time and decay time constant. Synaptic conductance is modeled by a
 double exponential function, as described by A. Roth and M.C.W. van Rossum
 in Computational Modeling Methods for Neuroscientists, MIT Press 2013,
 Chapter 6

 The time constants are supplied by two arrays taus_rise and taus_decay for
 the synaptic rise time and decay time, respectively.  Port numbers
 are then automatically assigned and there range is from 1 to n.  (n
 being the index of the last element of the tau_ex and tau_in
 arrays).
 During connection, the ports are selected with the property "receptor_type".

 Examples:
 % PyNEST example, of how to assign synaptic rise time and decay time
 % to a receptor type.

 nest.SetDefaults('aeif_cond_2exp_multisynapse', {'HMIN':0.001})
 nest.SetDefaults('aeif_cond_2exp_multisynapse', {'MAXERR':1e-10})

 neuron = nest.Create('aeif_cond_2exp_multisynapse')
 nest.SetStatus(neuron, {"V_peak": 0.0, "a": 4.0, "b":80.5})
 nest.SetStatus(neuron, {'taus_decay':[50.0,20.0,20.0,20.0],
                         'taus_rise':[10.0,10.0,1.0,1.0]})
 spike1 = nest.Create('spike_generator', params = {'spike_times':
                                                  np.array([10.0])})
 spike2 = nest.Create('spike_generator', params = {'spike_times':
                                                  np.array([10.0])})
 spike3 = nest.Create('spike_generator', params = {'spike_times':
                                                  np.array([10.0])})
 spike4 = nest.Create('spike_generator', params = {'spike_times':
                                                  np.array([10.0])})

 voltmeter = nest.Create('voltmeter', 1, {'withgid': True})

 nest.CopyModel("static_synapse", "synapse1", {"weight":0.1, "delay":1.0,
                                               'receptor_type': 1})
 nest.CopyModel("static_synapse", "synapse2", {"weight":0.1, "delay":300.0,
                                               'receptor_type': 2})
 nest.CopyModel("static_synapse", "synapse3", {"weight":0.1, "delay":500.0,
                                               'receptor_type': 3})
 nest.CopyModel("static_synapse", "synapse4", {"weight":-0.1, "delay":700.0,
                                               'receptor_type': 4})

 nest.Connect(spike1, neuron, model="synapse1")
 nest.Connect(spike2, neuron, model="synapse2")
 nest.Connect(spike3, neuron, model="synapse3")
 nest.Connect(spike4, neuron, model="synapse4")

 nest.Connect(voltmeter, neuron)

 Sends: SpikeEvent

 Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

 Adapted from aeif_cond_alpha_multisynapse by Bruno Golosio
 SeeAlso: aeif_cond_alpha_multisynapse
 */

namespace nest
{
/**
 * Conductance based exponential integrate-and-fire neuron model according to
 * Brette and Gerstner
 * (2005) with multiple ports.
 */
class aeif_cond_2exp_multisynapse : public Archiving_Node
{

public:
  aeif_cond_2exp_multisynapse();
  aeif_cond_2exp_multisynapse( const aeif_cond_2exp_multisynapse& );
  virtual ~aeif_cond_2exp_multisynapse();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  port send_test_event( Node&, rport, synindex, bool );

  void handle( SpikeEvent& );
  void handle( CurrentEvent& );
  void handle( DataLoggingRequest& );

  port handles_test_event( SpikeEvent&, rport );
  port handles_test_event( CurrentEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();
  void update( Time const&, const long_t, const long_t );

  inline void aeif_cond_2exp_multisynapse_dynamics(
    const std::vector< double_t >& y,
    std::vector< double_t >& f );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< aeif_cond_2exp_multisynapse >;
  friend class UniversalDataLogger< aeif_cond_2exp_multisynapse >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    double_t V_peak_;  //!< Spike detection threshold in mV
    double_t V_reset_; //!< Reset Potential in mV
    double_t t_ref_;   //!< Refractory period in ms

    double_t g_L;     //!< Leak Conductance in nS
    double_t C_m;     //!< Membrane Capacitance in pF
    double_t E_ex;    //!< Excitatory reversal Potential in mV
    double_t E_in;    //!< Inhibitory reversal Potential in mV
    double_t E_L;     //!< Leak reversal Potential (aka resting potential) in mV
    double_t Delta_T; //!< Slope faktor in ms.
    double_t tau_w;   //!< adaptation time-constant in ms.
    double_t a;       //!< Subthreshold adaptation in nS.
    double_t b;       //!< Spike-triggered adaptation in pA
    double_t V_th;    //!< Spike threshold in mV.
    double_t t_ref;   //!< Refractory period in ms.
    std::vector< double_t > taus_rise;  //!< Rise time of synaptic conductance
                                        //!< in ms..
    std::vector< double_t > taus_decay; //!< Decay time of synaptic conductance
                                        //!< in ms..

    double_t I_e;    //!< Intrinsic current in pA.
    double_t MAXERR; //!< Maximal error for adaptive stepsize solver
    double_t HMIN;   //!< Smallest permissible stepsize in ms.

    // type is long because other types are not put through in GetStatus
    std::vector< long > receptor_types_;
    size_t num_of_receptors_;

    // boolean flag which indicates whether the neuron has connections
    bool has_connections_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dictionary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor and assignment operator required because
   *       of C-style arrays.
   */
  struct State_
  {

    /**
     * Enumeration identifying elements in state vector State_::y_.
     * This enum identifies the elements of the vector. It must be public to be
     * accessible from the iteration function. The last four elements of this
     * enum (G_EXC_RISE, G_EXC_DECAY, G_INH_RISE, G_INH_DECAY) will be repeated
     * n times at the end of the state vector State_::y with n being the number
     * of synapses.
     */
    enum StateVecElems
    {
      V_M = 0,
      W,           // 1
      G_EXC_RISE,  // 2
      G_EXC_DECAY, // 3
      G_INH_RISE,  // 4
      G_INH_DECAY, // 5
      STATE_VECTOR_MIN_SIZE
    };

    static const size_t NUMBER_OF_FIXED_STATES_ELEMENTS = 2; // V_M, W
    // G_EXC_RISE, G_EXC_DECAY, G_INH_RISE, G_INH_DECAY
    static const size_t NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR = 4;

    std::vector< double_t > y_;   //!< neuron state
    std::vector< double_t > k1;   //!< Runge-Kutta variable
    std::vector< double_t > k2;   //!< Runge-Kutta variable
    std::vector< double_t > k3;   //!< Runge-Kutta variable
    std::vector< double_t > k4;   //!< Runge-Kutta variable
    std::vector< double_t > k5;   //!< Runge-Kutta variable
    std::vector< double_t > k6;   //!< Runge-Kutta variable
    std::vector< double_t > k7;   //!< Runge-Kutta variable
    std::vector< double_t > yin;  //!< Runge-Kutta variable
    std::vector< double_t > ynew; //!< 5th order update
    std::vector< double_t > yref; //!< 4th order update
    int_t r_;                     //!< number of refractory steps remaining

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );

  }; // State_

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( aeif_cond_2exp_multisynapse& );
    Buffers_( const Buffers_&, aeif_cond_2exp_multisynapse& );

    //! Logger for all analog data
    UniversalDataLogger< aeif_cond_2exp_multisynapse > logger_;

    /** buffers and sums up incoming spikes/currents */
    std::vector< RingBuffer > spike_exc_;
    std::vector< RingBuffer > spike_inh_;
    RingBuffer currents_;

    // IntergrationStep_ should be reset with the neuron on ResetNetwork,
    // but remain unchanged during calibration. Since it is initialized with
    // step_, and the resolution cannot change after nodes have been created,
    // it is safe to place both here.
    double_t step_;          //!< simulation step size in ms
    double IntegrationStep_; //!< current integration time step,
                             //!< updated by solver

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double_t I_stim_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {

    /** initial value to normalise excitatory synaptic conductance */
    std::vector< double_t > g0_ex_;

    /** initial value to normalise inhibitory synaptic conductance */
    std::vector< double_t > g0_in_;

    int_t RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double_t
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }

  // Data members -----------------------------------------------------------

  /**
   * @defgroup aeif_cond_2exp_multisynapse
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is important for speed.
   * @{
   */
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
  /** @} */

  //! Mapping of recordables names to access functions
  static RecordablesMap< aeif_cond_2exp_multisynapse > recordablesMap_;
};

inline port
aeif_cond_2exp_multisynapse::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
aeif_cond_2exp_multisynapse::handles_test_event( CurrentEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
aeif_cond_2exp_multisynapse::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
aeif_cond_2exp_multisynapse::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
aeif_cond_2exp_multisynapse::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

/**
 * Function computing right-hand side of ODE for the ODE solver.
 * @param y State vector (input).
 * @param f Derivatives (output).
 */
inline void
aeif_cond_2exp_multisynapse::aeif_cond_2exp_multisynapse_dynamics(
  const std::vector< double_t >& y,
  std::vector< double_t >& f )
{
  // a shorthand
  typedef aeif_cond_2exp_multisynapse::State_ S;

  // y[] is the current internal state of the integrator (yin), not the state
  // vector in the node, node.S_.y[].

  // The following code is verbose for the sake of clarity. We assume that a
  // good compiler will optimize the verbosity away ...

  // shorthand for state variables
  const double_t& V = y[ S::V_M ];
  const double_t& w = y[ S::W ];

  double_t I_syn_exc = 0.0;
  double_t I_syn_inh = 0.0;

  for ( size_t i = 0; i < ( P_.num_of_receptors_
                            * S::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR );
        i += S::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR )
  {
    I_syn_exc +=
      ( y[ S::G_EXC_DECAY + i ] - y[ S::G_EXC_RISE + i ] ) * ( V - P_.E_ex );
    I_syn_inh +=
      ( y[ S::G_INH_DECAY + i ] - y[ S::G_INH_RISE + i ] ) * ( V - P_.E_in );
  }

  // We pre-compute the argument of the exponential
  const double_t exp_arg = ( V - P_.V_th ) / P_.Delta_T;

  // Upper bound for exponential argument to avoid numerical instabilities
  const double_t MAX_EXP_ARG = 10.;

  // If the argument is too large, we clip it.
  const double_t I_spike =
    P_.Delta_T * std::exp( std::min( exp_arg, MAX_EXP_ARG ) );

  // dv/dt
  f[ S::V_M ] = ( -P_.g_L * ( ( V - P_.E_L ) - I_spike ) - I_syn_exc - I_syn_inh
                  - w + P_.I_e + B_.I_stim_ ) / P_.C_m;

  // Adaptation current w.
  f[ S::W ] = ( P_.a * ( V - P_.E_L ) - w ) / P_.tau_w;

  for ( size_t i = 0; i < P_.num_of_receptors_; ++i )
  {
    size_t j = i * S::NUMBER_OF_STATES_ELEMENTS_PER_RECEPTOR;
    // Synaptic conductance derivative dG/dt (nS) for excitatory connections
    f[ S::G_EXC_RISE + j ] = -y[ S::G_EXC_RISE + j ] / P_.taus_rise[ i ];
    f[ S::G_EXC_DECAY + j ] = -y[ S::G_EXC_DECAY + j ] / P_.taus_decay[ i ];

    // Synaptic conductance derivative dG/dt (nS) for inhibitory connections
    f[ S::G_INH_RISE + j ] = -y[ S::G_INH_RISE + j ] / P_.taus_rise[ i ];
    f[ S::G_INH_DECAY + j ] = -y[ S::G_INH_DECAY + j ] / P_.taus_decay[ i ];
  }
}

} // namespace

#endif /* #ifndef AEIF_COND_2EXP_MULTISYNAPSE_H */
