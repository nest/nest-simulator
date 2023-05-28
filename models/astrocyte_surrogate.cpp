/*
 *  astrocyte_surrogate.cpp
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


#include "astrocyte_surrogate.h"

#ifdef HAVE_GSL

// C++ includes:
#include <cmath> // in case we need isnan() // fabs
#include <cstdio>
#include <iostream>

// Includes from libnestutil:
#include "dict_util.h"
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dictutils.h"

nest::RecordablesMap< nest::astrocyte_surrogate > nest::astrocyte_surrogate::recordablesMap_;

namespace nest
{
// Override the create() method with one call to RecordablesMap::insert_()
// for each quantity to be recorded.
template <>
void
RecordablesMap< astrocyte_surrogate >::create()
{
}
}

/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::astrocyte_surrogate::Parameters_::Parameters_()
  : sic_( 1.0 )
{
}

nest::astrocyte_surrogate::State_::State_()
{
}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void
nest::astrocyte_surrogate::Parameters_::get( DictionaryDatum& d ) const
{
  def< double >( d, names::SIC, sic_ );
}

void
nest::astrocyte_surrogate::Parameters_::set( const DictionaryDatum& d, Node* node )
{
  updateValueParam< double >( d, names::SIC, sic_, node );

  if ( sic_ < 0.0 )
  {
    throw BadProperty( "SIC value must be >= 0" );
  }
}

void
nest::astrocyte_surrogate::State_::get( DictionaryDatum& d ) const
{
}

void
nest::astrocyte_surrogate::State_::set( const DictionaryDatum& d, const Parameters_&, Node* node )
{
}

nest::astrocyte_surrogate::Buffers_::Buffers_( astrocyte_surrogate& n )
  : logger_( n )
{
}

nest::astrocyte_surrogate::Buffers_::Buffers_( const Buffers_&, astrocyte_surrogate& n )
  : logger_( n )
{
}

/* ----------------------------------------------------------------
 * Default and copy constructor for node, and destructor
 * ---------------------------------------------------------------- */

nest::astrocyte_surrogate::astrocyte_surrogate()
  : ArchivingNode()
  , P_()
  , S_()
  , B_( *this )
{
  recordablesMap_.create();
}

nest::astrocyte_surrogate::astrocyte_surrogate( const astrocyte_surrogate& n )
  : ArchivingNode( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void
nest::astrocyte_surrogate::init_buffers_()
{
  B_.sic_values.resize( kernel().connection_manager.get_min_delay(), 0.0 );

  ArchivingNode::clear_history();

  B_.logger_.reset();
}

void
nest::astrocyte_surrogate::pre_run_hook()
{
  // ensures initialization in case mm connected after Simulate
  B_.logger_.init();
}

/* ----------------------------------------------------------------
 * Update and spike handling functions
 * ---------------------------------------------------------------- */

inline void
nest::astrocyte_surrogate::update( Time const& origin, const long from, const long to )
{
  for ( long lag = from; lag < to; ++lag )
  {
    B_.lag_ = lag;

    // log state data
    B_.logger_.record_data( origin.get_steps() + lag );

    B_.sic_values[ lag ] = P_.sic_;

  } // end for loop

  // Send SIC event
  SICEvent sic;
  sic.set_coeffarray( B_.sic_values );
  kernel().event_delivery_manager.send_secondary( *this, sic );
}

void
nest::astrocyte_surrogate::handle( SpikeEvent& e )
{
}

void
nest::astrocyte_surrogate::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e );
}

#endif // HAVE_GSL
