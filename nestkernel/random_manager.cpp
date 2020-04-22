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
#include <random>
#include <utility>

// Includes from nestkernel:
#include "kernel_manager.h"
#include "random.h"
#include "vp_manager_impl.h"


const std::string nest::RandomManager::DEFAULT_RNG_TYPE_ = "mt19937_64";

const std::uint32_t nest::RandomManager::DEFAULT_BASE_SEED_ = 143202461;

const std::uint32_t nest::RandomManager::RANK_SYNCED_SEEDER_ = 0xc229212d;
const std::uint32_t nest::RandomManager::THREAD_SYNCED_SEEDER_ = 0x37722d5e;
const std::uint32_t nest::RandomManager::THREAD_SPECIFIC_SEEDER_ = 0xb84c9bae;


nest::RandomManager::RandomManager()
  : current_rng_type_( DEFAULT_RNG_TYPE_ )
  , base_seed_( DEFAULT_BASE_SEED_ )
{
}

nest::RandomManager::~RandomManager()
{
  finalize();
}

void
nest::RandomManager::initialize()
{
  register_rng_type< std::mt19937_64 >( "mt19937_64" );
  register_rng_type< std::ranlux48 >( "ranlux48" );  // TODO: remove, testing only

  current_rng_type_ = DEFAULT_RNG_TYPE_;
  base_seed_ = DEFAULT_BASE_SEED_;

  reset_rngs_();
}

void
nest::RandomManager::finalize()
{
  for ( auto& it: rng_types_ )
  {
	delete it.second;
  }

  rng_types_.clear();
  thread_specific_rngs_.clear();
}

void
nest::RandomManager::get_status( DictionaryDatum& d )
{
  ArrayDatum rng_types;
  for ( auto rng = rng_types_.begin(); rng != rng_types_.end(); ++rng )
  {
	rng_types.push_back( rng->first );
  }

  def< ArrayDatum >( d, names::rng_types, rng_types );
  def< long >( d, names::rng_seed, base_seed_ );
  def< std::string >( d, names::rng_type, current_rng_type_ );
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
    if ( not ( 0 < rng_seed and rng_seed < ( 1L << 32 ) ) )
    {
      throw BadProperty( "RNG seed must be in (0, 2^32-1)." );
    }

    base_seed_ = static_cast< std::uint32_t >( rng_seed );
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
    reset_rngs_();
  }
}

void
nest::RandomManager::check_rng_synchrony() const
{
  if ( kernel().mpi_manager.get_num_processes() > 1 )
  {
    if ( not kernel().mpi_manager.grng_synchrony( get_rank_synced_rng()->ulrand( 100000 ) ) )
    {
	  throw KernelException( "Rank-synchronized random number generators are out of sync.");
    }
  }
}

template < typename RNG_TYPE >
void
nest::RandomManager::register_rng_type( std::string name )
{
  rng_types_.insert( std::make_pair( name, new RNGFactory< RNG_TYPE >() ) );
}

void
nest::RandomManager::reset_rngs_()
{
  rank_synced_rng_ = rng_types_[ current_rng_type_ ]->clone( { base_seed_, RANK_SYNCED_SEEDER_ } );

  thread_specific_rngs_.resize( kernel().vp_manager.get_num_threads() );

#pragma omp parallel
  {
    const auto tid = kernel().vp_manager.get_thread_id();
    const std::uint32_t vp = kernel().vp_manager.get_vp();
    thread_synced_rngs_[ tid ] = rng_types_[ current_rng_type_ ]->clone( { base_seed_, THREAD_SYNCED_SEEDER_ } );
    thread_specific_rngs_[ tid ] = rng_types_[ current_rng_type_ ]->clone( { base_seed_, THREAD_SPECIFIC_SEEDER_, vp } );
  }
}

