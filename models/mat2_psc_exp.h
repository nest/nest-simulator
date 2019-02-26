/*
 *  mat2_psc_exp.h
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


#ifndef MAT2_PSC_EXP_H
#define MAT2_PSC_EXP_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/** @BeginDocumentation
Name: mat2_psc_exp - Non-resetting leaky integrate-and-fire neuron model with
exponential PSCs and adaptive threshold.

Description:

mat2_psc_exp is an implementation of a leaky integrate-and-fire model
with exponential shaped postsynaptic currents (PSCs). Thus, postsynaptic
currents have an infinitely short rise time.

The threshold is lifted when the neuron is fired and then decreases in a
fixed time scale toward a fixed level [3].

The threshold crossing is followed by a total refractory period
during which the neuron is not allowed to fire, even if the membrane
potential exceeds the threshold. The membrane potential is NOT reset,
but continuously integrated.

The linear subthresold dynamics is integrated by the Exact
Integration scheme [1]. The neuron dynamics is solved on the time
grid given by the computation step size. Incoming as well as emitted
spikes are forced to that grid.

An additional state variable and the corresponding differential
equation represents a piecewise constant external current.

The general framework for the consistent formulation of systems with
neuron like dynamics interacting by point events is described in
[1]. A flow chart can be found in [2].

Remarks:

The present implementation uses individual variables for the
components of the state vector and the non-zero matrix elements of
the propagator. Because the propagator is a lower triangular matrix
no full matrix multiplication needs to be carried out and the
computation can be done "in place" i.e. no temporary state vector
object is required.

Parameters:

The following parameters can be set in the status dictionary:

C_m          double - Capacity of the membrane in pF
E_L          double - Resting potential in mV
tau_m        double - Membrane time constant in ms
tau_syn_ex   double - Time constant of postsynaptic excitatory currents in ms
tau_syn_in   double - Time constant of postsynaptic inhibitory currents in ms
t_ref        double - Duration of absolute refractory period (no spiking)
                     in ms
V_m          double - Membrane potential in mV
I_e          double - Constant input current in pA
t_spike      double - Point in time of last spike in ms
tau_1        double - Short time constant of adaptive threshold in ms
tau_2        double - Long time constant of adaptive threshold in ms
alpha_1      double - Amplitude of short time threshold adaption in mV [3]
alpha_2      double - Amplitude of long time threshold adaption in mV [3]
omega        double - Resting spike threshold in mV (absolute value, not
                     relative to E_L as in [3])

The following state variables can be read out with the multimeter device:

V_m          Non-resetting membrane potential
V_th         Two-timescale adaptive threshold

Remarks:

tau_m != tau_syn_{ex,in} is required by the current implementation to avoid a
degenerate case of the ODE describing the model [1]. For very similar values,
numerics will be unstable.

References:

[1] Rotter S & Diesmann M (1999) Exact simulation of
   time-invariant linear systems with applications to neuronal
   modeling. Biologial Cybernetics 81:381-402.
[2] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001) State
   space analysis of synchronous spiking in cortical neural
   networks. Neurocomputing 38-40:565-571.
[3] Kobayashi R, Tsubo Y and Shinomoto S (2009) Made-to-order
   spiking neuron model equipped with a multi-timescale adaptive
   threshold. Front. Comput. Neurosci. 3:9. doi:10.3389/neuro.10.009.2009

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

FirstVersion: Mai 2009

Author: Thomas Pfeil (modified iaf_psc_exp model of Moritz Helias)

*/
class mat2_psc_exp : public Archiving_Node
{

public:
  mat2_psc_exp();
  mat2_psc_exp( const mat2_psc_exp& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  port send_test_event( Node&, rport, synindex, bool );

  port handles_test_event( SpikeEvent&, rport );
  port handles_test_event( CurrentEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );

  void handle( SpikeEvent& );
  void handle( CurrentEvent& );
  void handle( DataLoggingRequest& );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();
  void update( Time const&, const long, const long );

  // The next two classes need to be friends to access private members
  friend class RecordablesMap< mat2_psc_exp >;
  friend class UniversalDataLogger< mat2_psc_exp >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {

    /** Membrane time constant in ms. */
    double Tau_;

    /** Membrane capacitance in pF. */
    double C_;

    /** Refractory period in ms. */
    double tau_ref_;

    /** Resting potential in mV. */
    double E_L_;

    /** External current in pA */
    double I_e_;

    /** Time constant of excitatory synaptic current in ms. */
    double tau_ex_;

    /** Time constant of inhibitory synaptic current in ms. */
    double tau_in_;

    /** Short and long time constant of adaptive threshold*/
    double tau_1_;
    double tau_2_;

    /** Amplitudes of threshold adaption*/
    double alpha_1_;
    double alpha_2_;

    /** Resting threshold in mV
        (relative to resting potential).
        The real resting threshold is (E_L_+omega_).
        Called omega in [3]. */
    double omega_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    double set( const DictionaryDatum& ); //!< Set values from dicitonary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    // state variables
    double i_0_;      //!< synaptic dc input current, variable 0
    double i_syn_ex_; //!< postsynaptic current for exc. inputs, variable 1
    double i_syn_in_; //!< postsynaptic current for inh. inputs, variable 1
    double V_m_;      //!< membrane potential, variable 2
    //! short time adaptive threshold (related to tau_1_), variable 1
    double V_th_1_;
    //! long time adaptive threshold (related to tau_2_), variable 2
    double V_th_2_;

    int r_; //!< total refractory counter (no spikes can be generated)

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, double );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( mat2_psc_exp& );                  //!<Sets buffer pointers to 0
    Buffers_( const Buffers_&, mat2_psc_exp& ); //!<Sets buffer pointers to 0

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_ex_;
    RingBuffer spikes_in_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< mat2_psc_exp > logger_;
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
    //    double PSCInitialValue_;

    // time evolution operator of membrane potential
    double P20_; // constant currents
    double P11ex_;
    double P11in_;
    double P21ex_;
    double P21in_;
    double P22_expm1_;

    // time evolution operator of dynamic threshold
    // P = ( exp(-h/tau_1)   0               )
    //    ( 0                 exp(-h/tau_2) )
    double P11th_;
    double P22th_;

    int RefractoryCountsTot_;
  };
  // ----------------------------------------------------------------

  //! Read out state variables, used by UniversalDataLogger
  double
  get_V_m_() const
  {
    return S_.V_m_ + P_.E_L_;
  }
  double
  get_V_th_() const
  {
    return P_.E_L_ + P_.omega_ + S_.V_th_1_ + S_.V_th_2_;
  }

  // ----------------------------------------------------------------

  /**
   * @defgroup mat2_psc_exp_data
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
  static RecordablesMap< mat2_psc_exp > recordablesMap_;
};


inline port
mat2_psc_exp::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}


inline port
mat2_psc_exp::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
mat2_psc_exp::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
mat2_psc_exp::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
mat2_psc_exp::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
mat2_psc_exp::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                 // temporary copy in case of errors
  const double delta_EL = ptmp.set( d ); // throws if BadProperty
  State_ stmp = S_;                      // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL );         // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  Archiving_Node::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif // MAT2_PSC_EXP_H
