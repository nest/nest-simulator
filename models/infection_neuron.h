/*
 *  infection_neuron.h
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

#ifndef INFECTION_NEURON_H
#define INFECTION_NEURON_H

// C++ includes:
#include <cassert>

// Includes from libnestutil:
#include "dict_util.h"

// Includes from nestkernel:
#include "archiving_node.h"
#include "event.h"
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "nest_timeconverter.h"
#include "nest_types.h"
#include "random_generators.h"
#include "recordables_map.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

namespace nest
{

/**
 * Base implementation for synchronously updated infection neurons.
 *
 * The update function supplies the model-specific transition probabilities,
 * status-dictionary entries, state transition, and event multiplicity. It must
 * provide the following interface:
 *
 *   void get( Dictionary& ) const;
 *   void set( const Dictionary&, Node* );
 *   size_t operator()( RngPtr, size_t old_state, double h ) const;
 *   size_t get_event_multiplicity( size_t new_state ) const;
 *
 * An event multiplicity of zero means that the state transition is silent.
 */
template < class TUpdateFunction >
class infection_neuron : public ArchivingNode
{

public:
  infection_neuron();
  infection_neuron( const infection_neuron& );

  /**
   * Import sets of overloaded virtual functions.
   * @see Technical Issues / Virtual Functions: Overriding, Overloading, and
   * Hiding
   */
  using Node::handle;
  using Node::handles_test_event;
  using Node::receives_signal;
  using Node::sends_signal;

  size_t send_test_event( Node&, size_t, synindex, bool ) override;

  void handle( SpikeEvent& ) override;
  void handle( CurrentEvent& ) override;
  void handle( DataLoggingRequest& ) override;

  size_t handles_test_event( SpikeEvent&, size_t ) override;
  size_t handles_test_event( CurrentEvent&, size_t ) override;
  size_t handles_test_event( DataLoggingRequest&, size_t ) override;

  SignalType sends_signal() const override;
  SignalType receives_signal() const override;

  void get_status( Dictionary& ) const override;
  void set_status( const Dictionary& ) override;

  void calibrate_time( const TimeConverter& tc ) override;


private:
  void init_buffers_() override;
  void pre_run_hook() override;

  void update( Time const&, const long, const long ) override;

  // The next two classes need to be friends to access State_ and its members.
  friend class RecordablesMap< infection_neuron< TUpdateFunction > >;
  friend class UniversalDataLogger< infection_neuron< TUpdateFunction > >;

  // ----------------------------------------------------------------

  /**
   * Independent parameters shared by all infection models.
   */
  struct Parameters_
  {
    //! Inter-update interval in ms (acts like a membrane time constant).
    double tau_m_;

    Parameters_();  //!< Sets default parameter values

    void get( Dictionary& ) const;              //!< Store current values in dictionary
    void set( const Dictionary&, Node* node );  //!< Set values from dictionary
  };

  // ----------------------------------------------------------------

  /**
   * State variables shared by all infection models.
   */
  struct State_
  {
    size_t y_;                //!< output state: susceptible=0, infected=1, recovered=2
    double h_;                //!< number of infected presynaptic neurons
    double last_in_node_id_;  //!< node ID of the last spike being received
    Time t_next_;             //!< time point of next update
    Time t_last_in_spike_;    //!< time point of last input spike seen

    State_();  //!< Default initialization

    void get( Dictionary&, const Parameters_& ) const;
    void set( const Dictionary&, Node* );
  };

  // ----------------------------------------------------------------

  /**
   * Buffers shared by all infection models.
   */
  struct Buffers_
  {
    Buffers_( infection_neuron& );
    Buffers_( const Buffers_&, infection_neuron& );

    /** buffers and sums up incoming spikes/currents */
    RingBuffer spikes_;
    RingBuffer currents_;

    //! Logger for all analog data
    UniversalDataLogger< infection_neuron > logger_;
  };

  // ----------------------------------------------------------------

  /**
   * Internal variables shared by all infection models.
   */
  struct Variables_
  {
    RngPtr rng_;  //!< random number generator of my own thread
  };

  // Access functions for UniversalDataLogger -------------------------------

  //! Read out the infection state of the neuron.
  double
  get_output_state_() const
  {
    return S_.y_;
  }

  //! Read out the number of infected presynaptic neurons.
  double
  get_input__() const
  {
    return S_.h_;
  }

  // ----------------------------------------------------------------

  /**
   * Instances of private data structures for the different types
   * of data pertaining to the model.
   * @note The order of definitions is important for speed.
   * @{
   */
  Parameters_ P_;
  TUpdateFunction update_function_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;
  /** @} */

  //! Mapping of recordable names to access functions
  static RecordablesMap< infection_neuron< TUpdateFunction > > recordablesMap_;
};


template < class TUpdateFunction >
inline size_t
infection_neuron< TUpdateFunction >::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

template < class TUpdateFunction >
inline size_t
infection_neuron< TUpdateFunction >::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TUpdateFunction >
inline size_t
infection_neuron< TUpdateFunction >::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

template < class TUpdateFunction >
inline size_t
infection_neuron< TUpdateFunction >::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}


template < class TUpdateFunction >
inline SignalType
infection_neuron< TUpdateFunction >::sends_signal() const
{
  return BINARY;
}

template < class TUpdateFunction >
inline SignalType
infection_neuron< TUpdateFunction >::receives_signal() const
{
  return BINARY;
}


template < class TUpdateFunction >
inline void
infection_neuron< TUpdateFunction >::get_status( Dictionary& d ) const
{
  P_.get( d );
  update_function_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );
  d[ names::recordables ] = recordablesMap_.get_list();
}

template < class TUpdateFunction >
inline void
infection_neuron< TUpdateFunction >::set_status( const Dictionary& d )
{
  Parameters_ ptmp = P_;  // temporary copy in case of errors
  ptmp.set( d, this );    // throws if BadProperty
  TUpdateFunction update_tmp = update_function_;
  update_tmp.set( d, this );  // throws if BadProperty
  State_ stmp = S_;           // temporary copy in case of errors
  stmp.set( d, this );        // throws if BadProperty

  // We now know that all temporary properties are internally consistent. Do
  // not write them back before the parent properties have also been checked.
  ArchivingNode::set_status( d );

  P_ = ptmp;
  update_function_ = update_tmp;
  S_ = stmp;
}

template < typename TUpdateFunction >
RecordablesMap< nest::infection_neuron< TUpdateFunction > > nest::infection_neuron< TUpdateFunction >::recordablesMap_;

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

template < class TUpdateFunction >
infection_neuron< TUpdateFunction >::Parameters_::Parameters_()
  : tau_m_( 10.0 )  // ms
{
  recordablesMap_.create();
}

template < class TUpdateFunction >
infection_neuron< TUpdateFunction >::State_::State_()
  : y_( 0 )
  , h_( 0.0 )
  , last_in_node_id_( 0 )
  , t_next_( Time::neg_inf() )           // mark as not initialized
  , t_last_in_spike_( Time::neg_inf() )  // mark as not initialized
{
}

/* ----------------------------------------------------------------
 * Parameter and state extraction and manipulation functions
 * ---------------------------------------------------------------- */

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::Parameters_::get( Dictionary& d ) const
{
  d[ names::tau_m ] = tau_m_;
}

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::Parameters_::set( const Dictionary& d, Node* node )
{
  update_value_param( d, names::tau_m, tau_m_, node );
  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }
}

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::State_::get( Dictionary& d, const Parameters_& ) const
{
  d[ names::h ] = h_;                         // summed input
  d[ names::S ] = static_cast< long >( y_ );  // infection-neuron output state
}

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::State_::set( const Dictionary& d, Node* node )
{
  update_value_param( d, names::h, h_, node );

  long y = static_cast< long >( y_ );
  if ( update_value_param( d, names::S, y, node ) )
  {
    y_ = static_cast< size_t >( y );
  }
}

template < class TUpdateFunction >
infection_neuron< TUpdateFunction >::Buffers_::Buffers_( infection_neuron& n )
  : logger_( n )
{
}

template < class TUpdateFunction >
infection_neuron< TUpdateFunction >::Buffers_::Buffers_( const Buffers_&, infection_neuron& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

template < class TUpdateFunction >
infection_neuron< TUpdateFunction >::infection_neuron()
  : ArchivingNode()
  , P_()
  , update_function_()
  , S_()
  , B_( *this )
{
}

template < class TUpdateFunction >
infection_neuron< TUpdateFunction >::infection_neuron( const infection_neuron& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , update_function_( n.update_function_ )
  , S_( n.S_ )
  , B_( *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::init_buffers_()
{
  B_.spikes_.clear();    // includes resize
  B_.currents_.clear();  // includes resize
  B_.logger_.reset();
  ArchivingNode::clear_history();
}

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::pre_run_hook()
{
  // Ensures initialization in case a multimeter is connected after Simulate.
  B_.logger_.init();
  V_.rng_ = get_vp_specific_rng( get_thread() );

  if ( S_.t_next_.is_neg_inf() )
  {
    S_.t_next_ = Time::ms( P_.tau_m_ );
  }
}


/* ----------------------------------------------------------------
 * Update and event-handling functions
 * ---------------------------------------------------------------- */

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::update( Time const& origin, const long from, const long to )
{
  for ( long lag = from; lag < to; ++lag )
  {
    // Incoming spike events contain the change in the number of infected
    // presynaptic neurons, so accumulate them into h.
    S_.h_ += B_.spikes_.get_value( lag );

    if ( Time::step( origin.get_steps() + lag ) > S_.t_next_ )
    {
      const size_t new_y = update_function_( V_.rng_, S_.y_, S_.h_ );

      if ( new_y != S_.y_ )
      {
        const size_t multiplicity = update_function_.get_event_multiplicity( new_y );

        if ( multiplicity > 0 )
        {
          SpikeEvent se;
          se.set_multiplicity( multiplicity );
          kernel().event_delivery_manager.send( *this, se, lag );

          // Multiplicity only conveys internal state information, so record
          // the spike time once regardless of multiplicity.
          set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        }

        S_.y_ = new_y;
      }

      S_.t_next_ += Time::ms( P_.tau_m_ );
    }

    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  // A single spike signals a transition into the infected state. A doublet
  // signals a transition out of the infected state. When doublets are
  // delivered as two consecutive unit-multiplicity events, the first adds one
  // and the second subtracts two, giving the required net change of minus one.
  const long m = e.get_multiplicity();
  const long node_id = e.retrieve_sender_node_id_from_source_table();
  const Time& t_spike = e.get_stamp();

  if ( m == 1 )
  {
    if ( node_id == S_.last_in_node_id_ and t_spike == S_.t_last_in_spike_ )
    {
      B_.spikes_.add_value(
        e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), -2.0 * e.get_weight() );
    }
    else
    {
      B_.spikes_.add_value(
        e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() );
    }
  }
  else if ( m == 2 )
  {
    B_.spikes_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), -e.get_weight() );
  }

  S_.last_in_node_id_ = node_id;
  S_.t_last_in_spike_ = t_spike;
}

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // current events are accepted and buffered, but are not consumed by update().
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

template < class TUpdateFunction >
void
infection_neuron< TUpdateFunction >::calibrate_time( const TimeConverter& tc )
{
  S_.t_next_ = tc.from_old_tics( S_.t_next_.get_tics() );
  S_.t_last_in_spike_ = tc.from_old_tics( S_.t_last_in_spike_.get_tics() );
}

}  // namespace nest

#endif /* #ifndef INFECTION_NEURON_H */
