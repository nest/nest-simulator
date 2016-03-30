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

/** A data structure containing all information required to uniquely
 * identify a target neuron on a machine. Used in TargetTable for
 * presynaptic part of connection infrastructure.
 */
struct Target
{
  // TODO@5g: additional types in nest_types?
  unsigned int lcid : 25;           //! local index of connection to target
  unsigned int rank : 22;           //!< rank of target neuron
  unsigned int tid : 10;            //!< thread of target neuron
  unsigned int syn_index : 6;       //!< index of synapse type
  unsigned int processed : 1;       //!< has this entry been processed
  Target();
  Target( const Target& target );
  Target( const thread tid, const unsigned int rank, const unsigned int syn_index, const unsigned int lcid);
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
  , processed( false )
{
}

inline
Target::Target( const thread tid, const unsigned int rank, const unsigned int syn_index, const unsigned int lcid)
  : lcid( lcid )
  , rank( rank )
  , tid( tid )
  , syn_index( syn_index )
  , processed( false )
{
}

/** A data structure used to communicate part of the infrastructure
 * from post- to presynaptic side.
 */
struct TargetData
{
  Target target;
  unsigned int lid : 16;
  unsigned int tid : 8;
  static const index complete_marker; // 2^16 - 1
  static const index end_marker; // 2^16 - 2
  TargetData();
  void set_complete_marker();
  void set_end_marker();
  bool is_complete_marker() const;
  bool is_end_marker() const;
};

inline
TargetData::TargetData()
  : lid( 0 )
  , target( Target() )
{
}

inline void
TargetData::set_complete_marker()
{
  lid = complete_marker;
}

inline void
TargetData::set_end_marker()
{
  lid = end_marker;
}

inline bool
TargetData::is_complete_marker() const
{
  return lid == complete_marker;
}

inline bool
TargetData::is_end_marker() const
{
  return lid == end_marker;
}

/** This data structure stores the targets of the local neurons, i.e.,
 * the presynaptic part of the connection infrastructure. The core
 * structure is a three dimensional vector, which is arranged as
 * follows:
 * 1st dimension: threads
 * 2nd dimension: local nodes/neurons
 * 3rd dimension: targets
 */
class TargetTable
{
private:
  //! stores remote targets of local neurons
  std::vector< std::vector< std::vector< Target > >* > targets_;
  //! stores the current value to mark processed entries in targets_
  std::vector< std::vector< bool >* > target_processed_flag_;
  //! keeps track of index in target vector for each thread
  std::vector< index > current_target_index_;
  
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
  void add_target( thread tid, const TargetData& target_data );
  // void clear( thread );
  //! returns the next spike data according to current_target_index
  bool get_next_spike_data( const thread tid, const thread current_tid, const index lid, index& rank, SpikeData& next_spike_data, const unsigned int rank_start, const unsigned int rank_end );
  //! flips the value of the processed entries marker
  void toggle_target_processed_flag( const thread tid, const index lid );
  //! rejects the last spike data and resets current_target_index accordinglyp
  void reject_last_spike_data( const thread tid, const thread current_tid, const index current_lid );
  // TODO@5g: don't we need save/restore/reset as in communication of source table?
  void reset_current_target_index( const thread );

  std::vector< Target >& get_targets( const thread tid, const index lid );
};

inline
void
TargetTable::reject_last_spike_data( const thread tid, const thread current_tid, const index current_lid )
{
  assert( current_target_index_[ tid ] > 0 );
  ( *targets_[ current_tid ])[ current_lid ][ current_target_index_[ tid ] - 1 ].processed = not (*target_processed_flag_[ current_tid ])[ current_lid ];
}

inline void
TargetTable::reset_current_target_index( const thread tid )
{
  current_target_index_[ tid ] = 0;
}

inline void
TargetTable::toggle_target_processed_flag( const thread tid, const index lid )
{
  (*target_processed_flag_[ tid ])[ lid ] = not (*target_processed_flag_[ tid ])[ lid ];
}

inline std::vector< Target >&
TargetTable::get_targets( const thread tid, const index lid )
{
  return (*targets_[ tid ])[ lid ];
}

} // namespace nest

#endif
