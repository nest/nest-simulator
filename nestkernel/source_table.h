/*
 *  source_table.h
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

#ifndef SOURCE_TABLE_H
#define SOURCE_TABLE_H

// C++ includes:
#include <vector>
#include <map>
#include <cassert>

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{

struct TargetData;

struct Source
{
  // TODO@5g: use nest_types: index
  // TODO@5g: possible to define in nest_types?
  unsigned long gid : 63;
  unsigned long processed : 1;
  Source();
  Source( index );
};

inline
Source::Source()
  : gid( 0 )
  , processed( false )
{
}

inline
Source::Source( index gid )
  : gid( gid )
  , processed( false )
{
}

class SourceTable
{
private:
  std::vector< std::vector< std::vector< Source > >* > sources_;
  std::vector< std::map< synindex, synindex >* > synapse_ids_;
  std::vector< bool > is_cleared_;
  std::vector< unsigned int > current_tid_;
  std::vector< unsigned int > current_syn_id_;
  std::vector< unsigned int > current_lcid_;
  std::vector< unsigned int > save_tid_;
  std::vector< unsigned int > save_syn_id_;
  std::vector< unsigned int > save_lcid_;
  std::vector< bool > saved_entry_point_;

public:
  SourceTable();
  ~SourceTable();
  void initialize();
  void finalize();
  // void reserve( thread, synindex, index );
  void add_source( thread, synindex, index );
  void clear( thread );
  bool is_cleared() const;
  void get_next_target_data( const thread tid, TargetData& next_target_data );
  void reject_last_target_data( const thread tid );
  void save_entry_point( const thread tid );
  void restore_entry_point( const thread tid );
  void reset_entry_point( const thread tid );
};

inline
void
nest::SourceTable::add_source( thread tid, synindex syn_id, index gid)
{
  std::map< synindex, synindex >::iterator it = synapse_ids_[ tid ]->find( syn_id );
  Source src( gid );
  // if this synapse type is not known yet, create entry for new synapse vector
  if (it == synapse_ids_[ tid ]->end())
  {
    index prev_n_synapse_types = synapse_ids_[ tid ]->size();
    (*synapse_ids_[ tid ])[ syn_id ] = prev_n_synapse_types;
    sources_[ tid ]->resize( prev_n_synapse_types + 1);
    (*sources_[ tid ])[ prev_n_synapse_types ].push_back( src );
  }
  else
  {
    (*sources_[ tid ])[ it->second ].push_back( src );
  }
}

inline
void
nest::SourceTable::clear( thread tid )
{
  sources_[ tid ]->clear();
  is_cleared_[ tid ] = true;
}

} // namespace nest

#endif
