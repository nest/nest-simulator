/*
 *  send_buffer_position.cpp
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
#include "kernel_manager.h"

#include "send_buffer_position.h"

nest::SendBufferPosition::SendBufferPosition()
  : begin_( kernel::manager< MPIManager >.get_num_processes(), 0 )
  , end_( kernel::manager< MPIManager >.get_num_processes(), 0 )
  , idx_( kernel::manager< MPIManager >.get_num_processes(), 0 )
{
  const size_t num_procs = kernel::manager< MPIManager >.get_num_processes();
  const size_t send_recv_count_per_rank = kernel::manager< MPIManager >.get_send_recv_count_spike_data_per_rank();

  for ( size_t rank = 0; rank < num_procs; ++rank )
  {
    begin_[ rank ] = rank * send_recv_count_per_rank;
    end_[ rank ] = ( rank + 1 ) * send_recv_count_per_rank;
    idx_[ rank ] = begin_[ rank ];
  }
}
