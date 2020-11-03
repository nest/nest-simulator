/*
 *  iaf_psc_exp_htum.h
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


#ifndef iaf_psc_exp_htum_H
#define iaf_psc_exp_htum_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/* BeginUserDocs: neuron, integrate-and-fire

Short description
+++++++++++++++++

Leaky integrate-and-fire model with separate relative and absolute refractory period

Description
+++++++++++

iaf_psc_exp_htum is an implementation of a leaky integrate-and-fire model
with exponential shaped postsynaptic currents (PSCs) according to [1]_.
The postsynaptic currents have an infinitely short rise time.
In particular, this model allows setting an absolute and relative
refractory time separately, as required by [1]_.

The threshold crossing is followed by an absolute refractory period
(t_ref_abs) during which the membrane potential is clamped to the resting
potential. During the total refractory period (t_ref_tot), the membrane
potential evolves, but the neuron will not emit a spike, even if the
membrane potential reaches threshold. The total refractory time must be
larger or equal to the absolute refractory time. If equal, the
refractoriness of the model if equivalent to the other models of NEST.

The linear subthreshold dynamics is integrated by the Exact
Integration scheme [2]_. The neuron dynamics is solved on the time
grid given by the computation step size. Incoming as well as emitted
spikes are forced to that grid.

An additional state variable and the corresponding differential
equation represents a piecewise constant external current.

The general framework for the consistent formulation of systems with
neuron like dynamics interacting by point events is described in
[2]_. A flow chart can be found in [3]_.

.. note::
   The present implementation uses individual variables for the
   components of the state vector and the non-zero matrix elements of
   the propagator. Because the propagator is a lower triangular matrix,
   no full matrix multiplication needs to be carried out and the
   computation can be done "in place", i.e. no temporary state vector
   object is required.

   The template support of recent C++ compilers enables a more succinct
   formulation without loss of runtime performance already at minimal
   optimization levels. A future version of iaf_psc_exp_htum will probably
   address the problem of efficient usage of appropriate vector and
   matrix objects.

.. note::
   If tau_m is very close to tau_syn_ex or tau_syn_in, the model
   will numerically behave as if tau_m is equal to tau_syn_ex or
   tau_syn_in, respectively, to avoid numerical instabilities.
   For details, please see
   <https://github.com/nest/nest-simulator/blob/master/doc/model_details/IAF_neurons_singularity.ipynb>`_
   in the NEST source code (docs/model_details).

Parameters
++++++++++

The following parameters can be set in the status dictionary.

===========  ====== ========================================================
 E_L          mV     Resting membrane potenial
 C_m          pF     Capacity of the membrane
 tau_m        ms     Membrane time constant
 tau_syn_ex   ms     Time constant of postsynaptic excitatory currents
 tau_syn_in   ms     Time constant of postsynaptic inhibitory currents
 t_ref_abs    ms     Duration of absolute refractory period (V_m = V_reset)
 t_ref_tot    ms     Duration of total refractory period (no spiking)
 V_m          mV     Membrane potential
 V_th         mV     Spike threshold
 V_reset      mV     Reset membrane potential after a spike
 I_e          pA     Constant input current
 t_spike      ms     Point in time of last spike
===========  ====== ========================================================

References
++++++++++

.. [1] Tsodyks M, Uziel A, Markram H (2000). Synchrony generation in recurrent
       networks with frequency-dependent synapses. The Journal of Neuroscience,
       20,RC50:1-5. URL: https://infoscience.epfl.ch/record/183402
.. [2] Hill, A. V. (1936). Excitation and accommodation in nerve. Proceedings of
       the Royal Society of London. Series B-Biological Sciences, 119(814), 305-355.
       DOI: https://doi.org/10.1098/rspb.1936.0012
.. [3] Rotter S,  Diesmann M (1999). Exact simulation of
       time-invariant linear systems with applications to neuronal
       modeling. Biologial Cybernetics 81:381-402.
       DOI: https://doi.org/10.1007/s004220050570
.. [4] Diesmann M, Gewaltig M-O, Rotter S, & Aertsen A (2001). State
       space analysis of synchronous spiking in cortical neural
       networks. Neurocomputing 38-40:565-571.
       DOI: https://doi.org/10.1016/S0925-2312(01)00409-X

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

EndUserDocs */

class iaf_psc_exp_htum : public Archiving_Node
{

public:
  iaf_psc_exp_htum();
  iaf_psc_exp_htum( const iaf_psc_exp_htum& );

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

  void update( Time const&, const long, const long );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_psc_exp_htum >;
  friend class UniversalDataLogger< iaf_psc_exp_htum >;

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
    double tau_ref_tot_;
    double tau_ref_abs_;

    /** Resting potential in mV. */
    double E_L_;

    /** External current in pA */
    double I_e_;

    /** Threshold, RELATIVE TO RESTING POTENTAIL(!).
        I.e. the real threshold is (E_L_+Theta_). */
    double Theta_;

    /** reset value of the membrane potential */
    double V_reset_;

    /** Time constant of excitatory synaptic current in ms. */
    double tau_ex_;

    /** Time constant of inhibitory synaptic current in ms. */
    double tau_in_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    double set( const DictionaryDatum&, Node* node );
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

    //! absolute refractory counter (no membrane potential propagation)
    int r_abs_;
    int r_tot_; //!< total refractory counter (no spikes can be generated)

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, double delta_EL, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_psc_exp_htum& );
    Buffers_( const Buffers_&, iaf_psc_exp_htum& );

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_ex_;
    RingBuffer spikes_in_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_psc_exp_htum > logger_;
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

    // time evolution operator
    double P20_;
    double P11ex_;
    double P11in_;
    double P21ex_;
    double P21in_;
    double P22_;

    int RefractoryCountsAbs_;
    int RefractoryCountsTot_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.V_m_;
  }
  double
  get_I_syn_ex_() const
  {
    return S_.i_syn_ex_;
  }
  double
  get_I_syn_in_() const
  {
    return S_.i_syn_in_;
  }

  // ----------------------------------------------------------------

  /**
   * @defgroup iaf_psc_exp_htum_data
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
  static RecordablesMap< iaf_psc_exp_htum > recordablesMap_;
};


inline port
iaf_psc_exp_htum::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_psc_exp_htum::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_exp_htum::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_exp_htum::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}


inline void
iaf_psc_exp_htum::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_exp_htum::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;                       // temporary copy in case of errors
  const double delta_EL = ptmp.set( d, this ); // throws if BadProperty
  State_ stmp = S_;                            // temporary copy in case of errors
  stmp.set( d, ptmp, delta_EL, this );         // throws if BadProperty

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

#endif // iaf_psc_exp_htum_H
