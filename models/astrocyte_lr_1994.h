/*
 *  astrocyte_lr_1994.h
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

#ifndef ASTROCYTE_LR_1994_H
#define ASTROCYTE_LR_1994_H

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
extern "C" int astrocyte_lr_1994_dynamics( double, const double*, double*, void* );

/* BeginUserDocs: astrocyte

Short description
+++++++++++++++++

A model of astrocyte with dynamics of three variables

Description
+++++++++++

``astrocyte_lr_1994`` is a model of astrocyte. It can be connected with neuron
models to study neuron-astrocyte interactions. The model defines dynamics of the
following variables:

====== ======== ==============================================================
IP3    uM       Inositol trisphosphate concentration in the astrocytic cytosol
Ca     uM       Calcium concentration in the astrocytic cytosol
h_IP3R unitless The fraction of active IP3 receptors on the astrocytic ER
====== ======== ==============================================================

``astrocyte_lr_1994`` is implemented according to the original model of
astrocyte dynamics by Li & Rinzel (1994) [1] and the derived model by Nadkarni &
Jung (2003) [2], where mechanisms for astrocytic IP3 generation and
calcium-depedent output are added. ``astrocyte_lr_1994`` implements all
equations in [2], except modifying the IP3 generation mechanism:

.. math::

 \frac{dIP3}{dt} =
 \frac{IP3_0 - IP3}{\tau_{IP3}} + incr_{IP3}J_{syn}(t)

Here :math:`J_{syn}(t)` is the summed synaptic weight received by the astrocyte
at time :math:`t`.

By default, the astrocytic output, the slow inward current (SIC), is determined
according to equation 9 in [2]:

.. math::

 I_{SIC} = SIC_{scale}\Theta \left( ln(y) \right) ln(y),\:y = Ca/nM - 196.69

Here :math:`I_{SIC}` is the SIC output, and :math:`\Theta(x)` is the Heaviside
function.

The output is implemented as ``SICEvent`` sent from the astrocyte to its targets
through the ``sic_connection``. The ``SICEvent`` models a continuous current
from the astrocyte to its targets. The weight of ``sic_connection`` can be used
to scale the SIC specific to the connection.

For implementation details, see the
`astrocyte_model_implementation <../model_details/astrocyte_model_implementation.ipynb>`_ notebook.

See also [1]_, [2]_, [3]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

======  ========= ==============================================================
**Dynamic state variables**
--------------------------------------------------------------------------------
IP3     uM        Inositol trisphosphate concentration in the astrocytic cytosol
Ca      uM        Calcium concentration in the astrocytic cytosol
h_IP3R  unitless  The fraction of active IP3 receptors on the astrocytic ER
======  ========= ==============================================================

=============== ========= ========================================================================================================
**Parameters**
----------------------------------------------------------------------------------------------------------------------------------
Ca_tot          uM        Total free astrocytic calcium concentration
IP3_0           uM        Baseline value of the astrocytic IP3 concentration
Kd_act          uM        Astrocytic IP3R dissociation constant of calcium (activation)
Kd_inh          uM        Astrocytic IP3R dissociation constant of calcium (inhibition)
Kd_IP3_1        uM        First astrocytic IP3R dissociation constant of IP3
Kd_IP3_2        uM        Second astrocytic IP3R dissociation constant of IP3
Km_SERCA        uM        Half-activation constant of astrocytic SERCA pump
ratio_ER_cyt    unitless  Ratio between astrocytic ER and cytosol volumes
incr_IP3        uM        Step increase in IP3 concentration with each unit synaptic weight received by the astrocyte
k_IP3R          1/(uM*ms) Astrocytic IP3R binding constant for calcium inhibition
logarithmic_SIC boolean   Use logarithmic SIC output
rate_L          1/ms      Rate constant for calcium leak from the astrocytic ER to cytosol
SIC_scale       unitless  Scale of SIC output
SIC_th          nM        Threshold that determines the minimal level of intracellular astrocytic calcium sufficient to induce SIC
rate_IP3R       1/ms      Maximum rate of calcium release via astrocytic IP3R
rate_SERCA      uM/ms     Maximum rate of calcium uptake by astrocytic SERCA pump
tau_IP3         ms        Time constant of astrocytic IP3 degradation
=============== ========= ========================================================================================================

References
++++++++++

.. [1] Li, Y. X., & Rinzel, J. (1994). Equations for InsP3 receptor-mediated
       [Ca2+]i oscillations derived from a detailed kinetic model: a
       Hodgkin-Huxley like formalism. Journal of theoretical Biology, 166(4),
       461-473.
.. [2] Nadkarni S, and Jung P. Spontaneous oscillations of dressed neurons: A
       new mechanism for epilepsy? Physical Review Letters, 91:26. DOI:
       10.1103/PhysRevLett.91.268101
.. [3] Hahne J, Helias M, Kunkel S, Igarashi J, Bolten M, Frommer A, Diesmann M
       (2015). A unified framework for spiking and gap-junction interactions
       in distributed neuronal netowrk simulations. Frontiers in
       Neuroinformatics, 9:22. DOI: https://doi.org/10.3389/fninf.2015.00022

Sends
+++++

SICEvent

Receives
++++++++

SpikeEvent, DataLoggingRequest

See also
++++++++

aeif_cond_alpha_astro, sic_connection, hh_psc_alpha_gap

EndUserDocs */

class astrocyte_lr_1994 : public ArchivingNode
{

public:
  typedef Node base;

  astrocyte_lr_1994();
  astrocyte_lr_1994( const astrocyte_lr_1994& );
  ~astrocyte_lr_1994() override;

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::sends_secondary_event;

  size_t send_test_event( Node& target, size_t receptor_type, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void
  sends_secondary_event( SICEvent& ) override
  {
  }

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  // void init_state_( const Node& proto );
  void init_buffers_() override;
  void pre_run_hook() override;

  /** This is the actual update function. The additional boolean parameter
   * determines if the function is called by update (false) or wfr_update (true)
   */
  bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long ) override;

  // END Boilerplate function declarations ----------------------------

  // Friends --------------------------------------------------------

  // make dynamics function quasi-member
  friend int astrocyte_lr_1994_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friend to access the State_ class/member
  friend class RecordablesMap< astrocyte_lr_1994 >;
  friend class UniversalDataLogger< astrocyte_lr_1994 >;

private:
  // ----------------------------------------------------------------

  //! Independent parameters
  struct Parameters_
  {

    double Ca_tot_;       //!< Total free astrocytic calcium concentration in uM
    double IP3_0_;        //!< Baseline value of the astrocytic IP3 concentration in uM
    double Kd_IP3_1_;     //!< First astrocytic IP3R dissociation constant of IP3 in uM
    double Kd_IP3_2_;     //!< Second astrocytic IP3R dissociation constant of IP3 in uM
    double Km_SERCA_;     //!< Activation constant of astrocytic SERCA pump in uM
    double Kd_act_;       //!< Astrocytic IP3R dissociation constant of calcium (activation) in uM
    double Kd_inh_;       //!< Astrocytic IP3R dissociation constant of calcium (inhibition) in uM
    double ratio_ER_cyt_; //!< Ratio between astrocytic ER and cytosol volumes
    double incr_IP3_;     //!< Step increase in IP3 concentration with each unit synaptic weight received by the astrocyte in uM
    double k_IP3R_;       //!< Astrocytic IP3R binding constant for calcium in 1/(uM*ms)
    bool logarithmic_SIC_; //!< Use logarithmic SIC if true
    double rate_L_;       //!< Rate constant for calcium leak from the astrocytic ER to cytosol in 1/ms
    double SIC_scale_;    //!< Scale of SIC output
    double SIC_th_;       //!< Calcium threshold for producing SIC in nM
    double tau_IP3_;      //!< Time constant of astrocytic IP3 degradation in ms
    double rate_IP3R_;    //!< Maximum rate of calcium release via astrocytic IP3R in 1/ms
    double rate_SERCA_;   //!< Maximum rate of calcium uptake by astrocytic IP3R in uM/ms

    // For alpha-shaped SIC; experimental
    bool alpha_SIC_;
    double tau_SIC_;
    double delay_SIC_;
    double SIC_reactivate_th_;
    double SIC_reactivate_time_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary
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
      IP3 = 0,
      Ca,     // 1
      h_IP3R, // 2
      SIC,
      DSIC,
      STATE_VEC_SIZE
    };

    //! neuron state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );
    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

  // ----------------------------------------------------------------

private:
  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( astrocyte_lr_1994& ); //!< Sets buffer pointers to 0
    //! Sets buffer pointers to 0
    Buffers_( const Buffers_&, astrocyte_lr_1994& );

    //! Logger for all analog data
    UniversalDataLogger< astrocyte_lr_1994 > logger_;

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

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double I_stim_;

    // values to be sent by SIC event
    std::vector< double > sic_values;

    // for alpha-shaped SIC
    // switch between SIC-on (activated) and SIC-off (reactivated) states
    bool sic_on_;
    double sic_on_timer_;
    double i0_ex_;
    bool sic_started_flag_;
    double sic_off_timer_;
  };

  // ----------------------------------------------------------------

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
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< astrocyte_lr_1994 > recordablesMap_;
};

inline size_t
astrocyte_lr_1994::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent se;
  se.set_sender( *this );
  return target.handles_test_event( se, receptor_type );
}


inline size_t
astrocyte_lr_1994::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
astrocyte_lr_1994::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
astrocyte_lr_1994::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
astrocyte_lr_1994::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  ArchivingNode::get_status( d );

  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
astrocyte_lr_1994::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;     // temporary copy in case of errors
  ptmp.set( d, this );       // throws if BadProperty
  State_ stmp = S_;          // temporary copy in case of errors
  stmp.set( d, ptmp, this ); // throws if BadProperty

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
#endif // ASTROCYTE_LR_1994_H
