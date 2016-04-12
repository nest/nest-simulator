/*
 *  spike_register_table_impl.h
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

#ifndef SPIKE_REGISTER_TABLE_IMPL_H
#define SPIKE_REGISTER_TABLE_IMPL_H

// Includes from nestkernel:
#include "kernel_manager.h"
#include "spike_register_table.h"
#include "vp_manager_impl.h"
#include "event.h"
#include "connection_builder_manager_impl.h"

namespace nest
{

inline void
SpikeData::set( const thread tid, const unsigned int syn_index, const unsigned int lcid, const unsigned int lag )
{
  std::cout << "set spike data for target " << kernel().connection_builder_manager.get_target_gid( tid, syn_index, lcid ) << std::endl;
  (*this).tid = tid;
  (*this).syn_index = syn_index;
  (*this).lcid = lcid;
  (*this).lag = lag;
}

inline void
SpikeRegisterTable::add_spike( const thread tid, const SpikeEvent& e, const long_t lag )
{
  // the spike register is separate for each thread; hence we can
  // store the (thread) local id of the sender neuron in the spike
  // register and still identify it uniquely; this simplifies threaded
  // readout of the spike register while collocating mpi buffers
  (*spike_register_[ tid ])[ lag ].push_back( kernel().vp_manager.gid_to_lid( e.get_sender().get_gid() ) );
}

inline bool
SpikeRegisterTable::get_next_spike_data( const thread tid, index& rank, SpikeData& next_spike_data, const unsigned int rank_start, const unsigned int rank_end )
{
  SpikeRegisterPosition& current_position = *current_positions_[ tid ];
  while ( true )
  {
    assert( current_position.tid <= spike_register_.size() );
    if ( current_position.tid == spike_register_.size() )
    {
      return false;
    }
    else
    {
      if ( current_position.lag == spike_register_[ current_position.tid ]->size() )
      {
        assert( current_position.sid == 0 );
        current_position.lag = 0;
        ++current_position.tid;
        continue;
      }
      else
      {
        if ( current_position.sid == (*spike_register_[ current_position.tid ])[ current_position.lag ].size() )
        {
          current_position.sid = 0;
          ++current_position.lag;
          continue;
        }
        else
        {
          const index current_lid = ( *spike_register_[ current_position.tid ] )[ current_position.lag ][ current_position.sid ];
          if ( kernel().connection_builder_manager.get_next_spike_data( tid, current_position.tid, current_lid, rank, next_spike_data, rank_start, rank_end ) )
          {
            next_spike_data.lag = current_position.lag;
            return true;
          }
          else
          {
            ++current_position.sid;
          }
        }
      }
    }
  }
}

inline void
SpikeRegisterTable::reject_last_spike_data( const thread tid )
{
  SpikeRegisterPosition& current_position = *current_positions_[ tid ];
  const index current_lid = ( *spike_register_[ current_position.tid ] )[ current_position.lag ][ current_position.sid ];
  kernel().connection_builder_manager.reject_last_spike_data( tid, current_position.tid, current_lid );
}

inline void
SpikeRegisterTable::toggle_target_processed_flags( const thread tid )
{
  for ( std::vector< std::vector< index > >::iterator it = spike_register_[ tid ]->begin();
        it != spike_register_[ tid ]->end(); ++it )
  {
    for ( std::vector< index >::iterator jt = it->begin(); jt != it->end(); ++jt )
    {
      kernel().connection_builder_manager.toggle_target_processed_flag( tid, *jt );
    }
  }
}

} // of namespace nest

#endif /* SPIKE_REGISTER_TABLE_IMPL_H */
