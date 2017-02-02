/*
 *  rng_manager.h
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

#ifndef RNG_MANAGER_H
#define RNG_MANAGER_H

// C++ includes:
#include <vector>

// Includes from libnestutil:
#include "manager_interface.h"

// Includes from librandom:
#include "randomgen.h"

// Includes from nestkernel:
#include "mpi_manager.h"
#include "nest_types.h"

// Includes from sli:
#include "dictdatum.h"

namespace nest
{

class RNGManager : public ManagerInterface
{
public:
  RNGManager();
  virtual ~RNGManager()
  {
  }

  virtual void initialize();
  virtual void finalize();

  virtual void set_status( const DictionaryDatum& );
  virtual void get_status( DictionaryDatum& );

  /**
   * Get random number client of a thread.
   * Defaults to thread 0 to allow use in non-threaded
   * context.  One may consider to introduce an additional
   * RNG just for the non-threaded context.
   */
  librandom::RngPtr get_rng( thread thrd = 0 ) const;

  /**
   * Get global random number client.
   * This grng must be used synchronized from all threads.
   */
  librandom::RngPtr get_grng() const;

private:
  void create_rngs_();
  void create_grng_();

  /**
   * Vector of random number generators for threads.
   * There must be PRECISELY one rng per thread.
   */
  std::vector< librandom::RngPtr > rng_;

  /**
   * Global random number generator.
   * This rng must be synchronized on all threads
   */
  librandom::RngPtr grng_;

  //! The seeds of the local RNGs. These do not necessarily describe the
  //! state of the RNGs.
  std::vector< long > rng_seeds_;

  //! The seed of the global RNG, not necessarily describing the
  //! state of the GRNG.
  long grng_seed_;

}; // class RNGManager
} // namespace nest

inline librandom::RngPtr
nest::RNGManager::get_rng( nest::thread t ) const
{
  assert( t < static_cast< nest::thread >( rng_.size() ) );
  return rng_[ t ];
}

inline librandom::RngPtr
nest::RNGManager::get_grng() const
{
  return grng_;
}

#endif /* RNG_MANAGER_H */
