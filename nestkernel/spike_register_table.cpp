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

const unsigned int nest::SpikeData::empty_marker = ( 1024 - 1 );
const unsigned int nest::SpikeData::complete_marker = ( 1024 - 2 );
const unsigned int nest::SpikeData::end_marker = ( 1024 - 3 );

nest::SpikeRegisterTable::SpikeRegisterTable()
{
}

nest::SpikeRegisterTable::~SpikeRegisterTable()
{
}

void
nest::SpikeRegisterTable::initialize()
{
  const thread num_threads = kernel().vp_manager.get_num_threads();

  spike_register_.resize( num_threads );

  saved_entry_point_.resize( num_threads, false );
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

void
nest::SpikeRegisterTable::clear( const thread tid )
{
  for ( std::vector< std::vector< index > >::iterator it = spike_register_[ tid ]->begin();
        it != spike_register_[ tid ]->end(); ++it )
  {
    it->clear();
  }
}

void
nest::SpikeRegisterTable::configure()
{
  for ( std::vector< std::vector< std::vector< index > >* >::iterator it = spike_register_.begin();
        it != spike_register_.end(); ++it )
  {
    (*it)->resize( kernel().connection_builder_manager.get_min_delay() );
    for ( std::vector< std::vector< index > >::iterator iit = (*it)->begin(); iit != (*it)->end(); ++iit)
    {
      iit->clear();
    }
  }
}
