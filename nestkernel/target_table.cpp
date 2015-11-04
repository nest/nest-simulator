/*
 *  target_table.cpp
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

#include "target_table.h"

nest::TargetTable::TargetTable()
{
}

nest::TargetTable::~TargetTable()
{
}

void
nest::TargetTable::initialize()
{
  thread num_threads = kernel().vp_manager.get_num_threads();
  targets_.resize( num_threads );
  for( thread tid = 0; tid < num_threads; ++tid)
  {
    targets_[ tid ] = new std::vector< std::vector< Target > >(
      0, std::vector< Target >( 0, Target() ) );
  }
}

void
nest::TargetTable::finalize()
{
  for( std::vector< std::vector< std::vector< Target > >* >::iterator it =
         targets_.begin(); it != targets_.end(); ++it )
  {
    delete *it;
  }
  targets_.clear();
}

// TODO@5g: benchmark with and without reserving memory for synapses
// TODO@5g: if we use reserve, we need to make sure the synapse type is known
// void
// nest::TargetTable::reserve( thread tid, synindex syn_id, index n_targets )
// {
// }

nest::index
nest::TargetTable::get_next_target( thread tid )
{
}

void
nest::TargetTable::reject_last_target( thread tid )
{
}
