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
#include "numerics.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

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

nest::ac_generator::State_::State_()
  : y_0_( 0.0 )
  , y_1_( 0.0 ) // pA
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
  ( *d )[ "y_0" ] = y_0_;
  ( *d )[ "y_1" ] = y_1_;
}

void
nest::ac_generator::Parameters_::set( const DictionaryDatum& d )
{
  updateValue< double >( d, names::amplitude, amp_ );
  updateValue< double >( d, names::offset, offset_ );
  updateValue< double >( d, names::frequency, freq_ );
  updateValue< double >( d, names::phase, phi_deg_ );
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::ac_generator::ac_generator()
  : Node()
  , device_()
  , P_()
  , S_()
{
}

nest::ac_generator::ac_generator( const ac_generator& n )
  : Node( n )
  , device_( n.device_ )
  , P_( n.P_ )
  , S_( n.S_ )
{
}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::ac_generator::init_state_( const Node& proto )
{
  const ac_generator& pr = downcast< ac_generator >( proto );

  device_.init_state( pr.device_ );
  S_ = pr.S_;
}

void
nest::ac_generator::init_buffers_()
{
  device_.init_buffers();
}

void
nest::ac_generator::calibrate()
{
  device_.calibrate();

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
  long start = origin.get_steps();

  CurrentEvent ce;
  for ( long lag = from; lag < to; ++lag )
  {
    if ( device_.is_active( Time::step( start + lag ) ) )
    {
      const double y_0 = S_.y_0_;
      S_.y_0_ = V_.A_00_ * y_0 + V_.A_01_ * S_.y_1_;
      S_.y_1_ = V_.A_10_ * y_0 + V_.A_11_ * S_.y_1_;
      ce.set_current( S_.y_1_ + P_.offset_ );
      kernel().event_delivery_manager.send( *this, ce, lag );
    }
  }
}
