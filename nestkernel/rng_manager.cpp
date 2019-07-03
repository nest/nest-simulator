/*
 *  rng_manager.cpp
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

#include "rng_manager.h"

// C++ includes:
#include <set>

// Includes from libnestutil:
#include "logging.h"

// Includes from librandom:
#include "gslrandomgen.h"
#include "random_datums.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "vp_manager_impl.h"

// Includes from sli:
#include "arraydatum.h"


nest::RNGManager::RNGManager()
  : rng_()
{
}

void
nest::RNGManager::initialize()
{
  create_rngs_();
  create_grng_();
}

void
nest::RNGManager::finalize()
{
}

void
nest::RNGManager::set_status( const DictionaryDatum& d )
{
  // Any changes in number of threads will be handled by
  // VPManager::set_status(),
  // which will force re-initialization of RNGManager if necessary. This method
  // will only be called *after* such a reset.

  // set RNGs --- MUST come after n_threads_ is updated
  if ( d->known( "rngs" ) )
  {
    // this array contains pre-seeded RNGs, so they can be used
    // directly, no seeding required
    ArrayDatum* ad = dynamic_cast< ArrayDatum* >( ( *d )[ "rngs" ].datum() );
    if ( ad == 0 )
    {
      throw BadProperty();
    }

    // n_threads_ is the new value after a change of the number of
    // threads
    if ( ad->size() != ( size_t )( kernel().vp_manager.get_num_virtual_processes() ) )
    {
      LOG( M_ERROR,
        "RNGManager::set_status",
        "Number of RNGs must equal number of virtual processes "
        "(threads*processes). RNGs "
        "unchanged." );
      throw DimensionMismatch( ( size_t )( kernel().vp_manager.get_num_virtual_processes() ), ad->size() );
    }

    // delete old generators, insert new generators this code is
    // robust under change of thread number in this call to
    // set_status, as long as it comes AFTER n_threads_ has been
    // upated
    rng_.clear();
    for ( index i = 0; i < ad->size(); ++i )
    {
      if ( kernel().vp_manager.is_local_vp( i ) )
      {
        rng_.push_back( getValue< librandom::RngDatum >( ( *ad )[ kernel().vp_manager.suggest_vp_for_gid( i ) ] ) );
      }
    }
  }

  if ( d->known( "rng_seeds" ) )
  {
    ArrayDatum* ad = dynamic_cast< ArrayDatum* >( ( *d )[ "rng_seeds" ].datum() );
    if ( ad == 0 )
    {
      throw BadProperty();
    }

    if ( ad->size() != ( size_t )( kernel().vp_manager.get_num_virtual_processes() ) )
    {
      LOG( M_ERROR,
        "RNGManager::set_status",
        "Number of seeds must equal number of virtual processes "
        "(threads*processes). RNGs unchanged." );
      throw DimensionMismatch( ( size_t )( kernel().vp_manager.get_num_virtual_processes() ), ad->size() );
    }

    // check if seeds are unique
    std::set< unsigned long > seedset;
    for ( index i = 0; i < ad->size(); ++i )
    {
      long s = ( *ad )[ i ]; // SLI has no ulong tokens
      if ( not seedset.insert( s ).second )
      {
        LOG( M_WARNING, "RNGManager::set_status", "Seeds are not unique across threads!" );
        break;
      }
    }

    // now apply seeds, resets generators automatically
    for ( index i = 0; i < ad->size(); ++i )
    {
      long s = ( *ad )[ i ];

      if ( kernel().vp_manager.is_local_vp( i ) )
      {
        rng_[ kernel().vp_manager.vp_to_thread( kernel().vp_manager.suggest_vp_for_gid( i ) ) ]->seed( s );
      }

      rng_seeds_[ i ] = s;
    }
  } // if rng_seeds

  // set GRNG
  if ( d->known( "grng" ) )
  {
    // pre-seeded grng that can be used directly, no seeding required
    updateValue< librandom::RngDatum >( d, names::grng, grng_ );
  }

  if ( d->known( "grng_seed" ) )
  {
    const long gseed = getValue< long >( d, names::grng_seed );

    // check if grng seed is unique with respect to rng seeds
    // if grng_seed and rng_seeds given in one SetStatus call
    std::set< unsigned long > seedset;
    seedset.insert( gseed );
    if ( d->known( "rng_seeds" ) )
    {
      ArrayDatum* ad_rngseeds = dynamic_cast< ArrayDatum* >( ( *d )[ "rng_seeds" ].datum() );
      if ( ad_rngseeds == 0 )
      {
        throw BadProperty();
      }
      for ( index i = 0; i < ad_rngseeds->size(); ++i )
      {
        const long vpseed = ( *ad_rngseeds )[ i ]; // SLI has no ulong tokens
        if ( not seedset.insert( vpseed ).second )
        {
          LOG( M_WARNING, "RNGManager::set_status", "Seeds are not unique across threads!" );
          break;
        }
      }
    }
    // now apply seed, resets generator automatically
    grng_seed_ = gseed;
    grng_->seed( gseed );

  } // if grng_seed
}

void
nest::RNGManager::get_status( DictionaryDatum& d )
{
  ( *d )[ names::rng_seeds ] = Token( rng_seeds_ );
  def< long >( d, names::grng_seed, grng_seed_ );
}


void
nest::RNGManager::create_rngs_()
{
  // if old generators exist, remove them; since rng_ contains
  // shared_ptrs, we don't have to worry about deletion
  if ( not rng_.empty() )
  {
    LOG( M_INFO, "Network::create_rngs_", "Deleting existing random number generators" );

    rng_.clear();
  }

  LOG( M_INFO, "Network::create_rngs_", "Creating default RNGs" );

  rng_seeds_.resize( kernel().vp_manager.get_num_virtual_processes() );

  for ( index i = 0; i < static_cast< index >( kernel().vp_manager.get_num_virtual_processes() ); ++i )
  {
    unsigned long s = i + 1;
    if ( kernel().vp_manager.is_local_vp( i ) )
    {
/*
 We have to ensure that each thread is provided with a different
 stream of random numbers.  The seeding method for Knuth's LFG
 generator guarantees that different seeds yield non-overlapping
 random number sequences.

 We therefore have to seed with known numbers: using random
 seeds here would run the risk of using the same seed twice.
 For simplicity, we use 1 .. n_vps.
 */
#ifdef HAVE_GSL
      librandom::RngPtr rng( new librandom::GslRandomGen( gsl_rng_knuthran2002, s ) );
#else
      librandom::RngPtr rng = librandom::RandomGen::create_knuthlfg_rng( s );
#endif

      if ( not rng )
      {
        LOG( M_ERROR, "Network::create_rngs_", "Error initializing knuthlfg" );

        throw KernelException();
      }

      rng_.push_back( rng );
    }

    rng_seeds_[ i ] = s;
  }
}

void
nest::RNGManager::create_grng_()
{
  // create new grng
  LOG( M_INFO, "Network::create_grng_", "Creating new default global RNG" );

// create default RNG with default seed
#ifdef HAVE_GSL
  grng_ = librandom::RngPtr( new librandom::GslRandomGen( gsl_rng_knuthran2002, librandom::RandomGen::DefaultSeed ) );
#else
  grng_ = librandom::RandomGen::create_knuthlfg_rng( librandom::RandomGen::DefaultSeed );
#endif

  if ( not grng_ )
  {
    LOG( M_ERROR, "Network::create_grng_", "Error initializing knuthlfg" );

    throw KernelException();
  }

  /*
   The seed for the global rng should be different from the seeds
   of the local rngs_ for each thread seeded with 1,..., n_vps.
   */
  long s = 0;
  grng_seed_ = s;
  grng_->seed( s );
}
