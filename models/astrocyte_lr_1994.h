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

An astrocyte model based on Li & Rinzel (1994)

Description
+++++++++++

``astrocyte_lr_1994`` is a model of astrocytic calcium dynamics. The model was
first proposed by Li & Rinzel (1994) and it is based on earlier work of DeYoung
& Kaiser (1992). The input and output of the model are implemented according to
Nadkarni & Jung (2003).

The model has three dynamic state variables: the concentration of inositol
1,4,5-trisphosphate in the astrocyte (:math:`IP3`), the calcium concentration in
the astrocytic cytosol (:math:`Ca`), and the fraction of active IP3 receptors on
the astrocytic endoplasmatic reticulum (:math:`h_{IP3R}`).

The astrocyte receives excitatory synaptic inputs, which affect its IP3
concentration, and outputs a slow inward current (SIC) dependent on its
cytosolic calcium dynamics to its target neurons. The input and output are based
on the equations in Nadkarni & Jung (2003) but with adaptations:

**Input:** The astrocyte receives inputs from excitatory (glutamatergic)
synapses. The synaptic inputs directly affect IP3 concentration according to the
following equation:

.. math::

 \frac{dIP3}{dt} =
 \frac{IP3_0 - IP3}{\tau_{IP3}} + \Delta_{IP3} \cdot J_{syn}(t)

In the absence of inputs, :math:`IP3` decays to its baseline value
(:math:`IP3_0`) with the time constant (:math:`\tau_{IP3}`). Each time when an
astrocyte receives an excitatory synaptic input, it triggers an instantaneous
increase of :math:`IP3`. In this implementation, the inputs are spike events
sent from neurons. The summed synaptic weight the astrocyte receives at time
:math:`t` is given by :math:`J_{syn}(t)`. The parameter :math:`\Delta_{IP3}`
scales the impact of synaptic inputs on the IP3 dynamics.

In addition, in this implementation a current input to the astrocyte is directly
added to its cytosolic calcium concentration. Generators that send out currents
can be connected to astrocytes to directly generate fluctuations in cytosolic
calcium:

.. math::

 \frac{dCa}{dt} =
 J_{channel} - J_{pump} + J_{leak} + J_{noise}

Here, :math:`Ca` is the cytosolic calcium concentration, and :math:`J_{noise}`
is the current input. :math:`J_{channel}`, :math:`J_{pump}`, :math:`J_{leak}`
are the calcium flux defined as in Nadkarni & Jung (2003), which are affected by
:math:`IP3`.

**Output:** If the astrocyte receives excitatory synaptic inputs, it might
output SIC to the its target neurons. This current depends on the cytosolic
calcium concentration. This dependency is modeled according to the expressions
first proposed in Nadkarni & Jung (2003):

.. math::

 I_{SIC} =  \mbox{SIC}_{scale} \cdot H \left(\mbox{ln}(y)\right) \cdot \mbox{ln}(y)

where

.. math::

 y = Ca/nM - \mbox{SIC}_{th}

When the cytosolic calcium concentration of the astrocyte exceeds the threshold
value (:math:`\mbox{SIC}_{th}`) a SIC output (:math:`I_{SIC}`) is generated.
This thresholding is modeled as a Heaviside function (:math:`H(\cdot)`). In this
implementation, the SIC threshold :math:`\mbox{SIC}_{th}` as well as the scaling
constant :math:`\mbox{SIC}_{scale}` are treated as model parameters that can be
set together with other parameters. Nadkarni & Jung (2003) proposed values for
these parameters by fitting the equation for SIC to an experimental data set.

The output is implemented as SICEvent sent from the astrocyte to its target
neurons through the ``sic_connection``.

For implementation details, see the
`astrocyte_model_implementation <../model_details/astrocyte_model_implementation.ipynb>`_ notebook.

See also [1]_, [2]_, [3]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

======  ========= ====================================================================
**Dynamic state variables**
--------------------------------------------------------------------------------------
IP3     uM        Inositol 1,4,5-trisphosphate concentration in the astrocytic cytosol
Ca      uM        Calcium concentration in the astrocytic cytosol
h_IP3R  unitless  The fraction of active IP3 receptors on the astrocytic ER
======  ========= ====================================================================

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
incr_IP3        uM        Parameter for the rate of IP3 increment induced by synaptic input
k_IP3R          1/(uM*ms) Astrocytic IP3R binding constant for calcium inhibition
rate_L          1/ms      Rate constant for calcium leak from the astrocytic ER to cytosol
rate_IP3R       1/ms      Maximum rate of calcium release via astrocytic IP3R
rate_SERCA      uM/ms     Maximum rate of calcium uptake by astrocytic SERCA pump
tau_IP3         ms        IP3 exponential decay constant
SIC_th          uM        Threshold that determines the minimal level of intracellular astrocytic calcium sufficient to induce SIC
SIC_scale       unitless  Parameter for the scale of SIC output
=============== ========= ========================================================================================================

References
++++++++++

.. [1] De Young, G. W., & Keizer, J. (1992). A single-pool inositol
       1,4,5-trisphosphate-receptor-based model for agonist-stimulated
       oscillations in Ca2+ concentration. Proceedings of the National Academy
       of Sciences, 89(20), 9895-9899. DOI:
       https://doi.org/10.1073/pnas.89.20.9895

.. [2] Li, Y. X., & Rinzel, J. (1994). Equations for InsP3 receptor-mediated
       [Ca2+]i oscillations derived from a detailed kinetic model: a
       Hodgkin-Huxley like formalism. Journal of theoretical Biology, 166(4),
       461-473. DOI: https://doi.org/10.1006/jtbi.1994.1041

.. [3] Nadkarni S, and Jung P. Spontaneous oscillations of dressed neurons: A
       new mechanism for epilepsy? Physical Review Letters, 91:26. DOI:
       https://doi.org/10.1103/PhysRevLett.91.268101

Sends
+++++

SICEvent

Receives
++++++++

SpikeEvent, DataLoggingRequest

See also
++++++++

aeif_cond_alpha_astro, sic_connection

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

  void register_stdp_connection( double t_first_read, double delay ) override;

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
    double rate_L_;       //!< Rate constant for calcium leak from the astrocytic ER to cytosol in 1/ms
    double SIC_th_;       //!< Calcium threshold for producing SIC in uM
    double tau_IP3_;      //!< Time constant of astrocytic IP3 degradation in ms
    double rate_IP3R_;    //!< Maximum rate of calcium release via astrocytic IP3R in 1/ms
    double rate_SERCA_;   //!< Maximum rate of calcium uptake by astrocytic IP3R in uM/ms

    // For SIC; experimental
    double SIC_scale_;     //!< Scale of SIC output
    bool alpha_SIC_;       //!< Use alpha-shaped SIC if true
    double tau_SIC_;       //!< Time constant of alpha-shaped SIC
    double delay_SIC_;     //!< Delay of alpha-shaped SIC
    double SIC_reactivate_th_;   //!< Calcium level for reactivating SIC
    double SIC_reactivate_time_; //!< Time staying SIC_reactivate_th_ required for reactivating SIC

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
