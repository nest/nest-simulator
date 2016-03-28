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
  const static unsigned int complete_marker; // 1024 - 1
  const static unsigned int end_marker; // 1024 - 2
  SpikeData();
  SpikeData( const thread tid, const unsigned int syn_index, const unsigned int lcid, const unsigned int lag );
  void set_complete_marker();
  void set_end_marker();
  bool is_complete_marker() const;
  bool is_end_marker() const;
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
SpikeData::set_complete_marker()
{
  tid = complete_marker;
}

inline void
SpikeData::set_end_marker()
{
  tid = end_marker;
}

inline bool
SpikeData::is_complete_marker() const
{
  return tid == complete_marker;
}

inline bool
SpikeData::is_end_marker() const
{
  return tid == end_marker;
}

struct SpikeRegisterPosition
{
  // index for threads
  unsigned int tid;
  // index for lags
  unsigned int lag;
  // index for spikes
  unsigned int sid;
  SpikeRegisterPosition();
  void reset();
};

inline
SpikeRegisterPosition::SpikeRegisterPosition()
  : tid(0)
  , lag(0)
  , sid(0)
{
}

inline void
SpikeRegisterPosition::reset()
{
  tid = 0;
  lag = 0;
  sid = 0;
}

class SpikeRegisterTable
{
private:
  std::vector< std::vector< std::vector< index > >* > spike_register_;
  std::vector< SpikeRegisterPosition* > current_positions_;
  std::vector< SpikeRegisterPosition* > saved_positions_;
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
  void toggle_target_processed_flags( const thread tid );
};
  
inline void
SpikeRegisterTable::save_entry_point( const thread tid )
{
  if ( not saved_entry_point_[ tid ] )
  {
    SpikeRegisterPosition& current_position = *current_positions_[ tid ];
    SpikeRegisterPosition& saved_position = *saved_positions_[ tid ];
    saved_position.tid = current_position.tid;
    saved_position.lag = current_position.lag;
    // we substract one since this function can be called after
    // reject_last_spike_data, in which the current_sid_ target was
    // not added to the spike buffer. if we start one before the
    // current one, we make sure to pick it up in the next
    // communication round.
    if ( current_position.sid > 0 )
    {
      saved_position.sid = current_position.sid - 1;
    }
    else
    {
      saved_position.sid = 0;
    }
    saved_entry_point_[ tid ] = true;
  }
}

inline void
SpikeRegisterTable::restore_entry_point( const thread tid )
{
  *current_positions_[ tid ] = *saved_positions_[ tid ];
  saved_entry_point_[ tid ] = false;
}

inline void
SpikeRegisterTable::reset_entry_point( const thread tid )
{
  saved_positions_[ tid ]->reset();
  current_positions_[ tid ]->reset();
}

inline void
SpikeRegisterTable::clear( const thread tid )
{
  for ( std::vector< std::vector< index > >::iterator it = spike_register_[ tid ]->begin();
        it != spike_register_[ tid ]->end(); ++it )
  {
    it->clear();
  }
}

} // namespace nest

#endif /* SPIKE_REGISTER_TABLE_H */
