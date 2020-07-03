/*
 *  correlospinmatrix_detector.cpp
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

#include "correlospinmatrix_detector.h"

// C++ includes:
#include <cmath>
#include <functional>
#include <numeric>

// Includes from libnestutil:
#include "dict_util.h"

// Includes from nestkernel:
#include "kernel_manager.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::correlospinmatrix_detector::Parameters_::Parameters_()
  : delta_tau_( Time::get_resolution() )
  , tau_max_( 10 * delta_tau_ )
  , Tstart_( Time::ms( 0.0 ) )
  , Tstop_( Time::pos_inf() )
  , N_channels_( 1 )
{
}

nest::correlospinmatrix_detector::Parameters_::Parameters_( const Parameters_& p )
  : delta_tau_( p.delta_tau_ )
  , tau_max_( p.tau_max_ )
  , Tstart_( p.Tstart_ )
  , Tstop_( p.Tstop_ )
  , N_channels_( p.N_channels_ )
{
  // Check for proper properties is not done here but in the
  // correlospinmatrix_detector() copy c'tor. The check cannot be
  // placed here, since this c'tor is also used to copy to
  // temporaries in correlospinmatrix_detector::set_status().
  // If we checked for errors here, we could never change values
  // that have become invalid after a resolution change.
  delta_tau_.calibrate();
  tau_max_.calibrate();
  Tstart_.calibrate();
  Tstop_.calibrate();
}

nest::correlospinmatrix_detector::State_::State_()
  : incoming_()
  , last_i_( 0 )
  , t_last_in_spike_( Time::neg_inf() )
  , count_covariance_( 1, std::vector< std::vector< long > >( 1, std::vector< long >() ) )
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::correlospinmatrix_detector::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::delta_tau ] = delta_tau_.get_ms();
  ( *d )[ names::tau_max ] = tau_max_.get_ms();
  ( *d )[ names::Tstart ] = Tstart_.get_ms();
  ( *d )[ names::Tstop ] = Tstop_.get_ms();
  ( *d )[ names::N_channels ] = N_channels_;
}

void
nest::correlospinmatrix_detector::State_::get( DictionaryDatum& d ) const
{
  ArrayDatum* CountC = new ArrayDatum;
  for ( size_t i = 0; i < count_covariance_.size(); ++i )
  {
    ArrayDatum* CountC_i = new ArrayDatum;
    for ( size_t j = 0; j < count_covariance_[ i ].size(); ++j )
    {
      CountC_i->push_back( new IntVectorDatum( new std::vector< long >( count_covariance_[ i ][ j ] ) ) );
    }
    CountC->push_back( *CountC_i );
  }
  ( *d )[ names::count_covariance ] = CountC;
}

bool
nest::correlospinmatrix_detector::Parameters_::set( const DictionaryDatum& d,
  const correlospinmatrix_detector& n,
  Node* node )
{
  bool reset = false;
  double t;
  long N;

  if ( updateValueParam< long >( d, names::N_channels, N, node ) )
  {
    if ( N < 1 )
    {
      throw BadProperty( "/N_channels can only be larger than zero." );
    }
    else
    {
      N_channels_ = N;
      reset = true;
    }
  }

  if ( updateValueParam< double >( d, names::delta_tau, t, node ) )
  {
    delta_tau_ = Time::ms( t );
    reset = true;
    if ( t < 0 )
    {
      throw BadProperty( "/delta_tau must not be negative." );
    }
  }

  if ( updateValueParam< double >( d, names::tau_max, t, node ) )
  {
    tau_max_ = Time::ms( t );
    reset = true;
    if ( t < 0 )
    {
      throw BadProperty( "/tau_max must not be negative." );
    }
  }

  if ( updateValueParam< double >( d, names::Tstart, t, node ) )
  {
    Tstart_ = Time::ms( t );
    reset = true;
    if ( t < 0 )
    {
      throw BadProperty( "/Tstart must not be negative." );
    }
  }

  if ( updateValueParam< double >( d, names::Tstop, t, node ) )
  {
    Tstop_ = Time::ms( t );
    reset = true;
    if ( t < 0 )
    {
      throw BadProperty( "/Tstop must not be negative." );
    }
  }

  if ( not delta_tau_.is_step() )
  {
    throw StepMultipleRequired( n.get_name(), names::delta_tau, delta_tau_ );
  }

  if ( not tau_max_.is_multiple_of( delta_tau_ ) )
  {
    throw TimeMultipleRequired( n.get_name(), names::tau_max, tau_max_, names::delta_tau, delta_tau_ );
  }
  return reset;
}

void
nest::correlospinmatrix_detector::State_::set( const DictionaryDatum&, const Parameters_&, bool, Node* node )
{
}

void
nest::correlospinmatrix_detector::State_::reset( const Parameters_& p )
{

  last_i_ = 0;
  tentative_down_ = false;
  t_last_in_spike_ = Time::neg_inf();

  incoming_.clear();

  assert( p.tau_max_.is_multiple_of( p.delta_tau_ ) );

  count_covariance_.clear();
  count_covariance_.resize( p.N_channels_ );

  curr_state_.clear();
  curr_state_.resize( p.N_channels_ );

  last_change_.clear();
  last_change_.resize( p.N_channels_ );

  for ( long i = 0; i < p.N_channels_; ++i )
  {
    count_covariance_[ i ].resize( p.N_channels_ );
    for ( long j = 0; j < p.N_channels_; ++j )
    {
      count_covariance_[ i ][ j ].resize( 1 + 2.0 * p.tau_max_.get_steps() / p.delta_tau_.get_steps(), 0 );
    }
  }
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::correlospinmatrix_detector::correlospinmatrix_detector()
  : Node()
  , device_()
  , P_()
  , S_()
{
  if ( not P_.delta_tau_.is_step() )
  {
    throw InvalidDefaultResolution( get_name(), names::delta_tau, P_.delta_tau_ );
  }
}

nest::correlospinmatrix_detector::correlospinmatrix_detector( const correlospinmatrix_detector& n )
  : Node( n )
  , device_( n.device_ )
  , P_( n.P_ )
  , S_()
{
  if ( not P_.delta_tau_.is_step() )
  {
    throw InvalidTimeInModel( get_name(), names::delta_tau, P_.delta_tau_ );
  }
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::correlospinmatrix_detector::init_state_( const Node& proto )
{
  const correlospinmatrix_detector& pr = downcast< correlospinmatrix_detector >( proto );

  device_.init_state( pr.device_ );
  S_ = pr.S_;
  set_buffers_initialized( false ); // force recreation of buffers
}

void
nest::correlospinmatrix_detector::init_buffers_()
{
  device_.init_buffers();
  S_.reset( P_ );
}

void
nest::correlospinmatrix_detector::calibrate()
{
  device_.calibrate();
}


/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::correlospinmatrix_detector::update( Time const&, const long, const long )
{
}

void
nest::correlospinmatrix_detector::handle( SpikeEvent& e )
{
  // The receiver port identifies the sending node in our
  // sender list.
  const rport curr_i = e.get_rport();

  // If this assertion breaks, the sender does not honor the
  // receiver port during connection or sending.
  assert( 0 <= curr_i && curr_i <= P_.N_channels_ - 1 );

  // accept spikes only if detector was active when spike was emitted
  Time const stamp = e.get_stamp();

  if ( device_.is_active( stamp ) )
  {

    // The following logic implements the decoding
    // A single spike signals a transition to 0 state, two spikes in same time
    // step signal the transition to 1 state.
    //
    // Remember the node ID of the sender of the last spike being received
    // this assumes that several spikes being sent by the same neuron in the
    // same time step are received consecutively or are conveyed by setting the
    // multiplicity accordingly.

    long m = e.get_multiplicity();
    bool down_transition = false;

    if ( m == 1 )
    { // multiplicity == 1, either a single 1->0 event or the first or second of
      // a pair of 0->1
      // events
      if ( curr_i == S_.last_i_ && stamp == S_.t_last_in_spike_ )
      {
        // received twice the same node ID, so transition 0->1
        // revise the last event written to the buffer
        S_.curr_state_[ curr_i ] = true;
        S_.last_change_[ curr_i ] = stamp.get_steps();
        S_.tentative_down_ = false; // previous event was first event of two, so no down transition
      }
      else
      {
        // count this event negatively, assuming it comes as single event
        // transition 1->0
        // assume it will stay alone, so meaning a down tansition

        if ( S_.tentative_down_ ) // really was a down transition, because we
                                  // now have another event
        {
          down_transition = true;
        }

        S_.tentative_down_ = true;
      }
    }
    else // multiplicity != 1
      if ( m == 2 )
    {
      S_.curr_state_[ curr_i ] = true;

      if ( S_.tentative_down_ ) // really was a down transition, because we now
                                // have another double event
      {
        down_transition = true;
      }

      S_.curr_state_[ S_.last_i_ ] = false;
      S_.last_change_[ curr_i ] = stamp.get_steps();
      // previous event was first event of two, so no down transition
      S_.tentative_down_ = false;
    }

    if ( down_transition ) // only do something on the downtransitions
    {
      long i = S_.last_i_; // index of neuron making the down transition
      // last time point of change, must have been on
      long t_i_on = S_.last_change_[ i ];

      const long t_i_off = S_.t_last_in_spike_.get_steps();

      // throw out all binary pulses from event list that are too old to enter
      // the correlation window
      BinaryPulselistType& otherPulses = S_.incoming_;

      // calculate the minimum of those neurons that switched on and are not off
      // yet every impulse in the queue that is further in the past than
      // this minimum - tau_max cannot contribute to the count covariance
      long t_min_on = t_i_on;
      for ( int n = 0; n < P_.N_channels_; n++ )
      {
        if ( S_.curr_state_[ n ] )
        {
          if ( S_.last_change_[ n ] < t_min_on )
          {
            t_min_on = S_.last_change_[ n ];
          }
        }
      }
      const double tau_edge = P_.tau_max_.get_steps() + P_.delta_tau_.get_steps();

      const delay min_delay = kernel().connection_manager.get_min_delay();
      while ( not otherPulses.empty() && ( t_min_on - otherPulses.front().t_off_ ) >= tau_edge + min_delay )
      {
        otherPulses.pop_front();
      }


      // insert new event into history
      // must happen here so event is taken into account in autocorrelation
      const BinaryPulse_ bp_i( t_i_on, t_i_off, i );

      BinaryPulselistType::iterator insert_pos = std::find_if( S_.incoming_.begin(),
        S_.incoming_.end(),
        std::bind( std::greater< BinaryPulse_ >(), std::placeholders::_1, bp_i ) );

      // insert before the position we have found
      // if no element greater found, insert_pos == end(), so append at the end
      // of the deque
      S_.incoming_.insert( insert_pos, bp_i );


      // go through history of other binary pulses
      for ( BinaryPulselistType::const_iterator pulse_j = otherPulses.begin(); pulse_j != otherPulses.end(); ++pulse_j )
      {
        // id of other neuron
        long j = pulse_j->receptor_channel_;
        long t_j_on = pulse_j->t_on_;
        long t_j_off = pulse_j->t_off_;

        // minimum and maximum time difference in histogram
        long Delta_ij_min = std::max( t_j_on - t_i_off, -P_.tau_max_.get_steps() );
        long Delta_ij_max = std::min( t_j_off - t_i_on, P_.tau_max_.get_steps() );

        long t0 = P_.tau_max_.get_steps() / P_.delta_tau_.get_steps();
        long dt = P_.delta_tau_.get_steps();


        // zero time lag covariance

        long lag = std::min( t_i_off, t_j_off ) - std::max( t_i_on, t_j_on );
        if ( lag > 0 )
        {
          S_.count_covariance_[ i ][ j ][ t0 ] += lag;
          if ( i != j )
          {
            S_.count_covariance_[ j ][ i ][ t0 ] += lag;
          }
        }

        // non-zero time lag covariance
        for ( long Delta = Delta_ij_min / dt; Delta < 0; Delta++ )
        {
          long lag = std::min( t_i_off, t_j_off - Delta * dt ) - std::max( t_i_on, t_j_on - Delta * dt );
          if ( lag > 0 )
          {
            S_.count_covariance_[ i ][ j ][ t0 - Delta ] += lag;
            S_.count_covariance_[ j ][ i ][ t0 + Delta ] += lag;
          }
        }

        if ( i != j )
        {
          for ( long Delta = 1; Delta <= Delta_ij_max / dt; Delta++ )
          {
            long lag = std::min( t_i_off, t_j_off - Delta * dt ) - std::max( t_i_on, t_j_on - Delta * dt );
            if ( lag > 0 )
            {
              S_.count_covariance_[ i ][ j ][ t0 - Delta ] += lag;
              S_.count_covariance_[ j ][ i ][ t0 + Delta ] += lag;
            }
          }
        }
      } // loop over history

      S_.last_change_[ i ] = t_i_off;
    } // down transition happened

    S_.last_i_ = curr_i;
    S_.t_last_in_spike_ = stamp;

  } // device active
}
