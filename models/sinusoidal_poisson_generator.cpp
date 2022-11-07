/*
 *  sinusoidal_poisson_generator.cpp
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


#include "sinusoidal_poisson_generator.h"

// C++ includes:
#include <cmath>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "booldatum.h"
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"

namespace nest
{
RecordablesMap< sinusoidal_poisson_generator > sinusoidal_poisson_generator::recordablesMap_;

template <>
void
RecordablesMap< sinusoidal_poisson_generator >::create()
{
  insert_( Name( names::rate ), &sinusoidal_poisson_generator::get_rate_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::sinusoidal_poisson_generator::Parameters_::Parameters_()
  : om_( 0.0 )        // radian/ms
  , phi_( 0.0 )       // radian
  , rate_( 0.0 )      // spikes/ms
  , amplitude_( 0.0 ) // spikes/ms
  , individual_spike_trains_( true )
{
}

nest::sinusoidal_poisson_generator::Parameters_::Parameters_( const Parameters_& p )
  : om_( p.om_ )
  , phi_( p.phi_ )
  , rate_( p.rate_ )
  , amplitude_( p.amplitude_ )
  , individual_spike_trains_( p.individual_spike_trains_ )
{
}

nest::sinusoidal_poisson_generator::Parameters_&
nest::sinusoidal_poisson_generator::Parameters_::operator=( const Parameters_& p )
{
  if ( this == &p )
  {
    return *this;
  }

  rate_ = p.rate_;
  om_ = p.om_;
  phi_ = p.phi_;
  amplitude_ = p.amplitude_;
  individual_spike_trains_ = p.individual_spike_trains_;

  return *this;
}

nest::sinusoidal_poisson_generator::State_::State_()
  : y_0_( 0 )
  , y_1_( 0 )
  , rate_( 0 )
{
}


nest::sinusoidal_poisson_generator::Buffers_::Buffers_( sinusoidal_poisson_generator& n )
  : logger_( n )
{
}

nest::sinusoidal_poisson_generator::Buffers_::Buffers_( const Buffers_&, sinusoidal_poisson_generator& n )
  : logger_( n )
{
}


/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::sinusoidal_poisson_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::rate ] = rate_ * 1000.0;
  ( *d )[ names::frequency ] = om_ / ( 2.0 * numerics::pi / 1000.0 );
  ( *d )[ names::phase ] = 180.0 / numerics::pi * phi_;
  ( *d )[ names::amplitude ] = amplitude_ * 1000.0;
  ( *d )[ names::individual_spike_trains ] = individual_spike_trains_;
}

void
nest::sinusoidal_poisson_generator::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::y_0 ] = y_0_;
  ( *d )[ names::y_1 ] = y_1_;
}

void
nest::sinusoidal_poisson_generator::Parameters_::set( const DictionaryDatum& d,
  const sinusoidal_poisson_generator& n,
  Node* node )
{
  if ( not n.is_model_prototype() and d->known( names::individual_spike_trains ) )
  {
    throw BadProperty(
      "The individual_spike_trains property can only be set as"
      " a model default using SetDefaults or upon CopyModel." );
  }

  updateValue< bool >( d, names::individual_spike_trains, individual_spike_trains_ );

  if ( updateValueParam< double >( d, names::rate, rate_, node ) )
  {
    rate_ /= 1000.0; // scale to ms^-1
  }

  if ( updateValueParam< double >( d, names::frequency, om_, node ) )
  {
    om_ *= 2.0 * numerics::pi / 1000.0;
  }

  if ( updateValueParam< double >( d, names::phase, phi_, node ) )
  {
    phi_ *= numerics::pi / 180.0;
  }

  if ( updateValueParam< double >( d, names::amplitude, amplitude_, node ) )
  {
    amplitude_ /= 1000.0;
  }
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::sinusoidal_poisson_generator::sinusoidal_poisson_generator()
  : StimulationDevice()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::sinusoidal_poisson_generator::sinusoidal_poisson_generator( const sinusoidal_poisson_generator& n )
  : StimulationDevice( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::sinusoidal_poisson_generator::init_state_()
{
  StimulationDevice::init_state();
}

void
nest::sinusoidal_poisson_generator::init_buffers_()
{
  StimulationDevice::init_buffers();
  B_.logger_.reset();
}

void
nest::sinusoidal_poisson_generator::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();

  StimulationDevice::pre_run_hook();

  // time resolution
  V_.h_ = Time::get_resolution().get_ms();
  const double t = kernel().simulation_manager.get_time().get_ms();

  // initial state
  S_.y_0_ = P_.amplitude_ * std::cos( P_.om_ * t + P_.phi_ );
  S_.y_1_ = P_.amplitude_ * std::sin( P_.om_ * t + P_.phi_ );

  V_.sin_ = std::sin( V_.h_ * P_.om_ ); // block elements
  V_.cos_ = std::cos( V_.h_ * P_.om_ );
}

void
nest::sinusoidal_poisson_generator::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 and ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  const long start = origin.get_steps();

  // random number generator
  RngPtr rng = get_vp_specific_rng( get_thread() );

  // We iterate the dynamics even when the device is turned off,
  // but do not issue spikes while it is off. In this way, the
  // oscillators always have the right phase.  This is quite
  // time-consuming, so it should be done only if the device is
  // on most of the time.

  for ( long lag = from; lag < to; ++lag )
  {
    // update oscillator blocks, accumulate rate as sum of DC and N_osc_ AC
    // elements rate is instantaneous sum of state
    S_.rate_ = P_.rate_;

    const double new_y_0 = V_.cos_ * S_.y_0_ - V_.sin_ * S_.y_1_;

    S_.y_1_ = V_.sin_ * S_.y_0_ + V_.cos_ * S_.y_1_;
    S_.y_0_ = new_y_0;
    S_.rate_ += S_.y_1_;

    if ( S_.rate_ < 0 )
    {
      S_.rate_ = 0;
    }

    // create spikes
    if ( S_.rate_ > 0 and StimulationDevice::is_active( Time::step( start + lag ) ) )
    {
      if ( P_.individual_spike_trains_ )
      {
        DSSpikeEvent se;
        kernel().event_delivery_manager.send( *this, se, lag );
      }
      else
      {
        poisson_distribution::param_type param( S_.rate_ * V_.h_ );
        long n_spikes = V_.poisson_dist_( rng, param );
        SpikeEvent se;
        se.set_multiplicity( n_spikes );
        kernel().event_delivery_manager.send( *this, se, lag );
      }
    }
    // store rate in Hz
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::sinusoidal_poisson_generator::event_hook( DSSpikeEvent& e )
{
  poisson_distribution::param_type param( S_.rate_ * V_.h_ );
  long n_spikes = V_.poisson_dist_( get_vp_specific_rng( get_thread() ), param );

  if ( n_spikes > 0 ) // we must not send events with multiplicity 0
  {
    e.set_multiplicity( n_spikes );
    e.get_receiver().handle( e );
  }
}

void
nest::sinusoidal_poisson_generator::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::sinusoidal_poisson_generator::set_data_from_stimulation_backend( std::vector< double >& input_param )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  // For the input backend
  if ( not input_param.empty() )
  {
    if ( input_param.size() != 5 )
    {
      throw BadParameterValue(
        "The size of the data for the sinusoidal_gamma_generator needs to 5 "
        "[rate, frequency, phase, amplitude, individual_spike_trains]." );
    }
    DictionaryDatum d = DictionaryDatum( new Dictionary );
    ( *d )[ names::rate ] = DoubleDatum( input_param[ 0 ] );
    ( *d )[ names::frequency ] = DoubleDatum( input_param[ 1 ] );
    ( *d )[ names::phase ] = DoubleDatum( input_param[ 2 ] );
    ( *d )[ names::amplitude ] = DoubleDatum( input_param[ 3 ] );
    ( *d )[ names::individual_spike_trains ] = BoolDatum( input_param[ 4 ] );
    ptmp.set( d, *this, this );
  }

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}
