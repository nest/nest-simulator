/*
 *  noise_generator.cpp
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

#include "noise_generator.h"

// Includes from libnestutil:
#include "dict_util.h"
#include "logging.h"
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

namespace nest
{
RecordablesMap< noise_generator > noise_generator::recordablesMap_;

template <>
void
RecordablesMap< noise_generator >::create()
{
  insert_( Name( names::I ), &noise_generator::get_I_avg_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::noise_generator::Parameters_::Parameters_()
  : mean_( 0.0 )    // pA
  , std_( 0.0 )     // pA / sqrt(s)
  , std_mod_( 0.0 ) // pA / sqrt(s)
  , freq_( 0.0 )    // Hz
  , phi_deg_( 0.0 ) // degree
  , dt_( Time::ms( 1.0 ) )
  , num_targets_( 0 )
{
}

nest::noise_generator::Parameters_::Parameters_( const Parameters_& p )
  : mean_( p.mean_ )
  , std_( p.std_ )
  , std_mod_( p.std_mod_ )
  , freq_( p.freq_ )
  , phi_deg_( p.phi_deg_ )
  , dt_( p.dt_ )
  , num_targets_( 0 ) // we do not copy connections
{
  // do not check validity of dt_ here, otherwise we cannot copy
  // to temporary in set(); see node copy c'tor
  dt_.calibrate();
}

nest::noise_generator::Parameters_& nest::noise_generator::Parameters_::operator=( const Parameters_& p )
{
  if ( this == &p )
  {
    return *this;
  }

  mean_ = p.mean_;
  std_ = p.std_;
  std_mod_ = p.std_mod_;
  freq_ = p.freq_;
  phi_deg_ = p.phi_deg_;
  dt_ = p.dt_;

  return *this;
}

nest::noise_generator::State_::State_()
  : y_0_( 0.0 )
  , y_1_( 0.0 )   // pA
  , I_avg_( 0.0 ) // pA
{
}

nest::noise_generator::Buffers_::Buffers_( noise_generator& n )
  : next_step_( 0 )
  , logger_( n )
{
}

nest::noise_generator::Buffers_::Buffers_( const Buffers_& b, noise_generator& n )
  : next_step_( b.next_step_ )
  , logger_( n )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::noise_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::mean ] = mean_;
  ( *d )[ names::std ] = std_;
  ( *d )[ names::std_mod ] = std_mod_;
  ( *d )[ names::dt ] = dt_.get_ms();
  ( *d )[ names::phase ] = phi_deg_;
  ( *d )[ names::frequency ] = freq_;
}

void
nest::noise_generator::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::y_0 ] = y_0_;
  ( *d )[ names::y_1 ] = y_1_;
}

void
nest::noise_generator::Parameters_::set( const DictionaryDatum& d, const noise_generator& n, Node* node )
{
  updateValueParam< double >( d, names::mean, mean_, node );
  updateValueParam< double >( d, names::std, std_, node );
  updateValueParam< double >( d, names::std_mod, std_mod_, node );
  updateValueParam< double >( d, names::frequency, freq_, node );
  updateValueParam< double >( d, names::phase, phi_deg_, node );
  double dt;
  if ( updateValueParam< double >( d, names::dt, dt, node ) )
  {
    dt_ = Time::ms( dt );
  }
  if ( std_ < 0 )
  {
    throw BadProperty( "The standard deviation cannot be negative." );
  }
  if ( std_mod_ < 0 )
  {
    throw BadProperty( "The standard deviation cannot be negative." );
  }
  if ( std_mod_ > std_ )
  {
    throw BadProperty(
      "The modulation apmlitude must be smaller or equal to the baseline "
      "amplitude." );
  }

  if ( not dt_.is_step() )
  {
    throw StepMultipleRequired( n.get_name(), names::dt, dt_ );
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::noise_generator::noise_generator()
  : StimulationDevice()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
  if ( not P_.dt_.is_step() )
  {
    throw InvalidDefaultResolution( get_name(), names::dt, P_.dt_ );
  }
}

nest::noise_generator::noise_generator( const noise_generator& n )
  : StimulationDevice( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
  if ( not P_.dt_.is_step() )
  {
    throw InvalidTimeInModel( get_name(), names::dt, P_.dt_ );
  }
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::noise_generator::init_state_()
{
  StimulationDevice::init_state();
}

void
nest::noise_generator::init_buffers_()
{
  StimulationDevice::init_buffers();
  B_.logger_.reset();

  B_.next_step_ = 0;
  B_.amps_.clear();
  B_.amps_.resize( P_.num_targets_, 0.0 );
}

void
nest::noise_generator::calibrate()
{
  B_.logger_.init();

  StimulationDevice::calibrate();
  if ( P_.num_targets_ != B_.amps_.size() )
  {
    LOG( M_INFO, "noise_generator::calibrate()", "The number of targets has changed, drawing new amplitudes." );
    init_buffers_();
  }

  V_.dt_steps_ = P_.dt_.get_steps();

  const double h = Time::get_resolution().get_ms();
  const double t = kernel().simulation_manager.get_time().get_ms();

  // scale Hz to ms
  const double omega = 2.0 * numerics::pi * P_.freq_ / 1000.0;
  const double phi_rad = P_.phi_deg_ * 2.0 * numerics::pi / 360.0;

  // initial state
  S_.y_0_ = std::cos( omega * t + phi_rad );
  S_.y_1_ = std::sin( omega * t + phi_rad );

  // matrix elements
  V_.A_00_ = std::cos( omega * h );
  V_.A_01_ = -std::sin( omega * h );
  V_.A_10_ = std::sin( omega * h );
  V_.A_11_ = std::cos( omega * h );
}


/* ----------------------------------------------------------------
 * Update function and event hook
 * ---------------------------------------------------------------- */

nest::port
nest::noise_generator::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool dummy_target )
{
  StimulationDevice::enforce_single_syn_type( syn_id );

  if ( dummy_target )
  {
    DSCurrentEvent e;
    e.set_sender( *this );
    return target.handles_test_event( e, receptor_type );
  }
  else
  {
    CurrentEvent e;
    e.set_sender( *this );
    const port p = target.handles_test_event( e, receptor_type );
    if ( p != invalid_port_ and not is_model_prototype() )
    {
      ++P_.num_targets_;
    }
    return p;
  }
}

//
// Time Evolution Operator
//
void
nest::noise_generator::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  const long start = origin.get_steps();

  for ( long offs = from; offs < to; ++offs )
  {
    S_.I_avg_ = 0.0;

    const long now = start + offs;

    if ( not StimulationDevice::is_active( Time::step( now ) ) )
    {
      B_.logger_.record_data( origin.get_steps() + offs );
      continue;
    }

    if ( P_.std_mod_ != 0. )
    {
      const double y_0 = S_.y_0_;
      S_.y_0_ = V_.A_00_ * y_0 + V_.A_01_ * S_.y_1_;
      S_.y_1_ = V_.A_10_ * y_0 + V_.A_11_ * S_.y_1_;
    }

    // >= in case we woke from inactivity
    if ( now >= B_.next_step_ )
    {
      // compute new currents
      for ( double& amp : B_.amps_ )
      {
        amp = P_.mean_
          + std::sqrt( P_.std_ * P_.std_ + S_.y_1_ * P_.std_mod_ * P_.std_mod_ )
            * V_.normal_dist_( get_vp_specific_rng( get_thread() ) );
      }
      // use now as reference, in case we woke up from inactive period
      B_.next_step_ = now + V_.dt_steps_;
    }

    // record values
    for ( double& amp : B_.amps_ )
    {
      S_.I_avg_ += amp;
    }
    S_.I_avg_ /= std::max( 1, int( B_.amps_.size() ) );
    B_.logger_.record_data( origin.get_steps() + offs );

    DSCurrentEvent ce;
    kernel().event_delivery_manager.send( *this, ce, offs );
  }
}

void
nest::noise_generator::event_hook( DSCurrentEvent& e )
{
  // get port number
  const port prt = e.get_port();

  // we handle only one port here, get reference to vector elem
  assert( 0 <= prt && static_cast< size_t >( prt ) < B_.amps_.size() );

  e.set_current( B_.amps_[ prt ] );
  e.get_receiver().handle( e );
}

void
nest::noise_generator::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::noise_generator::set_data_from_stimulation_backend( std::vector< double >& input_param )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.num_targets_ = P_.num_targets_;

  // For the input backend
  if ( not input_param.empty() )
  {
    if ( input_param.size() != 5 )
    {
      throw BadParameterValue(
        "The size of the data for the noise_generator needs to be 5 [mean, std, std_mod, frequency, phase]." );
    }
    DictionaryDatum d = DictionaryDatum( new Dictionary );
    ( *d )[ names::mean ] = DoubleDatum( input_param[ 0 ] );
    ( *d )[ names::std ] = DoubleDatum( input_param[ 1 ] );
    ( *d )[ names::std_mod ] = DoubleDatum( input_param[ 2 ] );
    ( *d )[ names::frequency ] = DoubleDatum( input_param[ 3 ] );
    ( *d )[ names::phase ] = DoubleDatum( input_param[ 4 ] );
    ptmp.set( d, *this, this );
  }

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
  P_.num_targets_ = ptmp.num_targets_;
}
