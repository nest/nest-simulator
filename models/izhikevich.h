/*
 *  izhikevich.h
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

#ifndef IZHIKEVICH_H
#define IZHIKEVICH_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger_impl.h"

namespace nest
{
// Disable clang-formatting for documentation due to over-wide table.
// clang-format off
/* BeginUserDocs: neuron, integrate-and-fire, adaptation, soft threshold

Short description
+++++++++++++++++

Izhikevich neuron model

Description
++++++++++++

``izhikevich`` implements the simple spiking neuron model introduced by Izhikevich [1]_.
This model reproduces spiking and bursting behavior of known types of cortical neurons.

Membrane potential evolution, spike emission, and refractoriness
................................................................

The model is defined by the following differential equations:

.. math::

   \frac{dV_{\text{m}}}{dt} = 0.04 V_{\text{m}}^2 + 5 V_{\text{m}} + 140 - U_{\text{m}} + I_{\text{e}}

.. math::

   \frac{dU_{\text{m}}}{dt} = a (b V_{\text{m}} - U_{\text{m}})

where :math:`V_{\text{m}}` is the membrane potential, :math:`U_{\text{m}}` is the recovery variable, and
:math:`I_{\text{e}}` is the input current.

A spike is emitted when :math:`V_{\text{m}}` reaches a threshold :math:`V_{\text{th}}`.
At this point, the membrane potential and recovery variable are updated according to:

.. math::

   &\text{if}\;\ V_m \geq V_{th}:\\
   &\;\ V_m \text{ is set to } c\\
   &\;\ U_m \text{ is incremented by } d\\

In addition, each incoming spike increases :math:`V_{\text{m}}` by the synaptic weight associated with the spike.

As published in [1]_, the numerics differs from the standard forward Euler technique in two ways:

 * the recovery variable :math:`U_{\text{m}}` is updated based on the new value of :math:`V_{\text{m}}`, rather than the previous one.
 * the membrane potential :math:`V_{\text{m}}` is updated with a time step half the size of that used for :math:`U_{\text{m}}`.

This model offers both forms of integration, they can be selected using the boolean parameter ``consistent_integration``:

 * ``consistent_integration = false``: use the published form of the dynamics (for replicating published results).
 * ``consistent_integration = true`` *(default)*: use the standard Euler method (recommended for general use).

.. note::

   For a detailed analysis of the numerical differences between these integration schemes and their impact on simulation results, see [2]_.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

======================= ============ ====================== ===================================================
**Parameter**           **Default**  **Math equivalent**    **Description**
======================= ============ ====================== ===================================================
 V_m                    -65 mV       :math:`V_{\text{m}}`   Membrane potential
 U_m                    -13 mV       :math:`U_{\text{m}}`   Membrane potential recovery variable
 V_th                   30 mV        :math:`V_{\text{th}}`  Spike threshold
 I_e                    0 pA         :math:`I_{\text{e}}`   Constant input current (R=1)
 V_min                  -1.79 mV     :math:`V_{\text{min}}` Absolute lower value for the membrane potential
 a                      0.02 real    :math:`a`              Describes time scale of recovery variable
 b                      0.2 real     :math:`b`              Sensitivity of recovery variable
 c                      -65 mV       :math:`c`              After-spike reset value of V_m
 d                      8 mV         :math:`d`              After-spike reset value of U_m
 consistent_integration ``true``     None                   Use standard integration technique
======================= ============ ====================== ===================================================

References
++++++++++

.. [1] Izhikevich EM. (2003). Simple model of spiking neurons. IEEE Transactions
       on Neural Networks, 14:1569-1572. DOI: https://doi.org/10.1109/TNN.2003.820440

.. [2] Pauli R, Weidel P, Kunkel S, Morrison A (2018). Reproducing polychronization: A guide to maximizing
       the reproducibility of spiking network models. Frontiers in Neuroinformatics, 12.
       DOI: https://www.frontiersin.org/article/10.3389/fninf.2018.00046

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

See also
++++++++

iaf_psc_delta, mat2_psc_exp

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: izhikevich

EndUserDocs */
// clang-format on

void register_izhikevich( const std::string& name );

class izhikevich : public ArchivingNode
{

public:
  izhikevich();
  izhikevich( const izhikevich& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  void handle( DataLoggingRequest& ) override;
  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;

  size_t handles_test_event( DataLoggingRequest&, size_t ) override;
  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  friend class RecordablesMap< izhikevich >;
  friend class UniversalDataLogger< izhikevich >;

  void init_buffers_() override;
  void pre_run_hook() override;

  void update( Time const&, const long, const long ) override;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    double a_;
    double b_;
    double c_;
    double d_;

    /** External DC current */
    double I_e_;

    /** Threshold */
    double V_th_;

    /** Lower bound */
    double V_min_;

    /** Use standard integration numerics **/
    bool consistent_integration_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const;             //!< Store current values in dictionary
    void set( const DictionaryDatum&, Node* node ); //!< Set values from dictionary
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double v_; // membrane potential
    double u_; // membrane recovery variable
    double I_; // input current


    /** Accumulate spikes arriving during refractory period, discounted for
        decay until end of refractory period.
    */

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;
    void set( const DictionaryDatum&, const Parameters_&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {
    /**
     * Buffer for recording
     */
    Buffers_( izhikevich& );
    Buffers_( const Buffers_&, izhikevich& );
    UniversalDataLogger< izhikevich > logger_;

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
  };

  // Access functions for UniversalDataLogger -----------------------

  //! Read out the membrane potential
  double
  get_V_m_() const
  {
    return S_.v_;
  }
  //! Read out the recovery variable
  double
  get_U_m_() const
  {
    return S_.u_;
  }

  // ----------------------------------------------------------------

  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  //! Mapping of recordables names to access functions
  static RecordablesMap< izhikevich > recordablesMap_;
  /** @} */
};

inline size_t
izhikevich::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline size_t
izhikevich::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
izhikevich::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
izhikevich::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
izhikevich::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
izhikevich::set_status( const DictionaryDatum& d )
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

template <>
void RecordablesMap< izhikevich >::create();

} // namespace nest

#endif /* #ifndef IZHIKEVICH_H */
