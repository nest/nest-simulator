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
#include "arraydatum.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "random_numbers.h"
#include "random_datums.h"
#include "nest_names.h"
#include "tokenutils.h"

nest::ConnParameter*
nest::ConnParameter::create( const Token& t )
{
  // Code grabbed from TopologyModule::create_parameter()
  // See there for a more general solution

  // single double
  DoubleDatum* dd = dynamic_cast< DoubleDatum* >( t.datum() );
  if ( dd )
    return new ScalarDoubleParameter( *dd );

  // random deviate
  DictionaryDatum* rdv_spec = dynamic_cast< DictionaryDatum* >( t.datum() );
  if ( rdv_spec )
    return new RandomParameter( *rdv_spec );

  // single integer
  IntegerDatum* id = dynamic_cast< IntegerDatum* >( t.datum() );
  if ( id )
    return new ScalarIntegerParameter( *id );

  // array
  DoubleVectorDatum* dvd = dynamic_cast< DoubleVectorDatum* >( t.datum() );
  if ( dvd )
    throw NotImplemented( "Cannot handle parameter type." );
  // return new ArrayParameter(**dvd);

  throw BadProperty( "Cannot handle parameter type." );
}

nest::RandomParameter::RandomParameter( const DictionaryDatum& rdv_spec )
  : rdv_( 0 )
{
  if ( !rdv_spec->known( names::distribution ) )
    throw BadProperty( "Random distribution spec must contain distribution name." );

  const std::string rdv_name = ( *rdv_spec )[ names::distribution ];
  if ( !RandomNumbers::get_rdvdict().known( rdv_name ) )
    throw BadProperty( "Unknown random deviate: " + rdv_name );

  librandom::RdvFactoryDatum factory =
    getValue< librandom::RdvFactoryDatum >( RandomNumbers::get_rdvdict()[ rdv_name ] );

  rdv_ = factory->create();
  rdv_->set_status( rdv_spec );
}
