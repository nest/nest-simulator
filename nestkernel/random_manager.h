/*
 *  random_manager.h
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

#ifndef RANDOM_MANAGER_H
#define RANDOM_MANAGER_H

// Generated includes:
#include "config.h"

// C++ includes:
#include <string>
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from nestkernel:
#include "nest_types.h"
#include "random.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

/**
 * Manage the kernel's random number generators.
 *
 * This manager provides one random number generator per thread plus
 * the global RNG. It also handles selection of RNG type and seeding.
 */
class RandomManager : public ManagerInterface
{
public:
  RandomManager();
  ~RandomManager();

  /**
   * Register available RNG types, set default RNG type and create RNGs.
   */
  virtual void initialize();

  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  /**
   * Get thread-specific random number generator.
   *
   * Each thread can use this RNG freely and will receive an independent
   * random number sequence.
   */
  RngPtr get_thread_specific_rng( thread tid ) const;

  /**
   * Get rank-synchronized random number generator.
   *
   * The rank-synchronized generator provides identical random number
   * sequences on all MPI ranks. It may be used only by the master thread
   * on each rank and must be used in lock-step across all ranks. Synchronization
   * is checked by MPI exchange at certain points during a simulation.
   **/
  RngPtr get_rank_synced_rng() const;

  /**
   * Get thread-synchronized random number client.
   *
   * The kernel maintains one instance of a synchronized random number generator
   * for each thread (and thus, across ranks, for each VP). The purpose of these
   * synchronized generators is to provide identical random number sequences on
   * each thread while threads execute in parallel. All threads must use the
   * generators in lock-step to maintain synchrony. Synchronization
   * is checked by MPI exchange at certain points during a simulation.
   *
   * @param tid Number of thread requesting generator
   **/
  RngPtr get_thread_synced_rng( thread tid ) const;

  /**
   * Confirm that rank- and thread-synchronized RNGs are in sync.
   *
   * @throws KernelException if RNGs are out of sync
   */
  void check_rng_synchrony() const;

  /**
   * Register new random number generator type with manager.
   *
   * This allows NEST modules to add new RNG types.
   *
   * @param RNG_TYPE Class fulfilling requirements of C++ RNG.
   **/
  template < typename RNG_TYPE >
  void register_rng_type( std::string name );

private:
  /** Available RNG types. */
  std::map< std::string, BaseRNGFactory* > rng_types_;

  /** Name of currently used RNG type. */
  std::string current_rng_type_;

  /** Base seed used when RNGs were last created. */
  std::uint32_t base_seed_;

  /** Random number generator synchronized across ranks. */
  RngPtr rank_synced_rng_;

  /** Random number generators synchronized across threads. */
  std::vector< RngPtr > thread_synced_rngs_;

  /** Random number generators specific to threads. */
  std::vector< RngPtr > thread_specific_rngs_;

  /**
   * Replace current RNGs with newly seeded generators.
   *
   * The new generators will be of type `current_rng_type_` and will be
   * seeded using `base_seed_`.
   **/
  void reset_rngs_();

  /** RNG type used by default. */
  static const std::string DEFAULT_RNG_TYPE_;

  /** Base seed used by default. */
  static const std::uint32_t DEFAULT_BASE_SEED_;

  /** Rank-synchronized seed-sequence initializer component. */
  static const std::uint32_t RANK_SYNCED_SEEDER_;

  /** Thread-synchronized seed-sequence initializer component. */
  static const std::uint32_t THREAD_SYNCED_SEEDER_;

  /** Thread-specific seed-sequence initializer component. */
  static const std::uint32_t THREAD_SPECIFIC_SEEDER_;
};

inline RngPtr
nest::RandomManager::get_rank_synced_rng() const
{
  assert( kernel().vp_manager.get_thread_id() == 0 );  // rank-synced RNG to be used by master thread only
  return rank_synced_rng_;
}

inline RngPtr
nest::RandomManager::get_thread_synced_rng( thread tid ) const
{
  assert( tid >= 0 );
  assert( tid < static_cast< thread >( thread_specific_rngs_.size() ) );
  return thread_synced_rngs_[ tid ];
}

inline RngPtr
nest::RandomManager::get_thread_specific_rng( thread tid ) const
{
  assert( tid >= 0 );
  assert( tid < static_cast< thread >( thread_specific_rngs_.size() ) );
  return thread_specific_rngs_[ tid ];
}

} // namespace nest

#endif /* RANDOM_MANAGER_H */
