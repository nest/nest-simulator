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
#include "kernel_manager.h"

namespace nest
{

struct Target
{
  // TODO@5g: additional types in nest_types?
  unsigned int tid : 10;            //!< thread of target neuron
  unsigned int rank : 22;         //!< rank of target neuron
  unsigned int processed : 1;
  unsigned int syn_index : 6;  //!< index of synapse type
  unsigned int lcid : 25;          //! local index of connection to target
  Target();
  Target( thread tid, unsigned int rank, unsigned int syn_index, unsigned int lcid);
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
Target::Target( thread tid, unsigned int rank, unsigned int syn_index, unsigned int lcid)
  : tid( tid )
  , rank( rank )
  , processed( false )
  , syn_index( syn_index )
  , lcid( lcid )
{
}

class TargetTable
{
private:
  std::vector< std::vector< std::vector< Target > >* > targets_;
  
public:
  TargetTable();
  ~TargetTable();
  void initialize();
  void finalize();
  // void reserve( thread, synindex, index );
  void add_target( thread tid, index lid, unsigned int vp, synindex syn_index, unsigned int lcid );
  index get_next_target( thread );
  void reject_last_target( thread );
  void clear( thread );
};

inline
void
nest::TargetTable::add_target( thread tid, index lid, unsigned int vp, synindex syn_index, unsigned int lcid)
{
  thread target_tid = vp % kernel().vp_manager.get_num_threads();
  unsigned int target_rank = vp  /  kernel().vp_manager.get_num_threads();
  (*targets_[tid])[lid].push_back( Target( target_tid, target_rank, syn_index, lcid ) );
}

} // namespace nest

#endif
