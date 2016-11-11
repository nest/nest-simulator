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
#include <vector>
#include <map>
#include <cassert>

// Includes from nestkernel:
#include "nest_types.h"
#include "spike_data.h"
#include "target.h"
#include "target_data.h"

namespace nest
{

/**
 * This data structure stores all targets of the local neurons. This
 * is the presynaptic part of the connection infrastructure. The core
 * structure is a three dimensional vector, which is arranged as
 * follows:
 * 1st dimension: threads
 * 2nd dimension: local neurons
 * 3rd dimension: targets
 */
class TargetTable
{
private:
  //! stores (remote) targets of local neurons
  std::vector< std::vector< std::vector< Target > >* > targets_;

public:
  TargetTable();
  ~TargetTable();
  //! initialize data structures
  void initialize();
  //! delete data structure
  void finalize();
  //! adjust targets table's size to number of local nodes
  void prepare( const thread tid );
  // void reserve( thread, synindex, index );
  //! add entry to target table
  void add_target( const thread tid, const TargetData& target_data );
  //! returns all targets of a neuron. used to fill spike_register_5g_
  //! in event_delivery_manager
  const std::vector< Target >& get_targets( const thread tid,
    const index lid ) const;
  //! clear all entries
  void clear( const thread tid );

  void print_targets( const thread tid ) const;
};

inline void
TargetTable::add_target( const thread tid, const TargetData& target_data )
{
  ( *targets_[ tid ] )[ target_data.lid ].push_back( target_data.target );
}

inline const std::vector< Target >&
TargetTable::get_targets( const thread tid, const index lid ) const
{
  return ( *targets_[ tid ] )[ lid ];
}

inline void
TargetTable::clear( const thread tid )
{
  ( *targets_[ tid ] ).clear();
}

} // namespace nest

#endif
