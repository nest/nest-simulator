/*
 *  conn_parameter.cpp
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

#include "conn_parameter.h"

// Includes from librandom:
#include "random_datums.h"
#include "random_numbers.h"

// Includes from nestkernel:
#include "nest_names.h"
#include "kernel_manager.h"

// Includes from sli:
#include "arraydatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "tokenutils.h"

nest::ConnParameter*
nest::ConnParameter::create( const Token& t, const size_t nthreads )
{
  // single double
  DoubleDatum* dd = dynamic_cast< DoubleDatum* >( t.datum() );
  if ( dd )
  {
    return new ScalarDoubleParameter( *dd, nthreads );
  }

  // random deviate
  DictionaryDatum* rdv_spec = dynamic_cast< DictionaryDatum* >( t.datum() );
  if ( rdv_spec )
  {
    return new RandomParameter( *rdv_spec, nthreads );
  }

  // single integer
  IntegerDatum* id = dynamic_cast< IntegerDatum* >( t.datum() );
  if ( id )
  {
    return new ScalarIntegerParameter( *id, nthreads );
  }

  // array of doubles
  DoubleVectorDatum* dvd = dynamic_cast< DoubleVectorDatum* >( t.datum() );
  if ( dvd )
  {
    return new ArrayDoubleParameter( **dvd, nthreads );
  }

  // Parameter
  ParameterDatum* pd = dynamic_cast< ParameterDatum* >( t.datum() );
  if ( pd )
  {
    return new ParameterConnParameterWrapper( *pd, nthreads );
  }

  // array of integer
  IntVectorDatum* ivd = dynamic_cast< IntVectorDatum* >( t.datum() );
  if ( ivd )
  {
    return new ArrayIntegerParameter( **ivd, nthreads );
  }

  throw BadProperty( std::string( "Cannot handle parameter type. Received " ) + t.datum()->gettypename().toString() );
}

nest::RandomParameter::RandomParameter( const DictionaryDatum& rdv_spec, const size_t )
  : rdv_( 0 )
{
  if ( not rdv_spec->known( names::distribution ) )
  {
    throw BadProperty( "Random distribution spec must contain distribution name." );
  }

  const std::string rdv_name = ( *rdv_spec )[ names::distribution ];
  if ( not RandomNumbers::get_rdvdict().known( rdv_name ) )
  {
    throw BadProperty( "Unknown random deviate: " + rdv_name );
  }

  librandom::RdvFactoryDatum factory =
    getValue< librandom::RdvFactoryDatum >( RandomNumbers::get_rdvdict()[ rdv_name ] );

  rdv_ = factory->create();
  rdv_->set_status( rdv_spec );

  provides_long_ = rdv_->has_ldev();
}


nest::ParameterConnParameterWrapper::ParameterConnParameterWrapper( const ParameterDatum& pd, const size_t )
  : parameter_( pd.get() )
{
}

double
nest::ParameterConnParameterWrapper::value_double( thread target_thread,
  librandom::RngPtr& rng,
  index snode_id,
  Node* target ) const
{
  return parameter_->value( rng, snode_id, target, target_thread );
}
