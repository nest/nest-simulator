/*
 *  random_manager.cpp
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

#include "random_manager.h"

// C++ includes:
#include <cmath>
#include <random>
#include <utility>

// Includes from nestkernel:
#include "kernel_manager.h"
#include "vp_manager_impl.h"
#include "random.h"

void
nest::RandomManager::initialize()
{
  register_rng_type_< std::mt19937_64 >( "mt19937_64", true );
  //  register_rng_type_< Philox >( "philox", false );
  //  register_rng_type_< Threefry >( "threefry", false );

  create_rngs_();
}

void
nest::RandomManager::finalize()
{
}

void
nest::RandomManager::get_status( DictionaryDatum& d )
{
  def< long >( d, names::rng_seed, rng_seed_ );

  def< std::string >( d, names::rng_type, current_rng_type_ );

  ArrayDatum rng_types;
  for ( auto rng = rng_types_.begin(); rng != rng_types_.end(); ++rng )
  {
    rng_types.push_back( rng->first );
  }
  def< ArrayDatum >( d, names::rng_types, rng_types );
}

void
nest::RandomManager::set_status( const DictionaryDatum& d )
{
  long n_threads;
  bool n_threads_updated = updateValue< long >( d, names::local_num_threads, n_threads );

  long rng_seed;
  bool rng_seed_updated = updateValue< long >( d, names::rng_seed, rng_seed );

  if ( rng_seed_updated )
  {
    if ( rng_seed < 0 or rng_seed >= std::pow( 2, 32 ) )
    {
      throw BadProperty( "RNG seed must be in [0, 2^32)." );
    }

    rng_seed_ = rng_seed;
  }

  std::string rng_type;
  bool rng_type_updated = updateValue< std::string >( d, names::rng_type, rng_type );

  if ( rng_type_updated )
  {
    auto rng = rng_types_.find( rng_type );
    if ( rng == rng_types_.end() )
    {
      std::string msg = "'%1' is not a known RNG type. See /rng_types for available types";
      throw BadProperty( String::compose( msg, rng_type ) );
    }

    current_rng_type_ = rng_type;
  }

  if ( n_threads_updated or rng_seed_updated or rng_type_updated )
  {
    create_rngs_();
  }
}

void
nest::RandomManager::create_rngs_()
{
  // Seed for the global rng must be invariant with different numbers of VPs.
  global_rng_ = rng_types_[ current_rng_type_ ]->clone( rng_seed_ );

  long num_threads = kernel().vp_manager.get_num_threads();
  thread_rngs_.resize( num_threads );

#pragma omp parallel
  {
    // TODO: draw seeds from a well-defined RNG. For that use a 64-bit seed
    // with the 32 low bits corresponding to the number of vps and the high
    // ones being the ones specified by the user

    const auto tid = kernel().vp_manager.get_thread_id();
    const long local_seed = rng_seed_ + 1 + kernel().vp_manager.get_vp();
    thread_rngs_[ tid ] = rng_types_[ current_rng_type_ ]->clone( local_seed );
  }
}

template < typename RNG_TYPE_ >
void
nest::RandomManager::register_rng_type_( std::string name, bool set_as_default )
{
  rng_types_.insert( std::make_pair( name, new RNG< RNG_TYPE_ >( RNG_TYPE_( 0 ) ) ) );

  if ( set_as_default )
  {
    current_rng_type_ = name;
  }
}
