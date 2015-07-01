/*
 *  binary_neuron_impl.h
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

#ifndef BINARY_NEURON_IMPL_H
#define BINARY_NEURON_IMPL_H

#include "exceptions.h"
#include "binary_neuron.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"

#include <limits>


namespace nest
{

template < typename TGainfunction >
RecordablesMap< nest::binary_neuron< TGainfunction > >
  nest::binary_neuron< TGainfunction >::recordablesMap_;

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

template < class TGainfunction >
binary_neuron< TGainfunction >::Parameters_::Parameters_()
  : tau_m_( 10.0 ) // ms
{
  recordablesMap_.create();
}

template < class TGainfunction >
binary_neuron< TGainfunction >::State_::State_()
  : y_( false )
  , h_( 0.0 )
  , last_in_gid_( 0 )
  , t_next_( Time::neg_inf() )          // mark as not initialized
  , t_last_in_spike_( Time::neg_inf() ) // mark as not intialized
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

template < class TGainfunction >
void
binary_neuron< TGainfunction >::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::tau_m, tau_m_ );
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::tau_m, tau_m_ );

  if ( tau_m_ <= 0 )
    throw BadProperty( "All time constants must be strictly positive." );
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::State_::get( DictionaryDatum& d, const Parameters_& ) const
{
  def< double >( d, names::h, h_ ); // summed input
  def< double >( d, names::S, y_ ); // binary_neuron output state
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::State_::set( const DictionaryDatum&, const Parameters_& )
{
}

template < class TGainfunction >
binary_neuron< TGainfunction >::Buffers_::Buffers_( binary_neuron& n )
  : logger_( n )
{
}

template < class TGainfunction >
binary_neuron< TGainfunction >::Buffers_::Buffers_( const Buffers_&, binary_neuron& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

template < class TGainfunction >
binary_neuron< TGainfunction >::binary_neuron()
  : Archiving_Node()
  , P_()
  , S_()
  , B_( *this )
{
}

template < class TGainfunction >
binary_neuron< TGainfunction >::binary_neuron( const binary_neuron& n )
  : Archiving_Node( n )
  , gain_( n.gain_ )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

template < class TGainfunction >
void
binary_neuron< TGainfunction >::init_state_( const Node& proto )
{
  const binary_neuron& pr = downcast< binary_neuron >( proto );
  S_ = pr.S_;
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();
  Archiving_Node::clear_history();
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::calibrate()
{
  B_.logger_.init(); // ensures initialization in case mm connected after Simulate
  V_.rng_ = net_->get_rng( get_thread() );

  // draw next time of update for the neuron from exponential distribution
  // only if not yet initialized
  if ( S_.t_next_.is_neg_inf() )
    S_.t_next_ = Time::ms( V_.exp_dev_( V_.rng_ ) * P_.tau_m_ );
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

template < class TGainfunction >
void
binary_neuron< TGainfunction >::update( Time const& origin, const long_t from, const long_t to )
{
  assert( to >= 0 && ( delay ) from < Scheduler::get_min_delay() );
  assert( from < to );

  for ( long_t lag = from; lag < to; ++lag )
  {
    // update the input current
    // the buffer for incoming spikes for every time step contains the difference
    // of the total input h with respect to the previous step, so sum them up
    S_.h_ += B_.spikes_.get_value( lag );

    double_t c = B_.currents_.get_value( lag );

    // check, if the update needs to be done
    if ( Time::step( origin.get_steps() + lag ) > S_.t_next_ )
    {
      // change the state of the neuron with probability given by
      // gain function
      // if the state has changed, the neuron produces an event sent to all its targets

      bool new_y = gain_( V_.rng_, S_.h_ + c );

      if ( new_y != S_.y_ )
      {
        SpikeEvent se;
        // use multiplicity 2 to signalize transition to 1 state
        // use multiplicity 1 to signalize transition to 0 state
        se.set_multiplicity( new_y ? 2 : 1 );
        network()->send( *this, se, lag );
        S_.y_ = new_y;
      }

      // draw next update interval from exponential distribution
      S_.t_next_ += Time::ms( V_.exp_dev_( V_.rng_ ) * P_.tau_m_ );

    } // of if (update now)

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

  } // of for (lag ...
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::handle( SpikeEvent& e )
{
  assert( e.get_delay() > 0 );

  // The following logic implements the encoding:
  // A single spike signals a transition to 0 state, two spikes in same time step
  // signal the transition to 1 state.
  //
  // Remember the global id of the sender of the last spike being received
  // this assumes that several spikes being sent by the same neuron in the same time step
  // are received consecutively or are conveyed by setting the multiplicity accordingly.

  long_t m = e.get_multiplicity();
  long_t gid = e.get_sender_gid();
  const Time& t_spike = e.get_stamp();

  if ( m == 1 )
  { // multiplicity == 1, either a single 1->0 event or the first or second of a pair of 0->1 events
    if ( gid == S_.last_in_gid_ && t_spike == S_.t_last_in_spike_ )
    {
      // received twice the same gid, so transition 0->1
      // take double weight to compensate for subtracting first event
      B_.spikes_.add_value(
        e.get_rel_delivery_steps( network()->get_slice_origin() ), 2.0 * e.get_weight() );
    }
    else
    {
      // count this event negatively, assuming it comes as single event
      // transition 1->0
      B_.spikes_.add_value(
        e.get_rel_delivery_steps( network()->get_slice_origin() ), -e.get_weight() );
    }
  }
  else // multiplicity != 1
    if ( m == 2 )
  {
    // count this event positively, transition 0->1
    B_.spikes_.add_value(
      e.get_rel_delivery_steps( network()->get_slice_origin() ), e.get_weight() );
  }

  S_.last_in_gid_ = gid;
  S_.t_last_in_spike_ = t_spike;
}

template < class TGainfunction >
void
binary_neuron< TGainfunction >::handle( CurrentEvent& e )
{
  assert( e.get_delay() > 0 );

  const double_t c = e.get_current();
  const double_t w = e.get_weight();

  // we use the spike buffer to receive the binary events
  // but also to handle the incoming current events added
  // both contributions are directly added to the variable h
  B_.currents_.add_value( e.get_rel_delivery_steps( network()->get_slice_origin() ), w * c );
}


template < class TGainfunction >
void
binary_neuron< TGainfunction >::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}


} // namespace

#endif /* #ifndef BINARY_NEURON_IMPL_H */
