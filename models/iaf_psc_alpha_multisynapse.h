/*
 *  iaf_psc_alpha_multisynapse.h
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

#ifndef IAF_PSC_ALPHA_MULTISYNAPSE_H
#define IAF_PSC_ALPHA_MULTISYNAPSE_H

// Generated includes:
#include <sstream>

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

/* BeginUserDocs: neuron, integrate-and-fire, current-based

Short description
+++++++++++++++++

Leaky integrate-and-fire neuron model with multiple ports

Description
+++++++++++

iaf_psc_alpha_multisynapse is a direct extension of iaf_psc_alpha.
On the postsynaptic side, there can be arbitrarily many synaptic
time constants (iaf_psc_alpha has exactly two: tau_syn_ex and tau_syn_in).

This can be reached by specifying separate receptor ports, each for
a different time constant. The port number has to match the respective
"receptor_type" in the connectors.

.. note::

   If `tau_m` is very close to a synaptic time constant, the model
   will numerically behave as if `tau_m` is equal to the synaptic
   time constant, to avoid numerical instabilities.

   For implementation details see the
   `IAF_neurons_singularity <../model_details/IAF_neurons_singularity.ipynb>`_ notebook.

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

iaf_psc_alpha, iaf_psc_delta, iaf_psc_exp, iaf_cond_exp, iaf_psc_exp_multisynapse

EndUserDocs */

class iaf_psc_alpha_multisynapse : public ArchivingNode
{

public:
  iaf_psc_alpha_multisynapse();
  iaf_psc_alpha_multisynapse( const iaf_psc_alpha_multisynapse& );

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
  friend class DynamicRecordablesMap< iaf_psc_alpha_multisynapse >;
  friend class DynamicUniversalDataLogger< iaf_psc_alpha_multisynapse >;
  friend class DataAccessFunctor< iaf_psc_alpha_multisynapse >;

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
    double refractory_time_;

    /** Resting potential in mV. */
    double E_L_;

    /** External current in pA */
    double I_e_;

    /** Reset value of the membrane potential */
    double V_reset_;

    /** Threshold, RELATIVE TO RESTING POTENTIAL(!).
        I.e. the real threshold is (E_L_+Theta_). */
    double Theta_;

    /** Lower bound, RELATIVE TO RESTING POTENTIAL(!).
        I.e. the real lower bound is (LowerBound_+Theta_). */
    double LowerBound_;

    /** Time constants of synaptic currents in ms. */
    std::vector< double > tau_syn_;

    // boolean flag which indicates whether the neuron has connections
    bool has_connections_;

    size_t n_receptors_() const; //!< Returns the size of tau_syn_

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    double set( const DictionaryDatum&, Node* node );
  }; // Parameters_

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    /**
     * Enumeration identifying recordable state elements.
     * This enum identifies the element that will be recorded when
     * calling get_state_element. The first two (V_M and I) are fixed
     * sized state elements, while the third (I_SYN) represents the
     * synaptic current at each receptor, thus it can have a variable
     * size. The current at each receptor is read out from the vector
     * y2_syn_. To get the synaptic current's value at synapse k, one
     * must call get_state_element as:
     * get_state_element( State_::I_SYN + k *
     *    State_::NUM_STATE_ELEMENTS_PER_RECEPTOR )
     */
    enum StateVecElems
    {
      V_M = 0,
      I,    // 1
      I_SYN // 2
    };

    static const size_t NUMBER_OF_FIXED_STATES_ELEMENTS = I_SYN; // V_M, I
    static const size_t NUM_STATE_ELEMENTS_PER_RECEPTOR = 1;     // I_SYN

    double I_const_; //!< Constant current
    std::vector< double > y1_syn_;
    std::vector< double > y2_syn_;
    //! This is the membrane potential RELATIVE TO RESTING POTENTIAL.
    double V_m_;
    double current_; //! This is the current in a time step. This is only here
                     //! to allow logging

    int refractory_steps_; //!< Number of refractory steps remaining

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, const double, Node* );
  }; // State_

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_psc_alpha_multisynapse& );
    Buffers_( const Buffers_&, iaf_psc_alpha_multisynapse& );

    /** buffers and sums up incoming spikes/currents */
    std::vector< RingBuffer > spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    DynamicUniversalDataLogger< iaf_psc_alpha_multisynapse > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    std::vector< double > PSCInitialValues_;
    int RefractoryCounts_;

    std::vector< double > P11_syn_;
    std::vector< double > P21_syn_;
    std::vector< double > P22_syn_;
    std::vector< double > P31_syn_;
    std::vector< double > P32_syn_;

    double P30_;
    double P33_;

    unsigned int receptor_types_size_;

  }; // Variables

  // Data members -----------------------------------------------------------

  /**
   * @defgroup iaf_psc_alpha_multisynapse_data
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
  DynamicRecordablesMap< iaf_psc_alpha_multisynapse > recordablesMap_;

  // Data Access Functor getter
  DataAccessFunctor< iaf_psc_alpha_multisynapse > get_data_access_functor( size_t elem );
  inline double
  get_state_element( size_t elem )
  {
    if ( elem == State_::V_M )
    {
      return S_.V_m_ + P_.E_L_;
    }
    else if ( elem == State_::I )
    {
      return S_.current_;
    }
    else
    {
      return S_.y2_syn_[ elem - S_.NUMBER_OF_FIXED_STATES_ELEMENTS ];
    }
  };

  // Utility function that inserts the synaptic conductances to the
  // recordables map

  Name get_i_syn_name( size_t elem );
  void insert_current_recordables( size_t first = 0 );
};

inline size_t
iaf_psc_alpha_multisynapse::Parameters_::n_receptors_() const
{
  return tau_syn_.size();
}

inline port
iaf_psc_alpha_multisynapse::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_psc_alpha_multisynapse::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_alpha_multisynapse::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_alpha_multisynapse::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

} // namespace

#endif /* #ifndef IAF_PSC_ALPHA_MULTISYNAPSE_H */
