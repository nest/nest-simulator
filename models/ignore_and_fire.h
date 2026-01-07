/*
 *  ignore_and_fire.h
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

#ifndef IGNORE_AND_FIRE_H
#define IGNORE_AND_FIRE_H

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger_impl.h"

namespace nest
{

/* BeginUserDocs: neuron, integrate-and-fire, current-based

Short description
+++++++++++++++++

Ignore-and-fire neuron model for generating spikes at fixed intervals irrespective of inputs

Description
+++++++++++

The ``ignore_and_fire`` neuron is a neuron model generating spikes at a predefined ``rate`` with a constant inter-spike
interval ("fire"), irrespective of its inputs ("ignore"). In this simplest version of the ``ignore_and_fire`` neuron,
the inputs from other neurons or devices are not processed at all.

The model's state variable, the ``phase``, describes the time to the next spike relative to the firing period (the
inverse of the ``rate``). In each update step, the ``phase`` is decreased by a fixed amount. If it hits zero, a spike
is emitted and the ``phase`` is reset to +1.

To create asynchronous activity for a population of ``ignore_and_fire`` neurons, the firing phases can be randomly
initialized.

The ``ignore_and_fire`` neuron is primarily used for neuronal-network model verification and validation purposes
("benchmarking"), in particular, to evaluate the correctness and performance of connectivity generation and inter-neuron
communication. It permits an easy scaling of the network size and/or connectivity without affecting the output spike
statistics. The amount of network traffic is predefined by the user, and therefore fully controllable and predictable,
irrespective of the network size and structure.

.. note::

   The model can easily be extended and equipped with any arbitrary input processing (such as calculating input
   currents with alpha-function shaped PSC kernels or updating the gating variables in the Hodgkin-Huxley model) or
   (after-) spike generation dynamics to make it more similar and comparable to other non-ignorant neuron models. In
   such extended ignore_and_fire models, the spike emission process would still be decoupled from the intrinsic neuron
   dynamics.

Parameters
++++++++++

The following parameters can be set in the status dictionary.

================= ========= ======================================================
**Parameter**     **Unit**  **Description**
================= ========= ======================================================
 ``phase``                     Phase (relative time to next spike; 0<phase<=1)
 ``rate``          1/s         Firing rate
================= ========= ======================================================

Sends
+++++

SpikeEvent

Receives
++++++++

SpikeEvent, CurrentEvent, DataLoggingRequest

Examples using this model
+++++++++++++++++++++++++

.. listexamples:: ignore_and_fire

EndUserDocs */


void register_ignore_and_fire( const std::string& name );

class ignore_and_fire : public ArchivingNode
{

public:
  ignore_and_fire();
  ignore_and_fire( const ignore_and_fire& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  void get_status( DictionaryDatum& ) const override;
  void set_status( const DictionaryDatum& ) override;

private:
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( Time const&, const long, const long ) override;

  // The next two classes need to be friends to access the State_ class/member
  friend class RecordablesMap< ignore_and_fire >;
  friend class UniversalDataLogger< ignore_and_fire >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters of the model.
   */
  struct Parameters_
  {
    /** Phase. */
    double phase_;

    /** Firing rate in spikes/s. */
    double rate_;

    Parameters_(); //!< Sets default parameter values

    void get( DictionaryDatum& ) const; //!< Store current values in dictionary

    /** Set values from dictionary.
     * @returns Change in reversal potential E_L, to be passed to State_::set()
     */
    void set( const DictionaryDatum&, Node* node );
  };

  // ----------------------------------------------------------------

  /**
   * State variables of the model.
   */
  struct State_
  {
    double refr_spikes_buffer_;

    State_(); //!< Default initialization

    void get( DictionaryDatum&, const Parameters_& ) const;

    /** Set values from dictionary.
     * @param dictionary to take data from
     * @param current parameters
     * @param Change in reversal potential E_L specified by this dict
     */
    void set( const DictionaryDatum&, const Parameters_&, Node* node );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers of the model.
   */
  struct Buffers_
  {

    Buffers_( ignore_and_fire& );
    Buffers_( const Buffers_&, ignore_and_fire& );

    //! Indices for access to different channels of input_buffer_
    enum
    {
      SYN_IN = 0,
      SYN_EX,
      I0,
      NUM_INPUT_CHANNELS
    };

    /** buffers and sums up incoming spikes/currents */
    MultiChannelInputBuffer< NUM_INPUT_CHANNELS > input_buffer_;

    //! Logger for all analog data
    UniversalDataLogger< ignore_and_fire > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables of the model.
   */
  struct Variables_
  {
    int phase_steps_;
    int firing_period_steps_;
  };
  // ----------------------------------------------------------------

  /**
   * @defgroup ignore_and_fire_data
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
  static RecordablesMap< ignore_and_fire > recordablesMap_;

  inline void
  calc_initial_variables_()
  {
    V_.firing_period_steps_ = Time( Time::ms( 1. / P_.rate_ * 1000. ) ).get_steps();
    V_.phase_steps_ = Time( Time::ms( P_.phase_ / P_.rate_ * 1000. ) ).get_steps();
  }
};

inline size_t
nest::ignore_and_fire::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline size_t
ignore_and_fire::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
ignore_and_fire::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
ignore_and_fire::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
ignore_and_fire::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();
}

inline void
ignore_and_fire::set_status( const DictionaryDatum& d )
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

  ignore_and_fire::calc_initial_variables_();
}

template <>
void RecordablesMap< ignore_and_fire >::create();

} // namespace

#endif /* #ifndef IGNORE_AND_FIRE_H */
