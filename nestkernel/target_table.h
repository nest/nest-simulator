/*
 *  target_table.h
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

#ifndef TARGET_TABLE_H
#define TARGET_TABLE_H

// C++ includes:
#include <cassert>
#include <iostream>
#include <map>
#include <vector>

// Includes from nestkernel:
#include "nest_types.h"
#include "spike_data.h"
#include "target.h"
#include "target_data.h"

namespace nest
{

/**
 * This data structure stores all targets of the local neurons.
 *
 * This is the presynaptic part of the connection infrastructure.
 */
class TargetTable
{
private:
  /**
   * Stores targets of local neurons
   *
   * Three dimensional objects:
   *   - first dim: threads
   *   - second dim: local neurons
   *   - third dim: targets
   */
  std::vector< std::vector< std::vector< Target > > > targets_;

  /**
   * Stores MPI send buffer positions for secondary targets of local
   * neurons.
   *
   * Four dimensional object:
   *   - first dim: threads
   *   - second dim: local neurons
   *   - third dim: synapse types
   *   - fourth dim: MPI send buffer positions
   */
  std::vector< std::vector< std::vector< std::vector< size_t > > > > secondary_send_buffer_pos_;

public:
  /**
   * Initializes data structures.
   */
  void initialize();

  /**
   * Deletes data structure.
   */
  void finalize();

  /**
   * Adjusts targets_ to number of local nodes.
   */
  void prepare( const size_t tid );

  /**
   * Adds entry to targets_.
   */
  void add_target( const size_t tid, const size_t target_rank, const TargetData& target_data );

  /**
   * Returns all targets of a neuron. Used for filling
   * EventDeliveryManager::emitted_spikes_register_.
   */
  const std::vector< Target >& get_targets( const size_t tid, const size_t lid ) const;

  /**
   * Returns all MPI send buffer positions of a neuron.
   *
   * Used to fill MPI buffer in EventDeliveryManager.
   */
  const std::vector< size_t >&
  get_secondary_send_buffer_positions( const size_t tid, const size_t lid, const synindex syn_id ) const;

  /**
   * Clears all entries of targets_.
   */
  void clear( const size_t tid );

  /**
   * Removes identical MPI send buffer positions to avoid writing
   * data multiple times.
   */
  void compress_secondary_send_buffer_pos( const size_t tid );
};

} // namespace nest

#endif /* #ifndef TARGET_TABLE_H */
