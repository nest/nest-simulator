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

namespace nest
{
struct SpikeData;

// TODO@5g: documentation
struct Target
{
  // TODO@5g: additional types in nest_types?
  unsigned int tid : 10;            //!< thread of target neuron
  unsigned int rank : 22;         //!< rank of target neuron
  unsigned int processed : 1;
  unsigned int syn_index : 6;  //!< index of synapse type
  unsigned int lcid : 25;          //! local index of connection to target
  Target();
  Target( const Target& target );
  Target( const thread tid, const unsigned int rank, const unsigned int syn_index, const unsigned int lcid);
};

inline
Target::Target()
  : tid( 0 )
  , rank( 0 )
  , processed( false )
  , syn_index( 0 )
  , lcid( 0 )
{
}

inline
Target::Target( const Target& target )
  : tid( target.tid )
  , rank( target.rank )
  , processed( false )
  , syn_index( target.syn_index )
  , lcid( target.lcid )
{
}

inline
Target::Target( const thread tid, const unsigned int rank, const unsigned int syn_index, const unsigned int lcid)
  : tid( tid )
  , rank( rank )
  , processed( false )
  , syn_index( syn_index )
  , lcid( lcid )
{
}

// TODO@5g: documentation
struct TargetData
{
  index gid;
  Target target;
  static const index empty_marker; // std::numeric_limits< index >::max()
  static const index complete_marker; // std::numeric_limits< index >::max() - 1
  TargetData();
  TargetData( const index gid, const Target& target);
  void set_empty();
  void set_complete();
  bool is_empty() const;
  bool is_complete() const;
};

inline
TargetData::TargetData()
  : gid( 0 )
  , target( Target() )
{
}

inline
TargetData::TargetData( const index gid, const Target& target )
  : gid( gid )
  , target( target )
{
}

inline void
TargetData::set_empty()
{
  gid = empty_marker;
}

inline void
TargetData::set_complete()
{
  gid = complete_marker;
}

inline bool
TargetData::is_empty() const
{
  return gid == empty_marker;
}

inline bool
TargetData::is_complete() const
{
  return gid == complete_marker;
}

class TargetTable
{
private:
  std::vector< std::vector< std::vector< Target > >* > targets_;
  std::vector< index > current_target_index_;
  
public:
  TargetTable();
  ~TargetTable();
  void initialize();
  void finalize();
  void prepare( thread tid );
  // void reserve( thread, synindex, index );
  void add_target( thread tid, const TargetData& target_data );
  void clear( thread );
  bool get_next_spike_data( const thread tid, const thread current_tid, const index lid, index& rank, SpikeData& next_spike_data, const unsigned int rank_start, const unsigned int rank_end );
  void reject_last_spike_data( const thread tid, const thread current_tid, const index lid );
};

} // namespace nest

#endif
