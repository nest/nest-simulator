/*
 *  gslrandomgen.cpp
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

#include "gslrandomgen.h"

#ifdef HAVE_GSL

// nothing if GSL 1.2 or later not available

librandom::GslRandomGen::GslRandomGen( const gsl_rng_type* type, unsigned long seed )
  : RandomGen()
{
  rng_ = gsl_rng_alloc( type );
  rng_type_ = type;
  assert( rng_ != NULL );
  gsl_rng_set( rng_, seed );
}

librandom::GslRandomGen::~GslRandomGen()
{
  gsl_rng_free( rng_ );
}

// function initializing RngList
// add further self-implemented RNG below
void
librandom::GslRandomGen::add_gsl_rngs( Dictionary& rngdict )
{
  // add all standard GSL RNG, or those from GSL replacement
  const gsl_rng_type** t0 = gsl_rng_types_setup();
  for ( const gsl_rng_type** t = t0; *t != NULL; ++t )
  {
    assert( *t != NULL );
    const std::string name = std::string( "gsl_" ) + ( *t )->name;

    if ( not rngdict.known( name ) ) // avoid multiple insertion
    {
      GslRNGFactory* f = new GslRNGFactory( *t );
      assert( f != NULL );

      Token rngfactory = new librandom::RngFactoryDatum( f );
      rngdict.insert_move( Name( name ), rngfactory );
    }
  }
}

librandom::GslRNGFactory::GslRNGFactory( gsl_rng_type const* const t )
  : gsl_rng_( t )
{
  assert( t != 0 );
}

librandom::RngPtr
librandom::GslRNGFactory::create( unsigned long s ) const
{
  return RngPtr( new GslRandomGen( gsl_rng_, s ) );
}

#endif
