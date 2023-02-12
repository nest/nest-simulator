/*
 *  astrocyte.h
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

#ifndef ASTROCYTE_H
#define ASTROCYTE_H

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
#include "recordables_map.h"
#include "ring_buffer.h"
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
extern "C" int astrocyte_dynamics( double, const double*, double*, void* );

/* BeginUserDocs: astrocyte, current-based

Short description
+++++++++++++++++

A model of astrocyte with dynamics of three variables: IP3_astro, Ca_astro,
f_IP3R_astro

Description
+++++++++++

``astrocyte`` is a model of astrocyte. The model defines dynamics of the
following variables:

============  ========   ==========================================================
IP3_astro     uM         IP3 concentration in the astrocytic cytosol
Ca_astro      uM         Calcium concentration in the astrocytic cytosol
f_IP3_astro   unitless   The fraction of active IP3 receptors on the astrocytic ER
============  ========   ==========================================================

The model is developed by adapting a Hodgkin-Huxley neuron model
(``hh_psc_alpha_gap``). It can be connected to a presynaptic neuron with a
``tsodyks_synapse``, and to a postsynaptic neuron with a ``sic_connection``.

``astrocyte`` receives presynaptic input through the ``tsodyks_synapse``, which
determines its dynamics. The dynamics then determine the current being sent to
the postsynaptic neuron through the ``sic_connection``. The connections should
be created with the connectivity rule ``pairwise_bernoulli_astro``.

Presynaptic release of glutamate affects the dynamics according to the model
described in Nadkarni & Jung (2003) [1].

Spikes do not exist in the astrocyte model. However, the spiking-related
mechanisms were not removed from the code to avoid incompatibilities with the
rest of the simulator.

See also [1]_, [2]_, [3]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

==============  ========= ==================================================================
Ca_tot_astro    uM        Total free astrocytic calcium concentration
IP3_0_astro     uM        Baseline value of the astrocytic IP3 concentration
K_act_astro     uM        Astrocytic IP3R dissociation constant of calcium (activation)
K_inh_astro     uM        Astrocytic IP3R dissociation constant of calcium (inhibition)
K_IP3_1_astro   uM        Astrocytic IP3R dissociation constant of IP3
K_IP3_2_astro   uM        Astrocytic IP3R dissociation constant of IP3
K_SERCA_astro   uM        Activation constant of astrocytic SERCA pump
r_ER_cyt_astro  unitless  Ratio between astrocytic ER and cytosol volumes
r_IP3_astro     uM/w      Rate constant of astrocytic IP3 production
r_IP3R_astro    1/(uM*ms) Astrocytic IP3R binding constant for calcium inhibition
r_L_astro       1/ms      Rate constant for calcium leak from the astrocytic ER to cytosol
SIC_thr_astro   nM        Calcium threshold for producing SIC
v_IP3R_astro    1/ms      Maximum rate of calcium release via astrocytic IP3R
v_SERCA_astro   uM/ms     Maximum rate of calcium uptake by astrocytic IP3R
tau_IP3_astro   ms        Time constant of astrocytic IP3 degradation
==============  ========= ==================================================================

References
++++++++++

.. [1] Nadkarni S, and Jung P. Spontaneous oscillations of dressed neurons: A
       new mechanism for epilepsy? Physical Review Letters, 91:26. DOI:
       10.1103/PhysRevLett.91.268101
.. [2] Li, Y. X., & Rinzel, J. (1994). Equations for InsP3 receptor-mediated
       [Ca2+]i oscillations derived from a detailed kinetic model: a
       Hodgkin-Huxley like formalism. Journal of theoretical Biology, 166(4),
       461-473.
.. [3] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann M
       (2015). A unified framework for spiking and gap-junction interactions
       in distributed neuronal netowrk simulations. Frontiers in
       Neuroinformatics, 9:22. DOI: https://doi.org/10.3389/fninf.2015.00022

Sends
+++++

SICEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

aeif_cond_alpha_astro, sic_connection, hh_psc_alpha_gap

EndUserDocs */

class astrocyte : public ArchivingNode
{

public:
  typedef Node base;

  astrocyte();
  astrocyte( const astrocyte& );
  ~astrocyte() override;

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_secondary_event;

  port send_test_event( Node& target, rport receptor_type, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;
  void handle( GapJunctionEvent& ) override;

  port handles_test_event( SpikeEvent&, rport ) override;
  port handles_test_event( CurrentEvent&, rport ) override;
  port handles_test_event( DataLoggingRequest&, rport ) override;
  port handles_test_event( GapJunctionEvent&, rport ) override;

  void
  sends_secondary_event( GapJunctionEvent& ) override
  {
  }

  void
  sends_secondary_event( SICEvent& ) override
  {
  }

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_state_( const Node& proto );
  void init_buffers_() override;
  void pre_run_hook() override;

  /** This is the actual update function. The additional boolean parameter
   * determines if the function is called by update (false) or wfr_update (true)
   */
  bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long ) override;
  // bool wfr_update( Time const&, const long, const long ) override;

  // END Boilerplate function declarations ----------------------------

  // Friends --------------------------------------------------------

  // make dynamics function quasi-member
  friend int astrocyte_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friend to access the State_ class/member
  friend class RecordablesMap< astrocyte >;
  friend class UniversalDataLogger< astrocyte >;

private:
  // ----------------------------------------------------------------

  //! Independent parameters
  struct Parameters_
  {

    double Ca_tot_astro_;   //!< Total free astrocytic calcium concentration in uM
    double IP3_0_astro_;    //!< Baseline value of the astrocytic IP3 concentration in uM
    double K_IP3_1_astro_;  //!< Astrocytic IP3R dissociation constant of IP3 in uM
    double K_IP3_2_astro_;  //!< Astrocytic IP3R dissociation constant of IP3 in uM
    double K_SERCA_astro_;  //!< Activation constant of astrocytic SERCA pump in uM
    double K_act_astro_;    //!< Astrocytic IP3R dissociation constant of calcium (activation) in uM
    double K_inh_astro_;    //!< Astrocytic IP3R dissociation constant of calcium (inhibition) in uM
    double r_ER_cyt_astro_; //!< Ratio between astrocytic ER and cytosol volumes
    double r_IP3_astro_;    //!< Rate constant of astrocytic IP3 production in uM/w
    double r_IP3R_astro_;   //!< Astrocytic IP3R binding constant for calcium in 1/(uM*ms)
    double r_L_astro_;      //!< Rate constant for calcium leak from the astrocytic ER to cytosol in 1/ms
    double SIC_thr_astro_;  //!< Calcium threshold for producing SIC in nM
    double tau_IP3_astro_;  //!< Time constant of astrocytic IP3 degradation in ms
    double v_IP3R_astro_;   //!< Maximum rate of calcium release via astrocytic IP3R in 1/ms
    double v_SERCA_astro_;  //!< Maximum rate of calcium uptake by astrocytic IP3R in uM/ms

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
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
      IP3_astro = 0,
      Ca_astro,     // 1
      f_IP3R_astro, // 2
      STATE_VEC_SIZE
    };

    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum& );
  };

  // ----------------------------------------------------------------

private:
  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( astrocyte& ); //!< Sets buffer pointers to 0
    //! Sets buffer pointers to 0
    Buffers_( const Buffers_&, astrocyte& );

    //! Logger for all analog data
    UniversalDataLogger< astrocyte > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spike_exc_;
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
   * TO DO: These are not needed for the astrocyte model. Test whether they can be removed.
   */
  // struct Variables_
  // {
  //   /** initial value to normalise excitatory synaptic current */
  //   double PSCurrInit_E_;

  //   /** initial value to normalise inhibitory synaptic current */
  //   double PSCurrInit_I_;

  //   int RefractoryCounts_;
  // };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out state vector elements, used by UniversalDataLogger
  template < State_::StateVecElems elem >
  double
  get_y_elem_() const
  {
    return S_.y_[ elem ];
  }

  //! Read out SIC values; for testing, to be deleted
  double sic_ = 0.0;
  double
  get_sic_() const
  {
    return sic_;
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  // Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< astrocyte > recordablesMap_;
};

// inline void
// astrocyte::update( Time const& origin, const long from, const long to )
// {
//   update_( origin, from, to, false );
// }

// inline bool
// astrocyte::wfr_update( Time const& origin, const long from, const long to )
// {
//   State_ old_state = S_; // save state before wfr_update
//   const bool wfr_tol_exceeded = update_( origin, from, to, true );
//   S_ = old_state; // restore old state
//
//   return not wfr_tol_exceeded;
// }

inline port
astrocyte::send_test_event( Node& target, rport receptor_type, synindex, bool )
{
  SpikeEvent se;
  se.set_sender( *this );
  return target.handles_test_event( se, receptor_type );
}


inline port
astrocyte::handles_test_event( SpikeEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
astrocyte::handles_test_event( CurrentEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline port
astrocyte::handles_test_event( DataLoggingRequest& dlr, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline port
astrocyte::handles_test_event( GapJunctionEvent&, rport receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline void
astrocyte::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
astrocyte::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.set( d );         // throws if BadProperty
  State_ stmp = S_;      // temporary copy in case of errors
  stmp.set( d );         // throws if BadProperty

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
#endif // ASTROCYTE_H
