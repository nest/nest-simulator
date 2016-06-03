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

namespace nest
{

struct SpikeData;

/**
 * A structure containing all information required to uniquely
 * identify a target neuron on a (remote) machine. Used in TargetTable
 * for presynaptic part of connection infrastructure.
 */
struct Target
{
  index lcid : 27; //!< local connection index
  thread rank : 20; //!< rank of target neuron
  thread tid : 10; //!< thread index
  synindex syn_index : 6; //!< synapse-type index
  bool processed;
  Target();
  Target( const Target& target );
  Target( const thread tid, const thread rank, const synindex syn_index, const index lcid);
  void set_processed();
  bool is_processed() const;
  double_t get_offset() const;
};

inline
Target::Target()
  : lcid( 0 )
  , rank( 0 )
  , tid( 0 )
  , syn_index( 0 )
  , processed( false )
{
}

inline
Target::Target( const Target& target )
  : lcid( target.lcid )
  , rank( target.rank )
  , tid( target.tid )
  , syn_index( target.syn_index )
  , processed( false ) // always initialize as non-processed
{
}

inline
Target::Target( const thread tid, const thread rank, const synindex syn_index, const index lcid)
  : lcid( lcid )
  , rank( rank )
  , tid( tid )
  , syn_index( syn_index )
  , processed( false ) // always initialize as non-processed
{
}

inline void
Target::set_processed()
{
  processed = true;
}

inline bool
Target::is_processed() const
{
  return processed;
}

inline double_t
Target::get_offset() const
{
  return 0;
}

struct OffGridTarget : Target
{
  double_t offset;
  OffGridTarget();
  OffGridTarget( const Target& target, const double_t offset );
  double_t get_offset() const;
};

inline
OffGridTarget::OffGridTarget()
  : Target()
{
}

inline
OffGridTarget::OffGridTarget(const Target& target, const double_t offset )
  : Target( target )
  , offset( offset )
{
}

inline double_t
OffGridTarget::get_offset() const
{
  return offset;
}

/** 
 * Structure used to communicate part of the connection infrastructure
 * from post- to presynaptic side. These are the elements of the MPI
 * buffers.
 * SeeAlso: SpikeData
 */
struct TargetData
{
  Target target;
  index lid : 20; //!< local id of presynaptic neuron
  thread tid : 10; //!< thread index of presynaptic neuron
  unsigned int marker : 2;
  static const unsigned int end_marker; // =1
  static const unsigned int complete_marker; // =2
  static const unsigned int invalid_marker; // =3
  TargetData();
  void reset_marker();
  void set_complete_marker();
  void set_end_marker();
  void set_invalid_marker();
  bool is_complete_marker() const;
  bool is_end_marker() const;
  bool is_invalid_marker() const;
};

inline
TargetData::TargetData()
  : target( Target() )
  , lid( 0 )
  , tid( 0 )
  , marker( 0 )
{
}

inline void
TargetData::reset_marker()
{
  marker = 0;
}

inline void
TargetData::set_complete_marker()
{
  marker = complete_marker;
}

inline void
TargetData::set_end_marker()
{
  marker = end_marker;
}

inline void
TargetData::set_invalid_marker()
{
  marker = invalid_marker;
}

inline bool
TargetData::is_complete_marker() const
{
  return marker == complete_marker;
}

inline bool
TargetData::is_end_marker() const
{
  return marker == end_marker;
}

inline bool
TargetData::is_invalid_marker() const
{
  return marker == invalid_marker;
}

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
  const std::vector< Target >& get_targets( const thread tid, const index lid ) const;
  //! clear all entries
  void clear( const thread tid );
};

inline void
TargetTable::add_target( const thread tid, const TargetData& target_data )
{
  (*targets_[ tid ])[ target_data.lid ].push_back( target_data.target );
}

inline const std::vector< Target >&
TargetTable::get_targets( const thread tid, const index lid ) const
{
  return (*targets_[ tid ])[ lid ];
}

inline void
TargetTable::clear( const thread tid )
{
  (*targets_[ tid ]).clear();
}

} // namespace nest

#endif
