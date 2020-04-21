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
   * Get random number client of a thread.
   */
  RngPtr get_thread_rng( thread tid ) const;

  /**
   * Get global random number client.
   * This RNG must be used in a synchronized fashion from all virtual processes.
   **/
  RngPtr get_global_rng() const;

  /**
   * Return random number generator with node-specific seed.
   *
   * @param node_id
   * @return
   */
  RngPtr create_node_specific_rng( index node_id );

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

  /** Global random number generator. */
  RngPtr global_rng_;

  /** Random number generators for threads. */
  std::vector< RngPtr > thread_rngs_;

  /**
   * Replace current RNGs with newly seeded generators.
   *
   * The new generators will be of type `current_rng_type_` and will be
   * seeded using `base_seed_`.
   **/
  void reset_rngs_();

  static const std::string DEFAULT_RNG_TYPE_;
  static const std::uint32_t DEFAULT_BASE_SEED_;
  static const std::uint32_t NODE_RNG_SEED_OFFSET_;
};

inline RngPtr
nest::RandomManager::get_global_rng() const
{
  return global_rng_;
}

inline RngPtr
nest::RandomManager::get_thread_rng( thread tid ) const
{
  assert( tid >= 0 );
  assert( tid < static_cast< thread >( thread_rngs_.size() ) );
  return thread_rngs_[ tid ];
}


} // namespace nest

#endif /* RANDOM_MANAGER_H */
