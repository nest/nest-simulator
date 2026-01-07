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
#include "universal_data_logger_impl.h"

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
first proposed by Li & Rinzel (1994) [1]_ and it is based on earlier work of De
Young & Kaiser (1992) [2]_. The input and output of the model are implemented
according to Nadkarni & Jung (2003) [3]_.

The model has three dynamic state variables: the concentration of inositol
1,4,5-trisphosphate in the astrocyte (:math:`\mathrm{[IP3]}`), the calcium
concentration in the astrocytic cytosol (:math:`\mathrm{[Ca^{2+}]}`), and the
fraction of IP3 receptors on the astrocytic endoplasmic reticulum (ER) that are
not yet inactivated by calcium (:math:`h_\mathrm{IP3R}`).

In this model, excitatory synaptic inputs to the astrocyte trigger IP3
generation and change in calcium dynamics in the astrocyte. This might induce an
astrocytic output in the form of a slow inward current (SIC), which is dependent
on its calcium dynamics, to its target neurons. The input and output are based
on the equations in [3]_ but with adaptations, as described in the following.

Spike input
-----------

The astrocyte receives inputs from excitatory (glutamatergic)
synapses. The synaptic inputs directly affect IP3 concentration according to the
following equation:

.. math::

 \frac{d[\mathrm{IP3}]}{dt} =
 \frac{[\mathrm{IP3}]_0 - [\mathrm{IP3}]}{\tau_\mathrm{IP3}} + \Delta_\mathrm{IP3} \cdot J_\mathrm{syn}(t)

In the absence of inputs, :math:`\mathrm{[IP3]}` decays to its baseline value
(:math:`[\mathrm{IP3}]_0`) with the time constant (:math:`\tau_\mathrm{IP3}`). Each time when an
astrocyte receives an excitatory synaptic input, it triggers an instantaneous
increase of :math:`\mathrm{[IP3]}`. In this implementation, the inputs are spike events
sent from neurons or generators. The summed synaptic weight the astrocyte receives at time
:math:`t` is given by :math:`J_\mathrm{syn}(t)`. The parameter
:math:`\Delta_\mathrm{IP3}` scales the impact of synaptic inputs on the
IP3 dynamics.

Calcium current input
---------------------

In this implementation, a current input to the astrocyte is directly
added to its cytosolic calcium concentration. Generators that send out currents
can be connected to astrocytes to directly generate fluctuations in cytosolic
calcium:

.. math::

 \frac{d[\mathrm{Ca^{2+}}]}{dt} =
 J_\mathrm{channel} - J_\mathrm{pump} + J_\mathrm{leak} + J_\mathrm{noise}

Here, :math:`\mathrm{[Ca^{2+}]}` is the cytosolic calcium concentration, and
:math:`J_\mathrm{noise}` is the current input. :math:`J_\mathrm{channel}`,
:math:`J_\mathrm{pump}`, :math:`J_\mathrm{leak}` are the calcium fluxes defined
as in [3]_.

Output
------

If the astrocyte receives excitatory synaptic inputs, it might
output SIC to its target neurons. This current depends on the cytosolic
calcium concentration. This dependency is modeled according to the expressions
first proposed in [3]:

.. math::

 I_\mathrm{SIC} =  \mathrm{SIC_{scale}} \cdot \mathrm{H}\left(\mathrm{ln}(y)\right) \cdot \mathrm{ln}(y)

where

.. math::

 y = \left( \mathrm{[Ca^{2+}]} - \mathrm{SIC_{th}} \right)/\mathrm{nM}

When the cytosolic calcium concentration of the astrocyte exceeds the threshold
value (:math:`\mathrm{SIC_{th}}`), a SIC output (:math:`I_\mathrm{SIC}`) is
generated. This thresholding is modeled as a Heaviside function
(:math:`\mathrm{H(\cdot)}`). In this implementation, the SIC threshold
:math:`\mathrm{SIC_{th}}` as well as the scaling constant
:math:`\mathrm{SIC_{scale}}` are treated as model parameters that can be set
together with other parameters. Nadkarni & Jung (2003) [3]_ proposed values for
these parameters by fitting the equation for SIC to an experimental data set.

The output is implemented as SICEvent sent from the astrocyte to its target
neurons through the ``sic_connection``.

For the reference implementation of this model, see the
`astrocyte_model_implementation <../model_details/astrocyte_model_implementation.ipynb>`_ notebook.

See also [1]_, [2]_, [3]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

======== ========= =============================================================
**Dynamic state variables**
--------------------------------------------------------------------------------
IP3      µM        Inositol 1,4,5-trisphosphate concentration in the astrocytic
                   cytosol
Ca_astro µM        Calcium concentration in the astrocytic cytosol
h_IP3R   unitless  Fraction of IP3 receptors on the astrocytic ER that are not
                   yet inactivated by calcium
======== ========= =============================================================

=============== ========= =====================================================
**Parameters**
-------------------------------------------------------------------------------
Ca_tot          µM        Total free astrocytic calcium concentration in terms
                          of cytosolic volume
IP3_0           µM        Baseline value of astrocytic IP3 concentration
Kd_IP3_1        µM        First astrocytic IP3R dissociation constant of IP3
Kd_IP3_2        µM        Second astrocytic IP3R dissociation constant of IP3
Kd_act          µM        Astrocytic IP3R dissociation constant of calcium
                          (activation)
Kd_inh          µM        Astrocytic IP3R dissociation constant of calcium
                          (inhibition)
Km_SERCA        µM        Half-activation constant of astrocytic SERCA pump
SIC_scale       unitless  Parameter determining the scale of astrocytic SIC
                          output
SIC_th          µM        Threshold that determines the minimal level of
                          astrocytic cytosolic calcium sufficient to induce
                          SIC
delta_IP3       µM        Parameter determining the increase in astrocytic IP3
                          concentration induced by synaptic input
k_IP3R          1/(µM*ms) Astrocytic IP3R binding constant for calcium
                          inhibition
rate_IP3R       1/ms      Maximum rate of calcium release via astrocytic IP3R
rate_L          1/ms      Rate constant of calcium leak from astrocytic ER to
                          cytosol
rate_SERCA      µM/ms     Maximum rate of calcium uptake by astrocytic SERCA
                          pump
ratio_ER_cyt    unitless  Ratio between astrocytic ER and cytosol volumes
tau_IP3         ms        Time constant of the exponential decay of astrocytic
                          IP3
=============== ========= =====================================================

References
++++++++++

.. [1] Li, Y. X., & Rinzel, J. (1994). Equations for InsP3 receptor-mediated
       [Ca2+]i oscillations derived from a detailed kinetic model: a
       Hodgkin-Huxley like formalism. Journal of theoretical Biology, 166(4),
       461-473. DOI: https://doi.org/10.1006/jtbi.1994.1041

.. [2] De Young, G. W., & Keizer, J. (1992). A single-pool inositol
       1,4,5-trisphosphate-receptor-based model for agonist-stimulated
       oscillations in Ca2+ concentration. Proceedings of the National Academy
       of Sciences, 89(20), 9895-9899. DOI:
       https://doi.org/10.1073/pnas.89.20.9895

.. [3] Nadkarni, S., & Jung, P. (2003). Spontaneous oscillations of dressed
       neurons: a new mechanism for epilepsy?. Physical review letters, 91(26),
       268101. DOI: https://doi.org/10.1103/PhysRevLett.91.268101

Sends
+++++

SICEvent

Receives
++++++++

SpikeEvent, DataLoggingRequest

See also
++++++++

aeif_cond_alpha_astro, sic_connection

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: astrocyte_lr_1994

EndUserDocs */

void register_astrocyte_lr_1994( const std::string& name );

class astrocyte_lr_1994 : public Node
{

public:
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
  void init_buffers_() override;
  void pre_run_hook() override;

  /** This is the actual update function. The additional boolean parameter
   * determines if the function is called by update (false) or wfr_update (true)
   */
  // bool update_( Time const&, const long, const long, const bool );

  void update( Time const&, const long, const long ) override;

  // END Boilerplate function declarations ----------------------------

  // Friends --------------------------------------------------------

  // make dynamics function quasi-member
  friend int astrocyte_lr_1994_dynamics( double, const double*, double*, void* );

  // The next two classes need to be friend to access the State_ class/member
  friend class RecordablesMap< astrocyte_lr_1994 >;
  friend class UniversalDataLogger< astrocyte_lr_1994 >;

  // ----------------------------------------------------------------

  //! Model parameters
  struct Parameters_
  {
    // parameters based on Nadkarni & Jung (2003)
    double Ca_tot_;    //!< Total free astrocytic calcium concentration in terms of cytosolic volume in µM
    double IP3_0_;     //!< Baseline value of the astrocytic IP3 concentration in µM
    double Kd_IP3_1_;  //!< First astrocytic IP3R dissociation constant of IP3 in µM
    double Kd_IP3_2_;  //!< Second astrocytic IP3R dissociation constant of IP3 in µM
    double Kd_act_;    //!< Astrocytic IP3R dissociation constant of calcium (activation) in µM
    double Kd_inh_;    //!< Astrocytic IP3R dissociation constant of calcium (inhibition) in µM
    double Km_SERCA_;  //!< Half-activation constant of astrocytic SERCA pump in µM
    double SIC_scale_; //!< Parameter determining the scale of astrocytic SIC output
    double SIC_th_; //!< Threshold that determines the minimal level of intracellular astrocytic calcium sufficient to
                    //!< induce SIC in µM
    double delta_IP3_; //!< Parameter determining the increase in astrocytic IP3 concentration induced by synaptic input
                       //!< in µM
    double k_IP3R_;    //!< Astrocytic IP3R binding constant for calcium inhibition in 1/(µM*ms)
    double rate_IP3R_; //!< Maximum rate of calcium release via astrocytic IP3R in 1/ms
    double rate_L_;    //!< Rate constant of calcium leak from astrocytic ER to cytosol in 1/ms
    double rate_SERCA_;   //!< Maximum rate of calcium uptake by astrocytic SERCA pump in µM/ms
    double ratio_ER_cyt_; //!< Ratio between astrocytic ER and cytosol volumes
    double tau_IP3_;      //!< Time constant of the exponential decay of astrocytic IP3 in ms

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
      IP3 = 0,
      Ca_astro, // 1
      h_IP3R,   // 2
      STATE_VEC_SIZE
    };

    //! cell state, must be C-array for GSL solver
    double y_[ STATE_VEC_SIZE ];

    State_( const Parameters_& ); //!< Default initialization
    State_( const State_& );

    State_& operator=( const State_& );

    void get( DictionaryDatum& ) const;
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    Buffers_( astrocyte_lr_1994& );                  //!< Sets buffer pointers to 0
    Buffers_( const Buffers_&, astrocyte_lr_1994& ); //!< Sets buffer pointers to 0

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

    /**
     * Input current injected by CurrentEvent.
     * This variable is used to transport the current applied into the
     * _dynamics function computing the derivative of the state vector.
     * It must be a part of Buffers_, since it is initialized once before
     * the first simulation, but not modified before later Simulate calls.
     */
    double J_noise_;

    // values to be sent by SIC event
    std::vector< double > sic_values;
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

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< astrocyte_lr_1994 > recordablesMap_;
};


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

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;
}

template <>
void RecordablesMap< astrocyte_lr_1994 >::create();

} // namespace

#endif // HAVE_GSL
#endif // ASTROCYTE_LR_1994_H
