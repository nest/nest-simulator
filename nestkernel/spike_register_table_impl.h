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

namespace nest
{

inline void
SpikeRegisterTable::add_spike( const thread tid, const SpikeEvent& e, const long_t lag )
{
  // the spike register is separate for each thread; hence we can
  // store the (thread) local id of the sender neuron in the spike
  // register and still identify it uniquely; this simplifies threaded
  // readout of the spike register while collocating mpi buffers
  (*spike_register_[ tid ])[ lag ].push_back( kernel().vp_manager.gid_to_lid( e.get_sender().get_gid() ) );
}

inline void
SpikeRegisterTable::configure()
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

} // of namespace nest

#endif /* SPIKE_REGISTER_TABLE_IMPL_H */
