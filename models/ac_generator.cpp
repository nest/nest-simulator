/*
 *  ac_generator.cpp
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

#include "ac_generator.h"

// C++ includes:
#include <cmath>

// Includes from libnestutil:
#include "dict_util.h"
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
RecordablesMap< ac_generator > ac_generator::recordablesMap_;

template <>
void
RecordablesMap< ac_generator >::create()
{
  insert_( Name( names::I ), &ac_generator::get_I_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::ac_generator::Parameters_::Parameters_()
  : amp_( 0.0 )     // pA
  , offset_( 0.0 )  // pA
  , freq_( 0.0 )    // Hz
  , phi_deg_( 0.0 ) // degree
{
}

nest::ac_generator::Parameters_::Parameters_( const Parameters_& p )
  : amp_( p.amp_ )
  , offset_( p.offset_ )
  , freq_( p.freq_ )
  , phi_deg_( p.phi_deg_ )
{
}

nest::ac_generator::Parameters_& nest::ac_generator::Parameters_::operator=( const Parameters_& p )
{
  if ( this == &p )
  {
    return *this;
  }

  amp_ = p.amp_;
  offset_ = p.offset_;
  freq_ = p.freq_;
  phi_deg_ = p.phi_deg_;

  return *this;
}

nest::ac_generator::State_::State_()
  : y_0_( 0.0 )
  , y_1_( 0.0 ) // pA
  , I_( 0.0 )   // pA
{
}

nest::ac_generator::Buffers_::Buffers_( ac_generator& n )
  : logger_( n )
{
}

nest::ac_generator::Buffers_::Buffers_( const Buffers_&, ac_generator& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::ac_generator::Parameters_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::amplitude ] = amp_;
  ( *d )[ names::offset ] = offset_;
  ( *d )[ names::phase ] = phi_deg_;
  ( *d )[ names::frequency ] = freq_;
}

void
nest::ac_generator::State_::get( DictionaryDatum& d ) const
{
  ( *d )[ names::y_0 ] = y_0_;
  ( *d )[ names::y_1 ] = y_1_;
}

void
nest::ac_generator::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::amplitude, amp_, node );
  updateValueParam< double >( d, names::offset, offset_, node );
  updateValueParam< double >( d, names::frequency, freq_, node );
  updateValueParam< double >( d, names::phase, phi_deg_, node );
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::ac_generator::ac_generator()
  : StimulationDevice()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::ac_generator::ac_generator( const ac_generator& n )
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
nest::ac_generator::init_state_()
{
  StimulationDevice::init_state();
}

void
nest::ac_generator::init_buffers_()
{
  StimulationDevice::init_buffers();
  B_.logger_.reset();
}

void
nest::ac_generator::calibrate()
{
  B_.logger_.init();

  StimulationDevice::calibrate();

  const double h = Time::get_resolution().get_ms();
  const double t = kernel().simulation_manager.get_time().get_ms();

  // scale Hz to ms
  const double omega = 2.0 * numerics::pi * P_.freq_ / 1000.0;
  const double phi_rad = P_.phi_deg_ * 2.0 * numerics::pi / 360.0;

  // initial state
  S_.y_0_ = P_.amp_ * std::cos( omega * t + phi_rad );
  S_.y_1_ = P_.amp_ * std::sin( omega * t + phi_rad );

  // matrix elements
  V_.A_00_ = std::cos( omega * h );
  V_.A_01_ = -std::sin( omega * h );
  V_.A_10_ = std::sin( omega * h );
  V_.A_11_ = std::cos( omega * h );
}

void
nest::ac_generator::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  long start = origin.get_steps();

  CurrentEvent ce;
  for ( long lag = from; lag < to; ++lag )
  {
    // We need to iterate the oscillator throughout all steps, even when the
    // device is not active, since inactivity only windows the oscillator.
    const double y_0 = S_.y_0_;
    S_.y_0_ = V_.A_00_ * y_0 + V_.A_01_ * S_.y_1_;
    S_.y_1_ = V_.A_10_ * y_0 + V_.A_11_ * S_.y_1_;

    S_.I_ = 0.0;
    if ( StimulationDevice::is_active( Time::step( start + lag ) ) )
    {
      S_.I_ = S_.y_1_ + P_.offset_;
      ce.set_current( S_.I_ );
      kernel().event_delivery_manager.send( *this, ce, lag );
    }
    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
nest::ac_generator::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

/* ----------------------------------------------------------------
 * Other functions
 * ---------------------------------------------------------------- */

void
nest::ac_generator::set_data_from_stimulation_backend( std::vector< double >& input_param )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  // For the input backend
  if ( not input_param.empty() )
  {
    if ( input_param.size() != 4 )
    {
      throw BadParameterValue(
        "The size of the data for the ac_generator needs to be 4 [amplitude, offset, frequency, phase]." );
    }
    DictionaryDatum d = DictionaryDatum( new Dictionary );
    ( *d )[ names::amplitude ] = DoubleDatum( input_param[ 0 ] );
    ( *d )[ names::offset ] = DoubleDatum( input_param[ 1 ] );
    ( *d )[ names::frequency ] = DoubleDatum( input_param[ 2 ] );
    ( *d )[ names::phase ] = DoubleDatum( input_param[ 3 ] );
    ptmp.set( d, this );
  }

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}
