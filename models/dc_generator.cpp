/*
 *  dc_generator.cpp
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

#include "dc_generator.h"

// Includes from nestkernel:
#include "event_delivery_manager_impl.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from libnestutil:
#include "dict_util.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"

namespace nest
{
RecordablesMap< dc_generator > dc_generator::recordablesMap_;

template <>
void
RecordablesMap< dc_generator >::create()
{
  insert_( Name( names::I ), &dc_generator::get_I_ );
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameter
 * ---------------------------------------------------------------- */

nest::dc_generator::Parameters_::Parameters_()
  : amp_( 0.0 ) // pA
{
}

nest::dc_generator::Parameters_::Parameters_( const Parameters_& p )
  : amp_( p.amp_ )
{
}

nest::dc_generator::Parameters_& nest::dc_generator::Parameters_::operator=( const Parameters_& p )
{
  if ( this == &p )
  {
    return *this;
  }

  amp_ = p.amp_;

  return *this;
}

nest::dc_generator::State_::State_()
  : I_( 0.0 ) // pA
{
}


nest::dc_generator::Buffers_::Buffers_( dc_generator& n )
  : logger_( n )
{
}

nest::dc_generator::Buffers_::Buffers_( const Buffers_&, dc_generator& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Parameter extraction and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::dc_generator::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::amplitude, amp_ );
}

void
nest::dc_generator::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::amplitude, amp_, node );
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::dc_generator::dc_generator()
  : StimulationDevice()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::dc_generator::dc_generator( const dc_generator& n )
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
nest::dc_generator::init_state_()
{
  StimulationDevice::init_state();
}

void
nest::dc_generator::init_buffers_()
{
  StimulationDevice::init_buffers();
  B_.logger_.reset();
}

void
nest::dc_generator::calibrate()
{
  B_.logger_.init();

  StimulationDevice::calibrate();
}


/* ----------------------------------------------------------------
 * Update function
 * ---------------------------------------------------------------- */

void
nest::dc_generator::update( Time const& origin, const long from, const long to )
{
  assert( to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  long start = origin.get_steps();

  CurrentEvent ce;
  ce.set_current( P_.amp_ );
  for ( long offs = from; offs < to; ++offs )
  {
    S_.I_ = 0.0;
    if ( StimulationDevice::is_active( Time::step( start + offs ) ) )
    {
      S_.I_ = P_.amp_;
      kernel().event_delivery_manager.send( *this, ce, offs );
    }
    B_.logger_.record_data( origin.get_steps() + offs );
  }
}

void
nest::dc_generator::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

void
nest::dc_generator::set_data_from_stimulation_backend( std::vector< double >& input_param )
{
  Parameters_ ptmp = P_; // temporary copy in case of errors

  // For the input backend
  if ( not input_param.empty() )
  {
    if ( input_param.size() != 1 )
    {
      throw BadParameterValue( "The size of the data for the dc_generator needs to be 1 [amplitude]." );
    }
    DictionaryDatum d = DictionaryDatum( new Dictionary );
    ( *d )[ names::amplitude ] = DoubleDatum( input_param[ 0 ] );
    ptmp.set( d, this );
  }

  // if we get here, temporary contains consistent set of properties
  P_ = ptmp;
}
