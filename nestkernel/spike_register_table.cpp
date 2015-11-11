/*
 *  spike_register_table.cpp
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

// Includes from nestkernel:
#include "spike_register_table.h"
#include "target_table.h"
#include "kernel_manager.h"

nest::SpikeRegisterTable::SpikeRegisterTable()
{
}

nest::SpikeRegisterTable::~SpikeRegisterTable()
{
}

void
nest::SpikeRegisterTable::initialize()
{
  thread num_threads = kernel().vp_manager.get_num_threads();
  
  spike_register_.resize( num_threads );

  saved_entry_point_.resize( num_threads );
  current_tid_.resize( num_threads );
  current_lag_.resize( num_threads );
  current_lid_.resize( num_threads );
  save_tid_.resize( num_threads );
  save_lag_.resize( num_threads );
  save_lid_.resize( num_threads );

  for( thread tid = 0; tid < num_threads; ++tid)
  {
    spike_register_[ tid ] = new std::vector< std::vector< index > >(
      kernel().connection_builder_manager.get_min_delay(),
      std::vector< index >( 0 ) );
    saved_entry_point_[ tid ] = false;
  }
}

void
nest::SpikeRegisterTable::finalize()
{
  for( std::vector< std::vector< std::vector< index > >* >::iterator it =
         spike_register_.begin(); it != spike_register_.end(); ++it )
  {
    delete *it;
  }
  spike_register_.clear();
}

bool
nest::SpikeRegisterTable::get_next_spike_data( const thread tid, index& rank, SpikeData& next_spike_data, const unsigned int rank_start, const unsigned int rank_end )
{
  while ( true )
  {
    if ( current_tid_[ tid ] == spike_register_.size() )
    {
      return false;
    }
    else
    {
      if ( current_lag_[ tid ] == spike_register_[ current_tid_[ tid ] ]->size() )
      {
        current_lag_[ tid ] = 0;
        ++current_tid_[ tid ];
        continue;
      }
      else
      {
        if ( current_lid_[ tid ] == (*spike_register_[ current_tid_[ tid ] ])[ current_lag_[ tid ] ].size() )
        {
          current_lid_[ tid ] = 0;
          ++current_lag_[ tid ];
          continue;
        }
        else
        {
          index lid = ( *spike_register_[ current_tid_[ tid ] ] )[ current_lag_[ tid ] ][ current_lid_[ tid ] ];
          if ( kernel().connection_builder_manager.get_next_spike_data( tid, current_tid_[ tid ], lid, rank, next_spike_data, rank_start, rank_end ) )
          {
            next_spike_data.lag = current_lag_[ tid ];
            return true;
          }
          else
          {
            ++current_lid_[ tid ];
          }
        }
      }
    }
  }
}

void
nest::SpikeRegisterTable::reject_last_spike_data( const thread tid )
{
  index lid = ( *spike_register_[ current_tid_[ tid ] ] )[ current_lag_[ tid ] ][ current_lid_[ tid ] ];
  kernel().connection_builder_manager.reject_last_spike_data( tid, current_tid_[ tid ], lid );
}

void
nest::SpikeRegisterTable::save_entry_point( const thread tid )
{
  if ( not saved_entry_point_[ tid ] )
  {
    save_tid_[ tid ] = current_tid_[ tid ];
    save_lag_[ tid ] = current_lag_[ tid ];
    save_lid_[ tid ] = current_lid_[ tid ];
    saved_entry_point_[ tid ] = true;
  }
}

void
nest::SpikeRegisterTable::restore_entry_point( const thread tid )
{
  current_tid_[ tid ] = save_tid_[ tid ];
  current_lag_[ tid ] = save_lag_[ tid ];
  current_lid_[ tid ] = save_lid_[ tid ];
  saved_entry_point_[ tid ] = false;
}

void
nest::SpikeRegisterTable::reset_entry_point( const thread tid )
{
  save_tid_[ tid ] = 0;
  save_lag_[ tid ] = 0;
  save_lid_[ tid ] = 0;
}
