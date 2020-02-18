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

// TODO: Maybe caching numbers can help

namespace nest
{

class RandomManager : public ManagerInterface
{
public:
  RandomManager()
  {
  }
  ~RandomManager()
  {
  }

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

private:
  long rng_seed_;

  /** Vector of random number generators for threads. */
  std::vector< RngPtr > thread_rngs_;

  /** Global random number generator. */
  RngPtr global_rng_;

  /** */
  std::string current_rng_type_;

  /** */
  std::map< std::string, RngPtr > rng_types_;

  /**
   *
   **/
  template < typename RNG_TYPE_ >
  void register_rng_type_( std::string name, bool set_as_default );

  /**
   *
   **/
  void create_rngs_();
};

inline RngPtr
nest::RandomManager::get_thread_rng( thread tid ) const
{
  assert( tid >= 0 );
  assert( tid < static_cast< thread >( thread_rngs_.size() ) );
  return thread_rngs_[ tid ];
}

inline RngPtr
nest::RandomManager::get_global_rng() const
{
  return global_rng_;
}

} // namespace nest

#endif /* RANDOM_MANAGER_H */
