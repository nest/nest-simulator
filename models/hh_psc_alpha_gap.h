/*
 *  hh_psc_alpha_gap.h
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

#ifndef HH_PSC_ALPHA_GAP_H
#define HH_PSC_ALPHA_GAP_H

#include "config.h"

#ifdef HAVE_GSL

// C includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>
#include <gsl/gsl_sf_exp.h>

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "node.h"
#include "ring_buffer.h"
#include "recordables_map.h"
#include "universal_data_logger.h"

namespace nest
{

/**
 * Function computing right-hand side of ODE for GSL solver.
 * @note Must be declared here so we can befriend it in class.
 * @note Must have C-linkage for passing to GSL. Internally, it is
 *       a first-class C++ function, but cannot be a member function
 *       because of the C-linkage.
 * @note No point in declaring it inline, since it is called
 *       through a function pointer.
 * @param void* Pointer to model neuron instance.
 */
extern "C" int hh_psc_alpha_gap_dynamics( double, const double*, double*, void* );

/* BeginUserDocs: neuron, current-based, Hodgkin-Huxley, gap junction

Short description
+++++++++++++++++

Hodgkin-Huxley neuron model with gap-junction support

Description
+++++++++++

hh_psc_alpha_gap is an implementation of a spiking neuron using the
Hodgkin-Huxley formalism. In contrast to hh_psc_alpha the implementation
additionally supports gap junctions.

1. Postsynaptic currents
Incoming spike events induce a postsynaptic change of current modelled
by an alpha function. The alpha function is normalized such that an event of
weight 1.0 results in a peak current of 1 pA.

2. Spike Detection
Spike detection is done by a combined threshold-and-local-maximum search: if
there is a local maximum above a certain threshold of the membrane potential,
it is considered a spike.

3. Gap Junctions
Gap Junctions are implemented by a gap current of the form
:math:`g_{ij}( V_i - V_j)`.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

===========  ====== ============================================================
tau_ex       ms      Rise time of the excitatory synaptic alpha function
tau_in       ms      Rise time of the inhibitory synaptic alpha function
g_K          nS      Potassium peak conductance
V_m          mV      Membrane potential
E_L          mV      Leak reversal potential
g_L          nS      Leak conductance
C_m          pF      Capacity of the membrane
tau_syn_ex   ms      Rise time of the excitatory synaptic alpha function
tau_syn_in   ms      Rise time of the inhibitory synaptic alpha function
E_Na         mV      Sodium reversal potential
g_Na         nS      Sodium peak conductance
E_K          mV      Potassium reversal potential
g_Kv1        nS      Potassium peak conductance
g_Kv3        nS      Potassium peak conductance
Act_m        real    Activation variable m
Inact_h      real    Inactivation variable h
Act_n        real    Activation variable n
I_e          pA      External input current
===========  ====== ============================================================

References
++++++++++

.. [1] Gerstner W, Kistler W. Spiking neuron models: Single neurons,
       populations, plasticity. Cambridge University Press
.. [2] Mancilla JG, Lewis TG, Pinto DJ, Rinzel J, Connors BW (2007).
       Synchronization of electrically coupled pairs of inhibitory
       interneurons in neocortex, Journal of Neurosciece, 27:2058-2073
       DOI: https://doi.org/10.1523/JNEUROSCI.2715-06.2007
       (parameters taken from here)
.. [3] Hodgkin AL and Huxley A F (1952). A quantitative description of
       membrane current and its application to conduction and excitation in
       nerve. The Journal of Physiology 117.
       DOI: https://doi.org/10.1113/jphysiol.1952.sp004764
.. [4] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann M
       (2015). A unified framework for spiking and gap-junction interactions
       in distributed neuronal netowrk simulations. Frontiers in
       Neuroinformatics, 9:22. DOI: https://doi.org/10.3389/fninf.2015.00022

Sends
+++++

SpikeEvent, GapJunctionEvent

Receives
++++++++

SpikeEvent, GapJunctionEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

hh_psc_alpha, hh_cond_exp_traub, gap_junction

EndUserDocs */

class hh_psc_alpha_gap : public ArchivingNode
{

public:
  typedef Node base;

  hh_psc_alpha_gap();
  hh_psc_alpha_gap( const hh_psc_alpha_gap& );
  ~hh_psc_alpha_gap();

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_secondary_event;

  port send_test_event( Node& target, rport receptor_type, synindex, bool );

  void handle( SpikeEvent& );
  void handle( CurrentEvent& );
  void handle( DataLoggingRequest& );
  void handle( GapJunctionEvent& );

  port handles_test_event( SpikeEvent&, rport );
  port handles_test_event( CurrentEvent&, rport );
  port handles_test_event( DataLoggingRequest&, rport );
  port handles_test_event( GapJunctionEvent&, rport );

  void
  sends_secondary_event( GapJunctionEvent& )
  {
  }

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_buffers_();
  void calibrate();

  /** This is the actual update function. The additional boolean parameter
   * determines if the function is called by update (false) or wfr_update (true)
   */
  bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long );
  bool wfr_update( Time const&, const long, const long );

  // END Boilerplate function declarations ----------------------------

  // Friends --------------------------------------------------------

  // make dynamics function quasi-member
  friend int hh_psc_alpha_gap_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friend to access the State_ class/member
  friend class RecordablesMap< hh_psc_alpha_gap >;
  friend class UniversalDataLogger< hh_psc_alpha_gap >;

private:
  // ----------------------------------------------------------------

  //! Independent parameters
  struct Parameters_
  {
    double t_ref_;   //!< refractory time in ms
    double g_Na;     //!< Sodium Conductance in nS
    double g_Kv1;    //!< Potassium Conductance in nS
    double g_Kv3;    //!< Potassium Conductance in nS
    double g_L;      //!< Leak Conductance in nS
    double C_m;      //!< Membrane Capacitance in pF
    double E_Na;     //!< Sodium Reversal Potential in mV
    double E_K;      //!< Potassium Reversal Potential in mV
    double E_L;      //!< Leak reversal Potential (aka resting potential) in mV
    double tau_synE; //!< Synaptic Time Constant Excitatory Synapse in ms
    double tau_synI; //!< Synaptic Time Constant for Inhibitory Synapse in ms
    double I_e;      //!< Constant Current in pA

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dicitonary
  };

public:
  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor required because of C-style array.
   */
  struct State_
  {
    /**
     * Enumeration identifying elements in state array State_::y_.
     * The state vector must be passed to GSL as a C array. This enum
     * identifies the elements of the vector. It must be public to be
     * accessible from the iteration function.
     */
    enum StateVecElems
    {
      V_M = 0,
      HH_M,   // 1
      HH_H,   // 2
      HH_N,   // 3
      HH_P,   // 4
      DI_EXC, // 5
      I_EXC,  // 6
      DI_INH, // 7
      I_INH,  // 8
      STATE_VEC_SIZE
    };

    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];
    int r_; //!< number of refractory steps remaining

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );

    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, Node* node );
  };

  // ----------------------------------------------------------------

private:
  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( hh_psc_alpha_gap& ); //!<Sets buffer pointers to 0
    //! Sets buffer pointers to 0
    Buffers_( const Buffers_&, hh_psc_alpha_gap& );

    //! Logger for all analog data
    UniversalDataLogger< hh_psc_alpha_gap > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spike_exc_;
    RingBuffer spike_inh_;
    RingBuffer currents_;

    /** GSL ODE stuff */
    gsl_odeiv_step* s_;    //!< stepping function
    gsl_odeiv_control* c_; //!< adaptive stepsize control function
    gsl_odeiv_evolve* e_;  //!< evolution function
    gsl_odeiv_system sys_; //!< struct describing system

    // Since IntergrationStep_ is initialized with step_, and the resolution
    // cannot change after nodes have been created, it is safe to place both
    // here.
    double step_;            //!< step size in ms
    double IntegrationStep_; //!< current integration time step, updated by GSL

    // remembers current lag for piecewise interpolation
    long lag_;
    // remembers y_values from last wfr_update
    std::vector< double > last_y_values;
    // summarized gap weight
    double sumj_g_ij_;
    // summarized coefficients of the interpolation polynomial
    std::vector< double > interpolation_coefficients;

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double I_stim_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    /** initial value to normalise excitatory synaptic current */
    double PSCurrInit_E_;

    /** initial value to normalise inhibitory synaptic current */
    double PSCurrInit_I_;

    int RefractoryCounts_;
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< hh_psc_alpha_gap > recordablesMap_;
};

inline void
hh_psc_alpha_gap::update( Time const& origin, const long from, const long to )
{
  update_( origin, from, to, false );
}

inline bool
hh_psc_alpha_gap::wfr_update( Time const& origin, const long from, const long to )
{
  State_ old_state = S_; // save state before wfr_update
  const bool wfr_tol_exceeded = update_( origin, from, to, true );
  S_ = old_state; // restore old state

  return not wfr_tol_exceeded;
}

inline port
hh_psc_alpha_gap::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent se;
  se.set_sender( *this );
  return target.handles_test_event( se, receptor_type );
}


inline port
hh_psc_alpha_gap::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
hh_psc_alpha_gap::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
hh_psc_alpha_gap::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline port
hh_psc_alpha_gap::handles_test_event( GapJunctionEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline void
hh_psc_alpha_gap::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
hh_psc_alpha_gap::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d, this );   // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d, this );   // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

} // namespace

#endif // HAVE_GSL
#endif // HH_PSC_ALPHA_GAP_H
