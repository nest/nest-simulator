/*
 *  iaf_psc_exp_multisynapse.h
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

#ifndef IAF_PSC_EXP_MULTISYNAPSE_H
#define IAF_PSC_EXP_MULTISYNAPSE_H

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
/* BeginDocumentation
Name: iaf_psc_exp_multisynapse - Leaky integrate-and-fire neuron model with
                                 multiple ports.

Description:

 iaf_psc_exp_multisynapse is a direct extension of iaf_psc_exp.
 On the postsynapic side, there can be arbitrarily many synaptic
 time constants (iaf_psc_exp has exactly two: tau_syn_ex and tau_syn_in).

 This can be reached by specifying separate receptor ports, each for
 a different time constant. The port number has to match the respective
 "receptor_type" in the connectors.

Sends: SpikeEvent

Receives: SpikeEvent, CurrentEvent, DataLoggingRequest

Author:  Plesser, adapted from iaf_psc_alpha_multisynapse
SeeAlso: iaf_psc_alpha, iaf_psc_delta, iaf_psc_exp, iaf_cond_exp,
iaf_psc_alpha_multisynapse
*/
class iaf_psc_exp_multisynapse : public Archiving_Node
{

public:
  iaf_psc_exp_multisynapse();
  iaf_psc_exp_multisynapse( const iaf_psc_exp_multisynapse& );

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
  friend class RecordablesMap< iaf_psc_exp_multisynapse >;
  friend class UniversalDataLogger< iaf_psc_exp_multisynapse >;

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
    double set( const DictionaryDatum& );
  }; // Parameters_

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double I_const_; //!< synaptic dc input current, variable 0
    std::vector< double > i_syn_;
    double V_m_;     //!< membrane potential, variable 2
    double current_; //!< This is the current in a time step. This is only
                     //!< here to allow logging

    //! absolute refractory counter (no membrane potential propagation)
    int refractory_steps_;

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, const double );
  }; // State_

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_psc_exp_multisynapse& );
    Buffers_( const Buffers_&, iaf_psc_exp_multisynapse& );

    /** buffers and sums up incoming spikes/currents */
    std::vector< RingBuffer > spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_psc_exp_multisynapse > logger_;
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
    std::vector< double > P11_syn_;
    std::vector< double > P21_syn_;
    double P20_;
    double P22_;

    int RefractoryCounts_;

    unsigned int receptor_types_size_;

  }; // Variables

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.V_m_ + P_.E_L_;
  }

  /**
   * @defgroup iaf_psc_exp_multisynapse_data
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
  static RecordablesMap< iaf_psc_exp_multisynapse > recordablesMap_;
};

inline size_t
iaf_psc_exp_multisynapse::Parameters_::n_receptors_() const
{
  return tau_syn_.size();
}

inline port
iaf_psc_exp_multisynapse::send_test_event( Node& target,
  rport receptor_type,
  synindex,
  bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_psc_exp_multisynapse::handles_test_event( CurrentEvent&,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_psc_exp_multisynapse::handles_test_event( DataLoggingRequest& dlr,
  rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_psc_exp_multisynapse::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_psc_exp_multisynapse::set_status( const DictionaryDatum& d )
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

#endif /* #ifndef IAF_PSC_EXP_MULTISYNAPSE_H */
