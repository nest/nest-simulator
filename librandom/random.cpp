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

// C++ includes:
#include <cassert>
#include <string>

// Includes from sli:
#include "sliexceptions.h"
#include "tokenarray.h"


librandom::RngDatum
librandom::create_rng( const long seed, const RngFactoryDatum& factory )
{
  librandom::RngPtr rng_ptr = factory->create( seed );
  return librandom::RngDatum( rng_ptr );
}

librandom::RdvDatum
librandom::create_rdv( const RdvFactoryDatum& factory, const RngDatum& rng )
{
  return librandom::RdvDatum( factory->create( rng ) );
}

void
librandom::set_status( const DictionaryDatum& dict, RdvDatum& rdv )
{
  dict->clear_access_flags();
  rdv->set_status( dict );
  std::string missed;
  if ( not dict->all_accessed( missed ) )
  {
    throw UnaccessedDictionaryEntry( missed );
  }
}

DictionaryDatum
librandom::get_status( const RdvDatum& rdv )
{
  DictionaryDatum dict( new Dictionary );

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
librandom::random_array( RdvDatum& rdv, const size_t n )
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

Token
librandom::random( RdvDatum& rdv )
{
  if ( rdv->has_ldev() )
  {
    // returns long
    return Token( rdv->ldev() );
  }
  else
  {
    // returns double
    return Token( ( *rdv )() );
  }
}
