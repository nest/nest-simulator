/*
 *  correlomatrix_detector.cpp
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

#include "correlomatrix_detector.h"

// C++ includes:
#include <cmath>      // for less
#include <functional> // for bind2nd
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

nest::correlomatrix_detector::Parameters_::Parameters_()
  : delta_tau_( 5 * Time::get_resolution() )
  , tau_max_( 10 * delta_tau_ )
  , Tstart_( Time::ms( 0.0 ) )
  , Tstop_( Time::pos_inf() )
  , N_channels_( 1 )
{
}

nest::correlomatrix_detector::Parameters_::Parameters_( const Parameters_& p )
  : delta_tau_( p.delta_tau_ )
  , tau_max_( p.tau_max_ )
  , Tstart_( p.Tstart_ )
  , Tstop_( p.Tstop_ )
  , N_channels_( p.N_channels_ )
{
  // Check for proper properties is not done here but in the
  // correlomatrix_detector() copy c'tor. The check cannot be
  // placed here, since this c'tor is also used to copy to
  // temporaries in correlomatrix_detector::set_status().
  // If we checked for errors here, we could never change values
  // that have become invalid after a resolution change.
  delta_tau_.calibrate();
  tau_max_.calibrate();
  Tstart_.calibrate();
  Tstop_.calibrate();
}

nest::correlomatrix_detector::Parameters_& nest::correlomatrix_detector::Parameters_::operator=( const Parameters_& p )
{
  delta_tau_ = p.delta_tau_;
  tau_max_ = p.tau_max_;
  Tstart_ = p.Tstart_;
  Tstop_ = p.Tstop_;
  N_channels_ = p.N_channels_;

  delta_tau_.calibrate();
  tau_max_.calibrate();
  Tstart_.calibrate();
  Tstop_.calibrate();

  return *this;
}

nest::correlomatrix_detector::State_::State_()
  : n_events_( 1, 0 )
  , incoming_()
  , covariance_( 1, std::vector< std::vector< double > >( 1, std::vector< double >() ) )
  , count_covariance_( 1, std::vector< std::vector< long > >( 1, std::vector< long >() ) )
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::correlomatrix_detector::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::delta_tau ] = delta_tau_.get_ms();
  ( *d )[ names::tau_max ] = tau_max_.get_ms();
  ( *d )[ names::Tstart ] = Tstart_.get_ms();
  ( *d )[ names::Tstop ] = Tstop_.get_ms();
  ( *d )[ names::N_channels ] = N_channels_;
}

void
nest::correlomatrix_detector::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::n_events ] = IntVectorDatum( new std::vector< long >( n_events_ ) );

  ArrayDatum* C = new ArrayDatum;
  ArrayDatum* CountC = new ArrayDatum;
  for ( size_t i = 0; i < covariance_.size(); ++i )
  {
    ArrayDatum* C_i = new ArrayDatum;
    ArrayDatum* CountC_i = new ArrayDatum;
    for ( size_t j = 0; j < covariance_[ i ].size(); ++j )
    {
      C_i->push_back( new DoubleVectorDatum( new std::vector< double >( covariance_[ i ][ j ] ) ) );
      CountC_i->push_back( new IntVectorDatum( new std::vector< long >( count_covariance_[ i ][ j ] ) ) );
    }
    C->push_back( *C_i );
    CountC->push_back( *CountC_i );
  }
  ( *d )[ names::covariance ] = C;
  ( *d )[ names::count_covariance ] = CountC;
}

bool
nest::correlomatrix_detector::Parameters_::set( const DictionaryDatum& d, const correlomatrix_detector& n, Node* node )
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
  }

  if ( updateValueParam< double >( d, names::tau_max, t, node ) )
  {
    tau_max_ = Time::ms( t );
    reset = true;
  }

  if ( updateValueParam< double >( d, names::Tstart, t, node ) )
  {
    Tstart_ = Time::ms( t );
    reset = true;
  }

  if ( updateValueParam< double >( d, names::Tstop, t, node ) )
  {
    Tstop_ = Time::ms( t );
    reset = true;
  }

  if ( not delta_tau_.is_step() )
  {
    throw StepMultipleRequired( n.get_name(), names::delta_tau, delta_tau_ );
  }

  if ( not tau_max_.is_multiple_of( delta_tau_ ) )
  {
    throw TimeMultipleRequired( n.get_name(), names::tau_max, tau_max_, names::delta_tau, delta_tau_ );
  }

  if ( delta_tau_.get_steps() % 2 != 1 )
  {
    throw BadProperty( "/delta_tau must be odd multiple of resolution." );
  }

  return reset;
}

void
nest::correlomatrix_detector::State_::set( const DictionaryDatum&, const Parameters_&, bool, Node* )
{
}

void
nest::correlomatrix_detector::State_::reset( const Parameters_& p )
{
  n_events_.clear();
  n_events_.resize( p.N_channels_, 0 );

  incoming_.clear();

  assert( p.tau_max_.is_multiple_of( p.delta_tau_ ) );

  covariance_.clear();
  covariance_.resize( p.N_channels_ );

  count_covariance_.clear();
  count_covariance_.resize( p.N_channels_ );

  for ( long i = 0; i < p.N_channels_; ++i )
  {
    covariance_[ i ].resize( p.N_channels_ );
    count_covariance_[ i ].resize( p.N_channels_ );
    for ( long j = 0; j < p.N_channels_; ++j )
    {
      covariance_[ i ][ j ].resize( 1 + p.tau_max_.get_steps() / p.delta_tau_.get_steps(), 0 );
      count_covariance_[ i ][ j ].resize( 1 + p.tau_max_.get_steps() / p.delta_tau_.get_steps(), 0 );
    }
  }
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::correlomatrix_detector::correlomatrix_detector()
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

nest::correlomatrix_detector::correlomatrix_detector( const correlomatrix_detector& n )
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
nest::correlomatrix_detector::init_state_()
{
  device_.init_state();
}

void
nest::correlomatrix_detector::init_buffers_()
{
  device_.init_buffers();
  S_.reset( P_ );
}

void
nest::correlomatrix_detector::calibrate()
{
  device_.calibrate();
}


/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::correlomatrix_detector::update( Time const&, const long, const long )
{
}

void
nest::correlomatrix_detector::handle( SpikeEvent& e )
{
  // The receiver port identifies the sending node in our
  // sender list.
  const rport sender = e.get_rport();

  // If this assertion breaks, the sender does not honor the
  // receiver port during connection or sending.
  assert( 0 <= sender && sender <= P_.N_channels_ - 1 );

  // accept spikes only if detector was active when spike was emitted
  Time const stamp = e.get_stamp();

  if ( device_.is_active( stamp ) )
  {
    const long spike_i = stamp.get_steps();

    // find first appearence of element which is greater than spike_i
    const Spike_ sp_i( spike_i, e.get_multiplicity() * e.get_weight(), sender );
    SpikelistType::iterator insert_pos = std::find_if(
      S_.incoming_.begin(), S_.incoming_.end(), std::bind( std::greater< Spike_ >(), std::placeholders::_1, sp_i ) );

    // insert before the position we have found
    // if no element greater found, insert_pos == end(), so append at the end of
    // the deque
    S_.incoming_.insert( insert_pos, sp_i );

    SpikelistType& otherSpikes = S_.incoming_;
    const double tau_edge = P_.tau_max_.get_steps() + 0.5 * P_.delta_tau_.get_steps();

    // throw away all spikes which are too old to
    // enter the correlation window
    const delay min_delay = kernel().connection_manager.get_min_delay();
    while ( not otherSpikes.empty() && ( spike_i - otherSpikes.front().timestep_ ) >= tau_edge + min_delay )
    {
      otherSpikes.pop_front();
    }
    // all remaining spike times in the queue are >= spike_i - tau_edge -
    // min_delay

    // only count events in histogram, if the current event is within the time
    // window [Tstart,
    // Tstop]
    // this is needed in order to prevent boundary effects
    if ( P_.Tstart_ <= stamp && stamp <= P_.Tstop_ )
    {
      // calculate the effect of this spike immediately with respect to all
      // spikes in the past of the respectively other sources

      S_.n_events_[ sender ]++; // count this spike

      for ( SpikelistType::const_iterator spike_j = otherSpikes.begin(); spike_j != otherSpikes.end(); ++spike_j )
      {
        size_t bin;
        long other = spike_j->receptor_channel_;
        long sender_ind, other_ind;

        if ( spike_i < spike_j->timestep_ )
        {
          sender_ind = other;
          other_ind = sender;
        }
        else
        {
          sender_ind = sender;
          other_ind = other;
        }

        if ( sender_ind <= other_ind )
        {
          bin = -1. * std::floor( ( 0.5 * P_.delta_tau_.get_steps() - std::abs( spike_i - spike_j->timestep_ ) )
                        / P_.delta_tau_.get_steps() );
        }
        else
        {
          bin = std::floor( ( 0.5 * P_.delta_tau_.get_steps() + std::abs( spike_i - spike_j->timestep_ ) )
            / P_.delta_tau_.get_steps() );
        }

        if ( bin < S_.covariance_[ sender_ind ][ other_ind ].size() )
        {
          // weighted histogram
          S_.covariance_[ sender_ind ][ other_ind ][ bin ] += e.get_multiplicity() * e.get_weight() * spike_j->weight_;
          if ( bin == 0 && ( spike_i - spike_j->timestep_ != 0 || other != sender ) )
          {
            S_.covariance_[ other_ind ][ sender_ind ][ bin ] +=
              e.get_multiplicity() * e.get_weight() * spike_j->weight_;
          }
          // pure (unweighted) count histogram
          S_.count_covariance_[ sender_ind ][ other_ind ][ bin ] += e.get_multiplicity();
          if ( bin == 0 && ( spike_i - spike_j->timestep_ != 0 || other != sender ) )
          {
            S_.count_covariance_[ other_ind ][ sender_ind ][ bin ] += e.get_multiplicity();
          }
        }
      }

    } // t in [TStart, Tstop]

  } // device active
}
