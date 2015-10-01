/*
 *  random.cpp
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

#include "random.h"

#include <string>
#include <cassert>

#include "sliexceptions.h"
#include "tokenarray.h"


librandom::RngDatum
librandom::CreateRNG( const long seed, const RngFactoryDatum& factory )
{
  return librandom::RngDatum( factory->create( seed ) );
}

librandom::RdvDatum
librandom::CreateRDV( const RdvFactoryDatum& factory, const RngDatum& rng )
{
  return librandom::RdvDatum( factory->create( rng ) );
}

void
librandom::SetStatus( const DictionaryDatum& dict, RdvDatum& rdv )
{
  dict->clear_access_flags();
  rdv->set_status( dict );
  std::string missed;
  if ( !dict->all_accessed( missed ) )
    throw UnaccessedDictionaryEntry( missed );
}

DictionaryDatum
librandom::GetStatus( const RdvDatum& rdv )
{
  DictionaryDatum dict( new Dictionary );
  assert( dict.valid() );

  rdv->get_status( dict );

  return dict;
}

void
librandom::seed( const long seed, RngDatum& rng )
{
  rng->seed( seed );
}

unsigned long
librandom::irand( const long N, RngDatum& rng )
{
  return rng->ulrand( N );
}

double
librandom::drand( RngDatum& rng )
{
  return rng->drand();
}

ArrayDatum
librandom::RandomArray( RdvDatum& rdv, const size_t n )
{
  TokenArray result;
  result.reserve( n );

  if ( rdv->has_ldev() )
  {
    for ( size_t j = 0; j < n; ++j )
    {
      result.push_back( rdv->ldev() );
    }
  }
  else
  {
    for ( size_t j = 0; j < n; ++j )
    {
      result.push_back( ( *rdv )() );
    }
  }

  return ArrayDatum( result );
}

long
librandom::Random( RdvDatum& rdv )
{
  if ( rdv->has_ldev() )
  {
    return rdv->ldev();
  }
  else
  {
    return ( *rdv )();
  }
}
