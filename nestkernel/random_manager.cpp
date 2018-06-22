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

// Includes from librandom:
#include "clipped_dist.h"
#include "rng.h"
#include "rdist.h"

// Includes from nestkernel:
#include "kernel_manager.h"

void
nest::RandomManager::initialize()
{
  register_rng_type< random::MT19937 >( "mt19937" );
//  register_rng_type< random::Philox >( "philox" );
//  register_rng_type< random::Threefry >( "threefry" );
  rng_type_ = Name("threefry");

  register_rdist_type< random::binomial >( "binomial", true );
//  register_rdist_type< random::exponential >( "exponential", true );
//  register_rdist_type< random::gamma >( "gamma", true );
//  register_rdist_type< random::lognormal >( "lognormal", true );
//  register_rdist_type< random::normal >( "normal", true );
//  register_rdist_type< random::poisson >( "poisson", true );
//  register_rdist_type< random::uniform_int >( "uniform_int", false );
//  register_rdist_type< random::uniform_real >( "uniform_real", false );

  create_rngs_();
}

void
nest::RandomManager::finalize()
{
  delete_rngs();
    
  for (auto rng = rng_types_.begin(); rng != rng_types_.end(); ++rng )
  {
    delete *rng;
  }
}

void
nest::RandomManager::get_status( DictionaryDatum& d )
{
  def< long >( d, names::rng_seed, rng_seed_ );
  def< long >( d, names::rng_type, rng_type_ );

  ArrayDatum rng_types;
  for (auto rng = rng_types_.begin(); rng != rng_types_.end(); ++rng )
  {
    rng_types.push_back( rng.first );
  }
  def< ArrayDatum >( d, names::rng_types, rng_types );
}

void
nest::RandomManager::set_status( const DictionaryDatum& d )
{
  long n_threads;
  bool n_threads_updated =
    updateValue< long >( d, names::local_num_threads, n_threads );

  long rng_seed;
  bool rng_seed_updated = updateValue< long >( d, names::rng_seed, rng_seed );
  if ( rng_seed_updated )
  {
    if ( rng_seed < 0 or rng_seed >= 2**32 )
    {
      throw BadProperty( "RNG seed must be in [0, 2^32)." );
    }

    rng_seed_ = rng_seed;
  }

  long rng_type;
  bool rng_type_updated = updateValue< long >( d, names::rng_type, rng_type );
  if ( rng_type_updated )
  {
    auto rng = rng_types_.find( rng_type );
    if ( rng == rng_types_.end() )
    {
      throw BadProperty("/rng_type is not a known RNG type. "
			"See property /rng_types for available types" );
    }

    rng_type_ = rng_type;
  }

  if ( n_threads_updated or rng_seed_updated or rng_type_updated )
  {
    delete_rngs_();
    create_rngs_();
  }
}

void
nest::RandomManager::create_rngs_()
{
  long seed = rng_seed_ + kernel().vp_manager.get_num_virtual_processes();
  grng_ = rng_types_[ rng_type_ ].clone( seed );

  long num_threads = kernel().vp_manager.get_num_threads();
  trngs_.resize( num_threads );

#omp parallel
  {
    index tid = kernel().vp_manager.get_thread_id();
    seed = rng_seed_ + kernel().vp_manager.thread_to_vp( tid );
    trngs_[ tid ] = rng_types_[ rng_type_ ].clone( seed );
  }
}

void
nest::RandomManager::delete_rngs_()
{
  delete grng_;

#omp parallel
  {
    index tid = kernel().vp_manager.get_thread_id();
    delete trngs_[ tid ];
  }  
}

void
nest::RandomManager::create_rdist_( index rdist_id )
{
  // need some kind of lockpointer object to return the clone in
}

template< class RNG >
void
nest::RandomManager::register_rng_type_( Name name )
{
  rngdict_->insert( name, rng_types_.size() );
  rng_types_.push_back( new RNG {} );
}

template< class RDist >
void
nest::RandomManager::register_rdist_type_( Name name, bool register_clipped )
{
  rdistdict_.insert( name, rdict_types_.size() );
  rdist_types_.push_back( new RDist {} );

  if ( register_clipped )
  {
    rdistdict_.insert( name.toString() + "_clipped", rdict_types_.size() );
    rdist_types_.push_back( random::ClippedRedrawDist< RDist > {} );

    rdistdict_.insert( name.toString() + "_clipped_to_boundary", rdict_types_.size() );
    rdist_types_.push_back( random::ClippedToBoundaryDist< RDist > {} );
  }
}
