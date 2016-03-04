/*
 *  spike_register_table.h
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

#ifndef SPIKE_REGISTER_TABLE_H
#define SPIKE_REGISTER_TABLE_H

// C++ includes:
#include <vector>
#include <map>
#include <cassert>

// Includes from nestkernel:
#include "nest_types.h"

namespace nest
{
class SpikeEvent;

struct SpikeData
{
  unsigned int tid : 10;
  unsigned int syn_index : 6;
  unsigned int lcid : 25;
  unsigned int lag : 6;
  const static unsigned int empty_marker; // 1024 - 1
  const static unsigned int complete_marker; // 1024 - 2
  SpikeData();
  SpikeData( const thread tid, const unsigned int syn_index, const unsigned int lcid, const unsigned int lag );
  void set_empty();
  void set_complete();
  bool is_empty() const;
  bool is_complete() const;
};

inline
SpikeData::SpikeData()
  : tid( 0 )
  , syn_index( 0 )
  , lcid( 0 )
  , lag( 0 )
{
}

inline
SpikeData::SpikeData( const thread tid, const unsigned int syn_index, const unsigned int lcid, const unsigned int lag )
  : tid( tid )
  , syn_index( syn_index )
  , lcid( lcid )
  , lag( lag )
{
}

inline void
SpikeData::set_empty()
{
  tid = empty_marker;
}

inline void
SpikeData::set_complete()
{
  tid = complete_marker;
}

inline bool
SpikeData::is_empty() const
{
  return tid == empty_marker;
}

inline bool
SpikeData::is_complete() const
{
  return tid == complete_marker;
}

class SpikeRegisterTable
{
private:
  std::vector< std::vector< std::vector< index > >* > spike_register_;
  std::vector< unsigned int > current_tid_;
  std::vector< unsigned int > current_lag_;
  std::vector< unsigned int > current_lid_;
  std::vector< unsigned int > save_tid_;
  std::vector< unsigned int > save_lag_;
  std::vector< unsigned int > save_lid_;
  std::vector< bool > saved_entry_point_;

public:
  SpikeRegisterTable();
  ~SpikeRegisterTable();
  void initialize();
  void finalize();
  void add_spike( const thread tid, const SpikeEvent& e, const long_t lag );
  void configure();
  void clear( const thread tid );
  bool get_next_spike_data( const thread tid, index& rank, SpikeData& next_spike_data, const unsigned int rank_start, const unsigned int rank_end );
  void reject_last_spike_data( const thread tid );
  void save_entry_point( const thread tid );
  void restore_entry_point( const thread tid );
  void reset_entry_point( const thread tid );

};

} // namespace nest

#endif
