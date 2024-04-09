/*
 *  secondary_event_impl.h
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

#include "secondary_event.h"

// Includes from nestkernel
#include "kernel_manager.h"

template < typename DataType, typename Subclass >
void
nest::DataSecondaryEvent< DataType, Subclass >::add_syn_id( const nest::synindex synid )
{
  kernel().vp_manager.assert_thread_parallel();

  // This is done during connection model cloning, which happens thread-parallel.
  // To not risk trashing the set data structure, we let only master register the
  // new synid. This is not performance critical and avoiding collisions elsewhere
  // would be more difficult, so we do it here in a master section.
#pragma omp master
  {
    supported_syn_ids_.insert( synid );
  }
#pragma omp barrier
}

template < typename DataType, typename Subclass >
void
nest::DataSecondaryEvent< DataType, Subclass >::set_coeff_length( const size_t coeff_length )
{
  kernel().vp_manager.assert_single_threaded();
  coeff_length_ = coeff_length;
}
