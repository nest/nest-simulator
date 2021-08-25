/*
 *  io_manager_impl.h
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

#ifndef IO_MANAGER_IMPL_H
#define IO_MANAGER_IMPL_H

#include "io_manager.h"

namespace nest
{
template < class RBType >
void
IOManager::register_recording_backend( Name name )
{
  RBType* recording_backend = new RBType();
  recording_backend->pre_run_hook();

  recording_backends_.insert( std::make_pair( name, recording_backend ) );
}

template < class SBType >
void
IOManager::register_stimulation_backend( Name name )
{
  SBType* stimulation_backend = new SBType();
  auto it = stimulation_backends_.find( name );
  if ( it == stimulation_backends_.end() )
  {
    stimulation_backend->pre_run_hook();
    stimulation_backends_.insert( std::make_pair( name, stimulation_backend ) );
  }
}
}

#endif /* #ifndef IO_MANAGER_IMPL_H */
