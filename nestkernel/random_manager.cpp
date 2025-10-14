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
#include "dictutils.h"
#include "kernel_manager.h"
#include "mpi_manager.h"
#include "nest_names.h"
#include "random_generators.h"
#include "vp_manager.h"

// Includes from libnestutil:
#ifdef HAVE_RANDOM123
#include "Random123/conventional/Engine.hpp"
#include "Random123/philox.h"
#include "Random123/threefry.h"
#endif


const std::string nest::RandomManager::DEFAULT_RNG_TYPE_ = "mt19937_64";

const std::uint32_t nest::RandomManager::DEFAULT_BASE_SEED_ = 143202461;

const std::uint32_t nest::RandomManager::RANK_SYNCED_SEEDER_ = 0xc229212d;
const std::uint32_t nest::RandomManager::THREAD_SYNCED_SEEDER_ = 0x37722d5e;
const std::uint32_t nest::RandomManager::THREAD_SPECIFIC_SEEDER_ = 0xb84c9bae;


nest::RandomManager::RandomManager()
  : current_rng_type_( DEFAULT_RNG_TYPE_ )
  , base_seed_( DEFAULT_BASE_SEED_ )
  , rank_synced_rng_( nullptr )
{
}

nest::RandomManager::~RandomManager()
{
}

void
nest::RandomManager::initialize( const bool adjust_number_of_threads_or_rng_only )
{
  if ( not adjust_number_of_threads_or_rng_only )
  {
    register_rng_type< std::mt19937 >( "mt19937" );
    register_rng_type< std::mt19937_64 >( "mt19937_64" );
#ifdef HAVE_RANDOM123
    register_rng_type< r123::Engine< r123::Philox4x32 > >( "Philox_32" );
#if R123_USE_PHILOX_64BIT
    register_rng_type< r123::Engine< r123::Philox4x64 > >( "Philox_64" );
#endif
    register_rng_type< r123::Engine< r123::Threefry4x32 > >( "Threefry_32" );
#if R123_USE_64BIT
    register_rng_type< r123::Engine< r123::Threefry4x64 > >( "Threefry_64" );
#endif
#endif

    current_rng_type_ = DEFAULT_RNG_TYPE_;
    base_seed_ = DEFAULT_BASE_SEED_;
  }

  // Create new RNGs of the currently used RNG type.
  rank_synced_rng_ = rng_types_[ current_rng_type_ ]->create( { base_seed_, RANK_SYNCED_SEEDER_ } );

  vp_synced_rngs_.resize( kernel::manager< VPManager >.get_num_threads() );
  vp_specific_rngs_.resize( kernel::manager< VPManager >.get_num_threads() );

#pragma omp parallel
  {
    const auto tid = kernel::manager< VPManager >.get_thread_id();
    const std::uint32_t vp = kernel::manager< VPManager >.get_vp(); // type required for rng initializer
    vp_synced_rngs_[ tid ] = rng_types_[ current_rng_type_ ]->create( { base_seed_, THREAD_SYNCED_SEEDER_ } );
    vp_specific_rngs_[ tid ] = rng_types_[ current_rng_type_ ]->create( { base_seed_, THREAD_SPECIFIC_SEEDER_, vp } );
  }
}

void
nest::RandomManager::finalize( const bool adjust_number_of_threads_or_rng_only )
{
  // Delete existing RNGs
  auto delete_rngs = []( std::vector< RngPtr >& rng_vec )
  {
    for ( auto rng : rng_vec )
    {
      delete rng;
    }
  };

  delete rank_synced_rng_;
  delete_rngs( vp_synced_rngs_ );
  delete_rngs( vp_specific_rngs_ );
  vp_synced_rngs_.clear();
  vp_specific_rngs_.clear();

  if ( not adjust_number_of_threads_or_rng_only )
  {
    for ( auto& it : rng_types_ )
    {
      delete it.second;
    }
    rng_types_.clear();
  }
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
  long rng_seed;
  bool rng_seed_updated = updateValue< long >( d, names::rng_seed, rng_seed );

  if ( rng_seed_updated )
  {
    if ( not( 0 < rng_seed and rng_seed < ( 1L << 32 ) ) )
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

  if ( rng_seed_updated or rng_type_updated )
  {
    finalize( /* adjust_number_of_threads_or_rng_only */ true );
    initialize( /* adjust_number_of_threads_or_rng_only */ true );
  }
}

void
nest::RandomManager::check_rng_synchrony() const
{
  // Compare more than a single number to avoid false negatives
  const long NUM_ROUNDS = 5;

  // We check rank-synchrony even if we are on a single process to keep the code simple.
  for ( auto n = 0; n < NUM_ROUNDS; ++n )
  {
    const auto r = rank_synced_rng_->drand();
    if ( not kernel::manager< MPIManager >.equal_cross_ranks( r ) )
    {
      throw KernelException( "Rank-synchronized random number generators are out of sync." );
    }
  }

  // We check thread-synchrony under all circumstances to keep the code simple.
  for ( auto n = 0; n < NUM_ROUNDS; ++n )
  {
    const size_t num_threads = kernel::manager< VPManager >.get_num_threads();
    double local_min = std::numeric_limits< double >::max();
    double local_max = std::numeric_limits< double >::min();
    for ( size_t t = 0; t < num_threads; ++t )
    {
      const auto r = vp_synced_rngs_[ t ]->drand();
      local_min = std::min( r, local_min );
      local_max = std::max( r, local_max );
    }

    // If local values are not equal, flag this in local_min.
    if ( local_min != local_max )
    {
      local_min = -std::numeric_limits< double >::infinity();
    }

    if ( not kernel::manager< MPIManager >.equal_cross_ranks( local_min ) )
    {
      throw KernelException( "Thread-synchronized random number generators are out of sync." );
    }
  }
}

template < typename RNG_TYPE >
void
nest::RandomManager::register_rng_type( const std::string& name )
{
  rng_types_.insert( std::make_pair( name, new RandomGeneratorFactory< RNG_TYPE >() ) );
}


nest::RngPtr
nest::RandomManager::get_rank_synced_rng() const
{
  return rank_synced_rng_;
}

nest::RngPtr
nest::RandomManager::get_vp_synced_rng( size_t tid ) const
{
  assert( tid < static_cast< size_t >( vp_specific_rngs_.size() ) );
  return vp_synced_rngs_[ tid ];
}

nest::RngPtr
nest::RandomManager::get_vp_specific_rng( size_t tid ) const
{
  assert( tid < static_cast< size_t >( vp_specific_rngs_.size() ) );
  return vp_specific_rngs_[ tid ];
}
