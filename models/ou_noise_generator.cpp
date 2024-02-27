/*
 *  ou_noise_generator.cpp
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

#include "ou_noise_generator.h"

// Includes from libnestutil:
#include "compose.hpp"
#include "dict_util.h"
#include "logging.h"
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "nest_impl.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"

namespace nest
{
void
register_ou_noise_generator( const std::string& name )
{
  register_node_model< ou_noise_generator >( name );
}

RecordablesMap< ou_noise_generator > ou_noise_generator::recordablesMap_;

template <>
void
RecordablesMap< ou_noise_generator >::create()
{
  insert_( Name( names::I ), &ou_noise_generator::get_I_avg_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::ou_noise_generator::Parameters_::Parameters_()
  : mean_( 0.0 )    // pA
  , std_( 0.0 )     // pA / sqrt(s)
  , tau_( 0.0 )    // ms
  , dt_( get_default_dt() )
  , num_targets_( 0 )
{
}

nest::ou_noise_generator::Parameters_::Parameters_( const Parameters_& p )
  : mean_( p.mean_ )
  , std_( p.std_ )
  , tau_( p.tau_ )
  , dt_( p.dt_ )
  , num_targets_( 0 ) // we do not copy connections
{
  if ( dt_.is_step() )
  {
    dt_.calibrate();
  }
  else
  {
    dt_ = get_default_dt();
  }
}

nest::ou_noise_generator::Parameters_&
nest::ou_noise_generator::Parameters_::operator=( const Parameters_& p )
{
  if ( this == &p )
  {
    return *this;
  }

  mean_ = p.mean_;
  std_ = p.std_;
  tau_ = p.tau_;
  dt_ = p.dt_;

  return *this;
}

nest::ou_noise_generator::State_::State_()
  : I_avg_( 0.0 ) // pA
{
}

nest::ou_noise_generator::Buffers_::Buffers_( ou_noise_generator& n )
  : next_step_( 0 )
  , logger_( n )
{
}

nest::ou_noise_generator::Buffers_::Buffers_( const Buffers_& b, ou_noise_generator& n )
  : next_step_( b.next_step_ )
  , logger_( n )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::ou_noise_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::mean ] = mean_;
  ( *d )[ names::std ] = std_;
  ( *d )[ names::dt ] = dt_.get_ms();
  ( *d )[ names::tau ] = tau_;
}

void
nest::ou_noise_generator::State_::get( DictionaryDatum& d ) const
{
}

void
nest::ou_noise_generator::Parameters_::set( const DictionaryDatum& d, const ou_noise_generator& n, Node* node )
{
  updateValueParam< double >( d, names::mean, mean_, node );
  updateValueParam< double >( d, names::std, std_, node );
  updateValueParam< double >( d, names::tau, tau_, node );
  double dt;
  if ( updateValueParam< double >( d, names::dt, dt, node ) )
  {
    dt_ = Time::ms( dt );
  }
  if ( std_ < 0 )
  {
    throw BadProperty( "The standard deviation cannot be negative." );
  }

  if ( not dt_.is_step() )
  {
    throw StepMultipleRequired( n.get_name(), names::dt, dt_ );
  }
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::ou_noise_generator::ou_noise_generator()
  : StimulationDevice()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::ou_noise_generator::ou_noise_generator( const ou_noise_generator& n )
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
nest::ou_noise_generator::init_state_()
{
  StimulationDevice::init_state();
}

void
nest::ou_noise_generator::init_buffers_()
{
  StimulationDevice::init_buffers();
  B_.logger_.reset();

  B_.next_step_ = 0;
  B_.amps_.clear();
  B_.amps_.resize( P_.num_targets_, 0.0 );
}

void
nest::ou_noise_generator::pre_run_hook()
{
  B_.logger_.init();

  StimulationDevice::pre_run_hook();
  if ( P_.num_targets_ != B_.amps_.size() )
  {
    LOG( M_INFO, "ou_noise_generator::pre_run_hook()", "The number of targets has changed, drawing new amplitudes." );
    init_buffers_();
  }

  V_.dt_steps_ = P_.dt_.get_steps();

  const double h = Time::get_resolution().get_ms();
  const double t = kernel().simulation_manager.get_time().get_ms();

  // scale Hz to ms
  const double noise_amp = P_.std_ *  std::sqrt(-1 * std::expm1(-2 * h / P_.tau_));
  const double prop = std::exp(-1 * h / P_.tau_);
  const double tau_inv = h / P_.tau_;

  V_.noise_amp_ = noise_amp;
  V_.prop_ = prop;
  V_.tau_inv_ = tau_inv;

}


/* ----------------------------------------------------------------
 * Update function and event hook
 * ---------------------------------------------------------------- */

size_t
nest::ou_noise_generator::send_test_event( Node& target, size_t receptor_type, synindex syn_id, bool dummy_target )
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
    const size_t p = target.handles_test_event( e, receptor_type );
    if ( p != invalid_port and not is_model_prototype() )
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
nest::ou_noise_generator::update( Time const& origin, const long from, const long to )
{
  const long start = origin.get_steps();

  for ( long offs = from; offs < to; ++offs )
  {

    const long now = start + offs;

    if ( not StimulationDevice::is_active( Time::step( now ) ) )
    {
      B_.logger_.record_data( origin.get_steps() + offs );
      continue;
    }

    // >= in case we woke from inactivity
    if ( now >= B_.next_step_ )
    {
      // std::cout << "I am in update" << std::endl;
      // compute new currents
      for ( double& amp : B_.amps_ )
      {
        amp = P_.mean_ * V_.tau_inv_
            + amp * V_.prop_ + V_.noise_amp_
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
nest::ou_noise_generator::event_hook( DSCurrentEvent& e )
{
  // get port number
  const size_t prt = e.get_port();

  // we handle only one port here, get reference to vector elem
  assert( prt < B_.amps_.size() );

  e.set_current( B_.amps_[ prt ] );
  e.get_receiver().handle( e );
}

void
nest::ou_noise_generator::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::ou_noise_generator::set_data_from_stimulation_backend( std::vector< double >& input_param )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors
  ptmp.num_targets_ = P_.num_targets_;

  // For the input backend
  if ( not input_param.empty() )
  {
    if ( input_param.size() != 3 )
    {
      throw BadParameterValue(
        "The size of the data for the ou_noise_generator needs to be 3 [mean, std, tau]." );
    }
    DictionaryDatum d = DictionaryDatum( new Dictionary );
    ( *d )[ names::mean ] = DoubleDatum( input_param[ 0 ] );
    ( *d )[ names::std ] = DoubleDatum( input_param[ 1 ] );
    ( *d )[ names::tau ] = DoubleDatum( input_param[ 2 ] );
    ptmp.set( d, *this, this );
  }

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
  P_.num_targets_ = ptmp.num_targets_;
}

void
nest::ou_noise_generator::calibrate_time( const TimeConverter& tc )
{
  if ( P_.dt_.is_step() )
  {
    P_.dt_ = tc.from_old_tics( P_.dt_.get_tics() );
  }
  else
  {
    const double old = P_.dt_.get_ms();
    P_.dt_ = P_.get_default_dt();
    std::string msg = String::compose( "Default for dt changed from %1 to %2 ms", old, P_.dt_.get_ms() );
    LOG( M_INFO, get_name(), msg );
  }
}
