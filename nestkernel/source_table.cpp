/*
 *  source_table.cpp
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

#include "source_table.h"
#include "kernel_manager.h"

nest::SourceTable::SourceTable()
{
  for( thread tid = 0; kernel().vp_manager.get_num_threads(); ++tid)
  {
    synapse_ids_[ tid ] = new std::map< synindex, synindex >();
    sources_[ tid ] = new std::vector< std::vector< Source > >(
      0, std::vector< Source >( 0, Source() ) );
  }
}

nest::SourceTable::~SourceTable()
{
  for( thread tid = 0; kernel().vp_manager.get_num_threads(); ++tid)
  {
    delete synapse_ids_[ tid ];
    delete sources_[ tid ];
  }
}

void
nest::SourceTable::reserve( thread tid, synindex syn_id, index n_sources )
{
  std::map< synindex, synindex >::iterator it = synapse_ids_[ tid ]->find( syn_id );
  if (it != synapse_ids_[ tid ]->end())
  {
    synindex syn_index = it->second;
    index prev_n_sources = (*sources_[ tid ])[ syn_index ].size();
    (*sources_[ tid ])[ syn_index ].reserve( prev_n_sources + n_sources );
  }
  else
  {
    index prev_n_synapse_types = synapse_ids_[ tid ]->size();
    (*synapse_ids_[ tid ])[ syn_id ] = prev_n_synapse_types;
    sources_[ tid ]->resize( prev_n_synapse_types + 1);
    sources_[ tid ][ prev_n_synapse_types ].reserve( n_sources );
  }
}

nest::index
nest::SourceTable::get_next_source( thread tid )
{
}

void
nest::SourceTable::reject_last_source( thread tid )
{
}
