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
extern "C" int astrocyte_dynamics( double, const double*, double*, void* );

/** @BeginDocumentation
@ingroup Neurons
@ingroup psc
@ingroup hh
@ingroup gap

Name: 
astrocyte - contains variables IP3_astro, Ca_astro, f_IP3R_astro

Description: 
The model defines dynamics of the following variables: 
============  ========   ========================================================== 
IP3_astro     uM         IP3 concentration in the astrocytic cytosol 
Ca_astro      uM         Calcium concentration in the astrocytic cytosol
f_IP3_astro   unitless   The fraction of active IP3 receptors on the astrocytic ER
============  ========   ==========================================================  

The model is developed using the template for Hodgkin-Huxley neuron model with gap-junction support
(hh_psc_alpha) however, the spiking mechanism implemented in this model is not needed here.

1. Post-synaptic currents:
Synaptic release of glutamate affects IP3 production according to the model described in 
Nadkarni & Jung (2003). The percent of released glutamate (model parameter) binds to the astrocyte while the rest 
binds to the postsynaptic neuron. 

2. Spikes: Spikes do not exhist in the astrocyte model, however the spiking-related mechanisms were not removed from
the code to avoid incmpatibilities with the rest of the simulator. 

3. Gap Junctions: Used to implement the interaction from astrocyte to neuron in the form of Slow Inward Current (SIC). 
Gap Junctions are implemented by a gap current of the form
\f$ g_ij( V_i - V_j) \f$ 


Parameters:
The following parameters can be set in the status dictionary.

\verbatim embed:rst
==============  ======    ==================================================================
Ca_tot_astro    uM        Total free astrocytic calcium concntration
IP3_0_astro     uM        Baseline value of the astrocytic IP3 concentration
K_act_astro     uM        Astrocytic IP3R dissociation constant of calcium (activation)
K_inh_astro     uM        Astrocytic IP3R dissociation constant of calcium (inhibition)
K_IP3_1_astro   uM        Astrocytic IP3R dissociation constant of IP3
K_IP3_2_astro   uM        Astrocytic IP3R dissociation constant of IP3
K_SERCA_astro   uM        Activation constant of astrocytic SERCA pump 
r_ER_cyt_astro  unitless  Ratio between astrocytic ER and cytosol volumes
r_IP3_astro     uM/ms     Rate constant of astrocytic IP3 production
r_IP3R_astro    1/(uM*ms) Astrocytic IP3R binding constant for calcium inhibition
r_L_astro       1/ms      Rate constant for calcium leak from the astrocytic ER to cytosol 
v_IP3R_astro    1/ms      Maximum rate of calcium release via astrocytic IP3R 
v_SERCA_astro   uM/ms     Maximum rate of calcium uptake by astrocytic IP3R 
tau_IP3_astro   ms        Time constant of astrocytic IP3 degradation
==============  ======    ==================================================================
\endverbatim

References:

\verbatim embed:rst
.. [1] Nadkarni S, and Jung P. Spontaneous oscillations of dressed neurons: A 
	new mechanism for epilepsy? Physical Review Letters, 91:26. DOI: 10.1103/PhysRevLett.91.268101 
.. [2] TO DO: add the paper on network modeling	
.. [3] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann M
       (2015). A unified framework for spiking and gap-junction interactions
       in distributed neuronal netowrk simulations. Frontiers in
       Neuroinformatics, 9:22. DOI: https://doi.org/10.3389/fninf.2015.00022
\endverbatim


Sends: SpikeEvent, GapJunctionEvent

Receives: SpikeEvent, GapJunctionEvent, CurrentEvent, DataLoggingRequest

Authors: Jan Hahne, Jonas Stapmanns, Mikko Lehtimaki, Jugoslava Acimovic TO DO: add other authors  

SeeAlso: hh_psc_alpha, hh_cond_exp_traub, gap_junction
*/
class astrocyte : public ArchivingNode
{

public:
  typedef Node base;

  astrocyte();
  astrocyte( const astrocyte& );
  ~astrocyte();

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
  
  void
  sends_secondary_event( SICEvent& )
  {
  }

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  void init_state_( const Node& proto );
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
  friend int astrocyte_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friend to access the State_ class/member
  friend class RecordablesMap< astrocyte >;
  friend class UniversalDataLogger< astrocyte >;

private:
  // ----------------------------------------------------------------

  //! Independent parameters
  struct Parameters_
  {

    double Ca_tot_astro_;
    double IP3_0_astro_;
    double K_act_astro_;
    double K_inh_astro_;
    double K_IP3_1_astro_;
    double K_IP3_2_astro_;
    double K_SERCA_astro_; 
    double r_ER_cyt_astro_; 
    double r_IP3_astro_; 
    double r_IP3R_astro_;
    double r_L_astro_;
    double v_IP3R_astro_;
    double v_SERCA_astro_;
    double tau_IP3_astro_;
    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
    void set( const DictionaryDatum& ); //!< Set values from dicitonary
  };

public:
  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   * @note Copy constructor and assignment operator required because
   *       of C-style array.
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
      Ca_astro,   // 1
      f_IP3R_astro,   // 2
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
    Buffers_( astrocyte& ); //!<Sets buffer pointers to 0
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

    // IntergrationStep_ should be reset with the neuron on ResetNetwork,
    // but remain unchanged during calibration. Since it is initialized with
    // step_, and the resolution cannot change after nodes have been created,
    // it is safe to place both here.
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
  static RecordablesMap< astrocyte > recordablesMap_;
};

inline void
astrocyte::update( Time const& origin, const long from, const long to )
{
  update_( origin, from, to, false );
}

inline bool
astrocyte::wfr_update( Time const& origin, const long from, const long to )
{
  State_ old_state = S_; // save state before wfr_update
  const bool wfr_tol_exceeded = update_( origin, from, to, true );
  S_ = old_state; // restore old state

  return not wfr_tol_exceeded;
}

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
