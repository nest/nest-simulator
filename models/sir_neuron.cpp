/*
 *  sir_neuron.cpp
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

#include "sir_neuron.h"

namespace nest
{
void
register_sir_neuron( const std::string& name )
{
  register_node_model< sir_neuron >( name );
}


template <>
void RecordablesMap< sir_neuron >::create()
{
  // use standard names whereever you can for consistency!
  insert_( names::S, &sir_neuron::get_output_state__ );
  insert_( names::h, &sir_neuron::get_input__ );
}


inline size_t
sir_neuron::send_test_event( Node& target, size_t receptor_type, synindex, bool )
{
  SpikeEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline size_t
sir_neuron::handles_test_event( SpikeEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
sir_neuron::handles_test_event( CurrentEvent&, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return 0;
}

inline size_t
sir_neuron::handles_test_event( DataLoggingRequest& dlr, size_t receptor_type )
{
  if ( receptor_type != 0 )
  {
    throw UnknownReceptorType( receptor_type, get_name() );
  }
  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}


inline SignalType
sir_neuron::sends_signal() const
{
  return SIR;
}

inline SignalType
sir_neuron::receives_signal() const
{
  return SIR;
}


inline void
sir_neuron::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d, P_ );
  ArchivingNode::get_status( d );
  ( *d )[ names::recordables ] = recordablesMap_.get_list();

}

inline void
sir_neuron::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;     // temporary copy in case of errors
  ptmp.set( d, this );       // throws if BadProperty
  State_ stmp = S_;          // temporary copy in case of errors
  stmp.set( d, this ); // throws if BadProperty

  // We now know that (ptmp, stmp) are consistent. We do not
  // write them back to (P_, S_) before we are also sure that
  // the properties to be set in the parent class are internally
  // consistent.
  ArchivingNode::set_status( d );

  // if we get here, temporaries contain consistent set of properties
  P_ = ptmp;
  S_ = stmp;

}

RecordablesMap< nest::sir_neuron > nest::sir_neuron::recordablesMap_;

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

sir_neuron::Parameters_::Parameters_()
  : tau_m_( 10.0 ) // ms
  , beta_sir_( 0.1 ) // unitless
  , mu_sir_( 0.1 ) // unitless
{
  recordablesMap_.create();
}

sir_neuron::State_::State_()
  : y_( 0 )
  , h_( 0.0 )
  , last_in_node_id_( 0 )
  , t_next_( Time::neg_inf() )          // mark as not initialized
  , t_last_in_spike_( Time::neg_inf() ) // mark as not intialized
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
sir_neuron::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::tau_m, tau_m_ );
  def< double >( d, names::beta_sir, beta_sir_ );
  def< double >( d, names::mu_sir, mu_sir_ );
}

void
sir_neuron::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::tau_m, tau_m_, node );
  if ( tau_m_ <= 0 )
  {
    throw BadProperty( "All time constants must be strictly positive." );
  }

  updateValueParam< double >( d, names::beta_sir, beta_sir_, node );
  if ( beta_sir_ < 0 || beta_sir_ > 1 )
  {
    throw BadProperty( "All probabilities must be between 0 and 1." );
  }

  updateValueParam< double >( d, names::mu_sir, mu_sir_, node );
  if ( mu_sir_ < 0 || mu_sir_ > 1 )
  {
    throw BadProperty( "All probabilities must be between 0 and 1." );
  }
}

void
sir_neuron::State_::get( DictionaryDatum& d, const Parameters_& ) const
{
  def< double >( d, names::h, h_ ); // summed input
  def< double >( d, names::S, y_ ); // sir_neuron output state
}

void
sir_neuron::State_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::h, h_, node );
  updateValueParam< double >( d, names::S, y_, node );
}

sir_neuron::Buffers_::Buffers_( sir_neuron& n )
  : logger_( n )
{
}

sir_neuron::Buffers_::Buffers_( const Buffers_&, sir_neuron& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

sir_neuron::sir_neuron()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
}

sir_neuron::sir_neuron( const sir_neuron& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
sir_neuron::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();
  ArchivingNode::clear_history();
}

void
sir_neuron::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();
  V_.rng_ = get_vp_specific_rng( get_thread() );

  // update neuron in every time step
  if ( S_.t_next_.is_neg_inf() )
  {
    S_.t_next_ = Time::ms( P_.tau_m_ );
  }
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void
sir_neuron::update( Time const& origin, const long from, const long to )
{
  for ( long lag = from; lag < to; ++lag )
  {
    // update the input current
    // the buffer for incoming spikes for every time step contains the
    // difference
    // of the total input h with respect to the previous step, so sum them up
    S_.h_ += B_.spikes_.get_value( lag );

    // check, if the update needs to be done
    if ( Time::step( origin.get_steps() + lag ) > S_.t_next_ )
    {
      // change the state of the neuron with probability given by
      // infection / recovery rate and next neighbors
      // if the state has changed, the neuron produces an event sent to all its
      // targets

      // initialize y_new
      size_t new_y;

      if (S_.y_ == 0) // neuron is susceptible
      {
        new_y = 0;

        if (V_.rng_->drand() < P_.beta_sir_ * S_.h_)
        {
          new_y = 1; // neuron gets infected
        }
      }

      if (S_.y_ == 1) // neuron is infected
      {
        new_y = 1;
         
        if (V_.rng_->drand() < P_.mu_sir_)
        {
          new_y = 2;  // neuron recovers
        }
      }

      if (S_.y_ == 2) // neuron is recovered
      {
        new_y = 2;
      }


      if ( new_y != S_.y_ )
      {
        SpikeEvent se;
        // use multiplicity 2 to signal transition to 2 state
        // use multiplicity 1 to signal transition to 1 state
        se.set_multiplicity( ( new_y == 2 ) ? 2 : 1 );
        kernel().event_delivery_manager.send( *this, se, lag );

        // As multiplicity is used only to signal internal information
        // to other sir neurons, we only set spiketime once, independent
        // of multiplicity.
        set_spiketime( Time::step( origin.get_steps() + lag + 1 ) );
        S_.y_ = new_y;
      }

      // update interval
      S_.t_next_ += Time::ms( P_.tau_m_ );

    } // of if (update now)

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

  } // of for (lag ...
}

void
sir_neuron::handle( SpikeEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  // The following logic implements the encoding:
  // A single spike signals a transition to I state, two spikes in same time
  // step signal the transition to R state.
  //
  // Remember the node ID of the sender of the last spike being received
  // this assumes that several spikes being sent by the same neuron in the same
  // time step are received consecutively or are conveyed by setting the
  // multiplicity accordingly.
  //
  // Since in collocate_buffers spike events with multiplicity > 1
  // will be converted into sequences of spikes with unit multiplicity,
  // we will count the arrival of the first spike of a doublet (not yet knowing
  // it's a doublet) with a weight -1. The second part of a doublet is then
  // counted with weight 2. Since both parts of a doublet are delivered before
  // update is called, the final value in the ring buffer is guaranteed to be
  // correct.


  const long m = e.get_multiplicity();
  const long node_id = e.retrieve_sender_node_id_from_source_table();
  const Time& t_spike = e.get_stamp();

  if ( m == 1 )
  { // multiplicity == 1, either a single S->I event or the first or second of a
    // pair of I->R events
    if ( node_id == S_.last_in_node_id_ && t_spike == S_.t_last_in_spike_ )
    {
      // received twice the same node ID, so transition I->R
      // take double weight to compensate for subtracting first event
      B_.spikes_.add_value(
        e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), -2.0 * e.get_weight() );
    }
    else
    {
      // count this event negatively, assuming it comes as single event
      // transition S->I
      B_.spikes_.add_value(
        e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), e.get_weight() );
    }
  }
  else if ( m == 2 )
  {
    // count this event positively, transition I->R
    B_.spikes_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), -e.get_weight() );
  }

  S_.last_in_node_id_ = node_id;
  S_.t_last_in_spike_ = t_spike;
}

void
sir_neuron::handle( CurrentEvent& e )
{
  assert( e.get_delay_steps() > 0 );

  const double c = e.get_current();
  const double w = e.get_weight();

  // we use the spike buffer to receive the sir events
  // but also to handle the incoming current events added
  // both contributions are directly added to the variable h
  B_.currents_.add_value( e.get_rel_delivery_steps( kernel().simulation_manager.get_slice_origin() ), w * c );
}


void
sir_neuron::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
sir_neuron::calibrate_time( const TimeConverter& tc )
{
  S_.t_next_ = tc.from_old_tics( S_.t_next_.get_tics() );
  S_.t_last_in_spike_ = tc.from_old_tics( S_.t_last_in_spike_.get_tics() );
}


} // namespace
