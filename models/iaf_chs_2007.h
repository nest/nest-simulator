/*
 *  iaf_chs_2007.h
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

#ifndef IAF_CHS_2007_H
#define IAF_CHS_2007_H

// Includes from librandom:
#include "normal_randomdev.h"

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

/* BeginUserDocs: neuron, integrate-and-fire

Short description
+++++++++++++++++

Spike-response model used in Carandini et al. 2007

Description
+++++++++++

The membrane potential is the sum of stereotyped events: the postsynaptic
potentials (V_syn), waveforms that include a spike and the subsequent
after-hyperpolarization (V_spike) and Gaussian-distributed white noise.

The postsynaptic potential is described by alpha function where
U_epsp is the maximal amplitude of the EPSP and tau_epsp is the time to
peak of the EPSP.

The spike waveform is described as a delta peak followed by a membrane
potential reset and exponential decay. U_reset is the magnitude of the
reset/after-hyperpolarization and tau_reset is the time constant of
recovery from this hyperpolarization.

The linear subthreshold dynamics is integrated by the Exact
Integration scheme [1]_. The neuron dynamics is solved on the time
grid given by the computation step size. Incoming as well as emitted
spikes are forced to that grid.

Remarks:
The way the noise term was implemented in the original model makes it
unsuitable for simulation in NEST. The workaround was to prepare the
noise signal externally prior to simulation. The noise signal,
if present, has to be at least as long as the simulation.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

========== ============== ==================================================
 tau_epsp  ms             Membrane time constant
 tau_reset ms             Refractory time constant
 U_epsp    real           Maximum amplitude of the EPSP, normalized
 U_reset   real           Reset value of the membrane potential, normalized
 U_noise   real           Noise scale, normalized
 noise     list of real   Noise signal
========== ============== ==================================================

References
++++++++++

.. [1] Carandini M, Horton JC, Sincich LC (2007). Thalamic filtering of retinal
       spike trains by postsynaptic summation. Journal of Vision 7(14):20,1-11.
       DOI: https://doi.org/10.1167/7.14.20
.. [2] Rotter S,  Diesmann M (1999). Exact simulation of time-invariant linear
       systems with applications to neuronal modeling. Biologial Cybernetics
       81:381-402.
       DOI: https://doi.org/10.1007/s004220050570

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, DataLoggingRequest

EndUserDocs */

class iaf_chs_2007 : public Archiving_Node
{

public:
  iaf_chs_2007();
  iaf_chs_2007( const iaf_chs_2007& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  port send_test_event( Node&, rport, synindex, bool );

  void handle( SpikeEvent& );
  void handle( DataLoggingRequest& );

  port handles_test_event( SpikeEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_node_( const Node& proto );
  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  void update( const Time&, const long, const long );

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< iaf_chs_2007 >;
  friend class UniversalDataLogger< iaf_chs_2007 >;

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    // state variables
    double i_syn_ex_; // postsynaptic current for exc. inputs, variable 1
    double V_syn_;    // psp waveform, variable 2
    double V_spike_;  // post spike reset waveform, variable 3
    double V_m_;      // membrane potential, variable 4

    unsigned long position_;

    State_(); //!< Default initialization

    void get( DictionaryDatum& ) const;
    void set( DictionaryDatum const&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Membrane time constant in ms. */
    double tau_epsp_;

    /** Refractory time constant in ms. */
    double tau_reset_;

    /** Resting potential. Normalized = 0.0. */
    double E_L_;

    /** Threshold. Normalized = 1.0. */
    double U_th_;

    /** Normalized maximum amplitude of the EPSP. */
    double U_epsp_;

    /** Normalized magnitude of the membrane potential reset. */
    double U_reset_;

    /** Membrane capacitance. Note: Does not have any function currently. */
    double C_;

    /** Noise scale. */
    double U_noise_;

    /** Noise signal. */
    std::vector< double > noise_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     * @note State is passed so that the position can be reset if the
     *       noise_ vector has been filled with new data.
     */
    void set( const DictionaryDatum&, State_& s, Node* node );
  };


  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( iaf_chs_2007& );
    Buffers_( const Buffers_&, iaf_chs_2007& );

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_ex_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< iaf_chs_2007 > logger_;
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
    double P21ex_;
    double P22_;
    double P30_;

    librandom::NormalRandomDev normal_dev_; //!< random deviate generator
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the real membrane potential
  double
  get_V_m_() const
  {
    return S_.V_m_ + P_.E_L_;
  }

  // ----------------------------------------------------------------

  /**
   * @defgroup iaf_psc_exp_data
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
  static RecordablesMap< iaf_chs_2007 > recordablesMap_;
};

inline port
iaf_chs_2007::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline port
iaf_chs_2007::handles_test_event( SpikeEvent&, port receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
iaf_chs_2007::handles_test_event( DataLoggingRequest& dlr, port receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
iaf_chs_2007::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Archiving_Node::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
iaf_chs_2007::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, S_, this );
  State_ stmp = S_;    // temporary copy in case of errors
  stmp.set( d, this ); // throws if BadProperty

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

#endif // IAF_CHS_2007_H
