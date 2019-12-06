/*
 *  sinusoidal_gamma_generator.cpp
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

#include "sinusoidal_gamma_generator.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cmath>
#include <limits>

// External includes:
#include <gsl/gsl_sf_gamma.h>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "arraydatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

namespace nest
{
RecordablesMap< sinusoidal_gamma_generator > sinusoidal_gamma_generator::recordablesMap_;

template <>
void
RecordablesMap< sinusoidal_gamma_generator >::create()
{
  insert_( names::rate, &sinusoidal_gamma_generator::get_rate_ );
}
}


nest::sinusoidal_gamma_generator::Parameters_::Parameters_()
  : om_( 0.0 )  // radian/ms
  , phi_( 0.0 ) // radian
  , order_( 1.0 )
  , rate_( 0.0 )      // spikes/ms
  , amplitude_( 0.0 ) // spikes/ms
  , individual_spike_trains_( true )
  , num_trains_( 0 )
{
}

nest::sinusoidal_gamma_generator::Parameters_::Parameters_( const Parameters_& p )
  : om_( p.om_ )
  , phi_( p.phi_ )
  , order_( p.order_ )
  , rate_( p.rate_ )
  , amplitude_( p.amplitude_ )
  , individual_spike_trains_( p.individual_spike_trains_ )
  , num_trains_( p.num_trains_ )
{
}

nest::sinusoidal_gamma_generator::Parameters_& nest::sinusoidal_gamma_generator::Parameters_::operator=(
  const Parameters_& p )
{
  if ( this == &p )
  {
    return *this;
  }

  om_ = p.om_;
  phi_ = p.phi_;
  order_ = p.order_;
  rate_ = p.rate_;
  amplitude_ = p.amplitude_;
  individual_spike_trains_ = p.individual_spike_trains_;
  num_trains_ = p.num_trains_;

  return *this;
}

nest::sinusoidal_gamma_generator::State_::State_()
  : rate_( 0 )
{
}


nest::sinusoidal_gamma_generator::Buffers_::Buffers_( sinusoidal_gamma_generator& n )
  : logger_( n )
  , t0_ms_()
  , // will be set in init_buffers_
  Lambda_t0_()
  ,               // will be set in init_buffers_
  P_prev_( n.P_ ) // when creating Buffer, base on current parameters
{
}

nest::sinusoidal_gamma_generator::Buffers_::Buffers_( const Buffers_& b, sinusoidal_gamma_generator& n )
  : logger_( n )
  , t0_ms_( b.t0_ms_ )
  , Lambda_t0_( b.Lambda_t0_ )
  , P_prev_( b.P_prev_ )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::sinusoidal_gamma_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::rate ] = rate_ * 1000.0;
  ( *d )[ names::frequency ] = om_ / ( 2.0 * numerics::pi / 1000.0 );
  ( *d )[ names::phase ] = 180.0 / numerics::pi * phi_;
  ( *d )[ names::amplitude ] = amplitude_ * 1000.0;
  ( *d )[ names::order ] = order_;
  ( *d )[ names::individual_spike_trains ] = individual_spike_trains_;
}

void
nest::sinusoidal_gamma_generator::State_::get( DictionaryDatum& ) const
{
}

void
nest::sinusoidal_gamma_generator::Parameters_::set( const DictionaryDatum& d,
  const sinusoidal_gamma_generator& n,
  Node* node )
{
  if ( not n.is_model_prototype() && d->known( names::individual_spike_trains ) )
  {
    throw BadProperty(
      "The individual_spike_trains property can only be set as"
      " a model default using SetDefaults or upon CopyModel." );
  }

  if ( updateValue< bool >( d, names::individual_spike_trains, individual_spike_trains_ ) )
  {
    // this can happen only on model prototypes
    if ( individual_spike_trains_ )
    {
      // will be counted up as connections are made
      num_trains_ = 0;
    }
    else
    {
      // fixed
      num_trains_ = 1;
    }
  }

  if ( updateValueParam< double >( d, names::frequency, om_, node ) )
  {
    om_ *= 2.0 * numerics::pi / 1000.0;
  }

  if ( updateValueParam< double >( d, names::phase, phi_, node ) )
  {
    phi_ *= numerics::pi / 180.0;
  }

  if ( updateValueParam< double >( d, names::order, order_, node ) )
  {
    if ( order_ < 1.0 )
    {
      throw BadProperty( "The gamma order must be at least 1." );
    }
  }

  /* The *_unscaled variables here are introduced to avoid spurious
     floating-point comparison issues under 32-bit Linux.
  */
  double dc_unscaled = 1e3 * rate_;
  if ( updateValueParam< double >( d, names::rate, dc_unscaled, node ) )
  {
    rate_ = 1e-3 * dc_unscaled; // scale to 1/ms
  }

  double ac_unscaled = 1e3 * amplitude_;
  if ( updateValueParam< double >( d, names::amplitude, ac_unscaled, node ) )
  {
    amplitude_ = 1e-3 * ac_unscaled; // scale to 1/ms
  }

  if ( not( 0.0 <= ac_unscaled and ac_unscaled <= dc_unscaled ) )
  {
    throw BadProperty( "Rate parameters must fulfill 0 <= amplitude <= rate." );
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::sinusoidal_gamma_generator::sinusoidal_gamma_generator()
  : DeviceNode()
  , device_()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::sinusoidal_gamma_generator::sinusoidal_gamma_generator( const sinusoidal_gamma_generator& n )
  : DeviceNode( n )
  , device_( n.device_ )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::sinusoidal_gamma_generator::init_state_( const Node& proto )
{
  const sinusoidal_gamma_generator& pr = downcast< sinusoidal_gamma_generator >( proto );

  device_.init_state( pr.device_ );
  S_ = pr.S_;
}

void
nest::sinusoidal_gamma_generator::init_buffers_()
{
  device_.init_buffers();
  B_.logger_.reset();

  std::vector< double >( P_.num_trains_, kernel().simulation_manager.get_time().get_ms() ).swap( B_.t0_ms_ );
  std::vector< double >( P_.num_trains_, 0.0 ).swap( B_.Lambda_t0_ );
  B_.P_prev_ = P_;
}

// ----------------------------------------------------

inline double
nest::sinusoidal_gamma_generator::deltaLambda_( const Parameters_& p, double t_a, double t_b ) const
{
  if ( t_a == t_b )
  {
    return 0.0;
  }

  double deltaLambda = p.order_ * p.rate_ * ( t_b - t_a );
  if ( std::abs( p.amplitude_ ) > 0 && std::abs( p.om_ ) > 0 )
  {
    deltaLambda +=
      -p.order_ * p.amplitude_ / p.om_ * ( std::cos( p.om_ * t_b + p.phi_ ) - std::cos( p.om_ * t_a + p.phi_ ) );
  }
  return deltaLambda;
}

// ----------------------------------------------------

void
nest::sinusoidal_gamma_generator::calibrate()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();
  device_.calibrate();

  V_.h_ = Time::get_resolution().get_ms();
  V_.rng_ = kernel().rng_manager.get_rng( get_thread() );

  const double t_ms = kernel().simulation_manager.get_time().get_ms();

  // if new connections were created during simulation break, resize accordingly
  // this is a no-op if no new connections were created
  B_.t0_ms_.resize( P_.num_trains_, t_ms );
  B_.Lambda_t0_.resize( P_.num_trains_, 0.0 );

  // compute Lambda up to current time and store
  // this is a no-op for any new connections
  for ( size_t i = 0; i < P_.num_trains_; ++i )
  {
    B_.Lambda_t0_[ i ] += deltaLambda_( B_.P_prev_, B_.t0_ms_[ i ], t_ms );
    B_.t0_ms_[ i ] = t_ms;
  }
  B_.P_prev_ = P_;
}

double
nest::sinusoidal_gamma_generator::hazard_( port tgt_idx ) const
{
  // Note: We compute Lambda for the entire interval since the last spike/
  //       parameter change each time for better accuracy.
  const double Lambda = B_.Lambda_t0_[ tgt_idx ] + deltaLambda_( P_, B_.t0_ms_[ tgt_idx ], V_.t_ms_ );
  return V_.h_ * P_.order_ * S_.rate_ * std::pow( Lambda, P_.order_ - 1 ) * std::exp( -Lambda )
    / gsl_sf_gamma_inc( P_.order_, Lambda );
}

void
nest::sinusoidal_gamma_generator::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    const Time t = Time( Time::step( origin.get_steps() + lag + 1 ) );
    V_.t_ms_ = t.get_ms();
    V_.t_steps_ = t.get_steps();

    S_.rate_ = P_.rate_ + P_.amplitude_ * std::sin( P_.om_ * V_.t_ms_ + P_.phi_ );

    // t_steps_-1 since t_steps is end of interval, while activity det by start
    if ( P_.num_trains_ > 0 && S_.rate_ > 0 && device_.is_active( Time::step( V_.t_steps_ - 1 ) ) )
    {
      if ( P_.individual_spike_trains_ )
      {
        DSSpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
      else
      {
        if ( V_.rng_->drand() < hazard_( 0 ) )
        {
          SpikeEvent se;
          kernel().event_delivery_manager.send( *this, se, lag );
          B_.t0_ms_[ 0 ] = V_.t_ms_;
          B_.Lambda_t0_[ 0 ] = 0;
        }
      }
    }
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::sinusoidal_gamma_generator::event_hook( DSSpikeEvent& e )
{
  // get port number --- see #737
  const port tgt_idx = e.get_port();
  assert( 0 <= tgt_idx && static_cast< size_t >( tgt_idx ) < B_.t0_ms_.size() );

  if ( V_.rng_->drand() < hazard_( tgt_idx ) )
  {
    e.get_receiver().handle( e );
    B_.t0_ms_[ tgt_idx ] = V_.t_ms_;
    B_.Lambda_t0_[ tgt_idx ] = 0;
  }
}

void
nest::sinusoidal_gamma_generator::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
