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
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from librandom:
#include "librandom.h"

// Includes from nestkernel:
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"
#include "name.h"

namespace nest
{

class RandomManager : public ManagerInterface
{
public:

  RandomManager() {}
  ~RandomManager() {}

  virtual void initialize();
  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  /**
   * Get random number client of a thread.
   */
  random::BaseRNG& get_rng( thread tid ) const;

  /**
   * Get global random number client.
   * This RNG must be used in a synchronized fashion from all threads.
   **/
  random::BaseRNG& get_grng() const;

private:

  /** Vector of random number generators for threads. */
  std::vector< random::BaseRNG* > trngs_;

  /** Global random number generator. */
  random::BaseRNG* grng_;

  /** */
  DictionaryDatum rngdict_;

  /** */
  DictionaryDatum rdistdict_

  Name rng_type_;
  std::vector< random::BaseRNG* > rng_types;
  std::vector< random::BaseRDist* > rdist_types;

  /**
   *
   **/
  template< class RNG >
  void register_rng_type( Name name, bool set_as_default );

  /**
   *
   **/
  template< class RDist >
  void register_rdist_type( Name name, bool register_clipped );

  /**
   *
   **/
  void create_rngs_();

  /**
   *
   **/
  void delete_rngs_();

  /**
   *
   **/
  void create_rdist_();
};

inline random::BaseRNG&
nest::RandomManager::get_rng( thread tid ) const
{
  assert( t < static_cast< nest::thread >( rng_.size() ) );
  return trngs_[ t ];
}

inline random::BaseRNG&
nest::RandomManager::get_grng() const
{
  return grng_;
}

} // namespace nest

#endif /* RANDOM_MANAGER_H */
