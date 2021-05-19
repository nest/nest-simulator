/*
 *  correlation_detector.cpp
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

#include "correlation_detector.h"

// C++ includes:
#include <cmath>      // for less
#include <functional> // for bind2nd
#include <numeric>

// Includes from libnestutil:
#include "dict_util.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::correlation_detector::Parameters_::Parameters_()
  : delta_tau_( Time::ms( 1.0 ) )
  , tau_max_( 10 * delta_tau_ )
  , Tstart_( Time::ms( 0.0 ) )
  , Tstop_( Time::pos_inf() )
{
}

nest::correlation_detector::Parameters_::Parameters_( const Parameters_& p )
  : delta_tau_( p.delta_tau_ )
  , tau_max_( p.tau_max_ )
  , Tstart_( p.Tstart_ )
  , Tstop_( p.Tstop_ )
{
  // Check for proper properties is not done here but in the
  // correlation_detector() copy c'tor. The check cannot be
  // placed here, since this c'tor is also used to copy to
  // temporaries in correlation_detector::set_status().
  // If we checked for errors here, we could never change values
  // that have become invalid after a resolution change.
  delta_tau_.calibrate();
  tau_max_.calibrate();
  Tstart_.calibrate();
  Tstop_.calibrate();
}

nest::correlation_detector::Parameters_& nest::correlation_detector::Parameters_::operator=( const Parameters_& p )
{
  delta_tau_ = p.delta_tau_;
  tau_max_ = p.tau_max_;
  Tstart_ = p.Tstart_;
  Tstop_ = p.Tstop_;

  delta_tau_.calibrate();
  tau_max_.calibrate();
  Tstart_.calibrate();
  Tstop_.calibrate();

  return *this;
}

nest::correlation_detector::State_::State_()
  : n_events_( 2, 0 )
  , incoming_( 2 )
  , histogram_()
  , histogram_correction_()
  , count_histogram_()
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::correlation_detector::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::delta_tau ] = delta_tau_.get_ms();
  ( *d )[ names::tau_max ] = tau_max_.get_ms();
  ( *d )[ names::Tstart ] = Tstart_.get_ms();
  ( *d )[ names::Tstop ] = Tstop_.get_ms();
}

void
nest::correlation_detector::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::n_events ] = IntVectorDatum( new std::vector< long >( n_events_ ) );
  ( *d )[ names::histogram ] = DoubleVectorDatum( new std::vector< double >( histogram_ ) );
  ( *d )[ names::histogram_correction ] = DoubleVectorDatum( new std::vector< double >( histogram_correction_ ) );
  ( *d )[ names::count_histogram ] = IntVectorDatum( new std::vector< long >( count_histogram_ ) );
}

bool
nest::correlation_detector::Parameters_::set( const DictionaryDatum& d, const correlation_detector& n, Node* node )
{
  bool reset = false;
  double t;
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

  return reset;
}

void
nest::correlation_detector::State_::set( const DictionaryDatum& d, const Parameters_& p, bool reset_required, Node* )
{
  std::vector< long > nev;
  if ( updateValue< std::vector< long > >( d, names::n_events, nev ) )
  {
    if ( nev.size() == 2 && nev[ 0 ] == 0 && nev[ 1 ] == 0 )
    {
      reset_required = true;
    }
    else
    {
      throw BadProperty( "/n_events can only be set to [0 0]." );
    }
  }
  if ( reset_required )
  {
    reset( p );
  }
}

void
nest::correlation_detector::State_::reset( const Parameters_& p )
{
  n_events_.clear();
  n_events_.resize( 2, 0 );

  incoming_.clear();
  incoming_.resize( 2 );

  assert( p.tau_max_.is_multiple_of( p.delta_tau_ ) );
  histogram_.clear();
  histogram_.resize( 1 + 2 * p.tau_max_.get_steps() / p.delta_tau_.get_steps(), 0 );

  histogram_correction_.clear();
  histogram_correction_.resize( 1 + 2 * p.tau_max_.get_steps() / p.delta_tau_.get_steps(), 0 );

  count_histogram_.clear();
  count_histogram_.resize( 1 + 2 * p.tau_max_.get_steps() / p.delta_tau_.get_steps(), 0 );
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::correlation_detector::correlation_detector()
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

nest::correlation_detector::correlation_detector( const correlation_detector& n )
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
nest::correlation_detector::init_state_()
{
  device_.init_state();
}

void
nest::correlation_detector::init_buffers_()
{
  device_.init_buffers();
  S_.reset( P_ );
}

void
nest::correlation_detector::calibrate()
{
  device_.calibrate();
}


/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::correlation_detector::update( Time const&, const long, const long )
{
}

void
nest::correlation_detector::handle( SpikeEvent& e )
{
  // The receiver port identifies the sending node in our
  // sender list.
  const rport sender = e.get_rport();

  // If this assertion breaks, the sender does not honor the
  // receiver port during connection or sending.
  assert( 0 <= sender && sender <= 1 );

  // accept spikes only if detector was active when spike was emitted
  Time const stamp = e.get_stamp();

  if ( device_.is_active( stamp ) )
  {

    const long spike_i = stamp.get_steps();
    const port other = 1 - sender; // port of the neuron not sending
    SpikelistType& otherSpikes = S_.incoming_[ other ];
    const double tau_edge = P_.tau_max_.get_steps() + 0.5 * P_.delta_tau_.get_steps();

    // throw away all spikes of the other neuron which are too old to
    // enter the correlation window
    // subtract 0.5*other to make left interval closed, keep right interval open
    while ( not otherSpikes.empty() && ( spike_i - otherSpikes.front().timestep_ ) - 0.5 * other >= tau_edge )
    {
      otherSpikes.pop_front();
    }
    // all remaining spike times in the queue are
    //     >= spike_i - tau_edge, if sender = 0
    // all remaining spike times in the queue are
    //     > spike_i - tau_edge, if sender = 1

    // temporary variables for Kahan summation algorithm
    double y, t;

    // only count events in histogram, if the current event is within the time
    // window [Tstart, Tstop]
    // this is needed in order to prevent boundary effects
    if ( P_.Tstart_ <= stamp && stamp <= P_.Tstop_ )
    {
      // calculate the effect of this spike immediately with respect to all
      // spikes in the past of the respectively other source
      // if source 1 and source 2 produce a spike at the same time
      // it will not be counted twice, since handle() will be called
      // subsequently for both spikes, such that the first spike arriving here
      // will not yet be aware of the spike arriving as second
      // (which is not yet in the deque)
      S_.n_events_[ sender ]++; // count this spike

      const long sign = 2 * sender - 1; // takes into account relative timing
                                        // of spike from source 1 and source 2

      for ( SpikelistType::const_iterator spike_j = otherSpikes.begin(); spike_j != otherSpikes.end(); ++spike_j )
      {
        const size_t bin = static_cast< size_t >(
          std::floor( ( tau_edge + sign * ( spike_i - spike_j->timestep_ ) ) / P_.delta_tau_.get_steps() ) );
        assert( bin < S_.histogram_.size() );
        assert( bin < S_.histogram_correction_.size() );
        // weighted histogram with Kahan summation algorithm
        y = e.get_multiplicity() * e.get_weight() * spike_j->weight_ - S_.histogram_correction_[ bin ];
        t = S_.histogram_[ bin ] + y;
        S_.histogram_correction_[ bin ] = ( t - S_.histogram_[ bin ] ) - y;
        S_.histogram_[ bin ] = t;

        // pure (unweighted) count histogram
        S_.count_histogram_[ bin ] += e.get_multiplicity();
      }

    } // t in [TStart, Tstop]

    // store the spike time in the according deque
    // spikes are not guaranteed to arrive in temporal order,
    // so make an insertion sort

    // find first appearence of element which is greater than spike_i
    const Spike_ sp_i( spike_i, e.get_multiplicity() * e.get_weight() );
    SpikelistType::iterator insert_pos = std::find_if( S_.incoming_[ sender ].begin(),
      S_.incoming_[ sender ].end(),
      std::bind( std::greater< Spike_ >(), std::placeholders::_1, sp_i ) );

    // insert before the position we have found
    // if no element greater found, insert_pos == end(), so append at the end of
    // the deque
    S_.incoming_[ sender ].insert( insert_pos, sp_i );
  } // device active
}
